#include "serviceNode.h"

#include <cstdint>
#include <iostream>
#include <unordered_map>
#include <vector>

#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <ifaddrs.h>
#include <arpa/inet.h>
#include <net/if.h>

using namespace std;


static unordered_map<string, ServiceNode> aliveNeighbours;
static int inetSocket;
static vector<sockaddr_in> broadcastAddresses;

static void removeOldNeighbours(unordered_map<string, ServiceNode>& map) {
    static constexpr uint32_t serviceTimeoutInSeconds = 10;
    for (auto it = map.begin(); it != map.end();) {
        if (it->second.getSecondsPassedSinceLastActivity() > serviceTimeoutInSeconds) {
            cout << "Removed old service: " << it->second.getMacAddress() << "\n";
            it = map.erase(it);
        } else {
            it++; // we cant put it into for loop, as erase() invalidates iterator therefore making it++ UB
        }
    }
}

static int getUdpBroadcastSocket() {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("socket");
        std::exit(1);
    }
    int broadcastEnable = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &broadcastEnable, sizeof(broadcastEnable)) < 0) {
        perror("setsockopt");
        close(sock);
        std::exit(1);
    }
    return sock;
}

static void getBroadcastAddresses(vector<sockaddr_in>& whereToStore) {
    struct ifaddrs* interfaceListStart;
    if (getifaddrs(&interfaceListStart) == -1) {
        perror("getifaddrs");
        exit(1);
    }

    for (struct ifaddrs* interface = interfaceListStart; interface != nullptr; interface = interface->ifa_next) {
        if (interface->ifa_addr == nullptr) continue;

        if ((interface->ifa_addr->sa_family == AF_INET) && (interface->ifa_flags & IFF_BROADCAST)) {
            struct sockaddr_in* bcast = reinterpret_cast<sockaddr_in*>(interface->ifa_broadaddr);
            if (bcast) {
                broadcastAddresses.push_back(*bcast);

                char buf[INET_ADDRSTRLEN];
                inet_ntop(AF_INET, &bcast->sin_addr, buf, sizeof(buf));
                std::cout << "Interface " << interface->ifa_name << " broadcast: " << buf << std::endl;
            }
        }
    }

    freeifaddrs(interfaceListStart);
}

void setup() {
    ServiceNode testObject("127.0.0.1", "testMacAddress");
    aliveNeighbours.insert({testObject.getMacAddress(), testObject});

    inetSocket = getUdpBroadcastSocket();
    getBroadcastAddresses(broadcastAddresses);
    
}

void loop() {
    //broadcastHeartbeat();
    //checkForUdpMessages();
    removeOldNeighbours(aliveNeighbours);
}

int main () {
    setup();
    while (true) {
        loop();
        sleep(5);
    }
}

