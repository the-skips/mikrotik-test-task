#include "serviceNode.h"

#include <cstdint>
#include <iostream>
#include <unordered_map>
#include <vector>
#include <cstring>

#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <ifaddrs.h>
#include <arpa/inet.h>
#include <net/if.h>

using namespace std;


static unordered_map<string, ServiceNode> aliveNeighbours;
static int inetSocket;

struct InterfaceInfo {
    string name;
    sockaddr_in broadcastAddr;
    string macAddress;
};
static vector<InterfaceInfo> broadcastCapableInterfaces;

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

static std::vector<InterfaceInfo> getInetInterfaces(int socket) {
    struct ifaddrs* interfaceListStart;
    if (getifaddrs(&interfaceListStart) == -1) {
        perror("getifaddrs");
        exit(1);
    }
    std::vector<InterfaceInfo> inetInterfaces;

    for (struct ifaddrs* interface = interfaceListStart; interface != nullptr; interface = interface->ifa_next) {
        if (interface->ifa_addr == nullptr) continue;

        if ((interface->ifa_addr->sa_family == AF_INET) && (interface->ifa_flags & IFF_BROADCAST)) { // inet4 interfaces with broadcast capability
            InterfaceInfo ifInfo;

            // interface name
            ifInfo.name = interface->ifa_name;

            // broadcast ip
            struct sockaddr_in* bcast = reinterpret_cast<sockaddr_in*>(interface->ifa_broadaddr);
            if (bcast) {
                ifInfo.broadcastAddr = *bcast;

                char buf[INET_ADDRSTRLEN];
                inet_ntop(AF_INET, &bcast->sin_addr, buf, sizeof(buf));
                std::cout << "Interface " << ifInfo.name
                          << " broadcast: " << buf << std::endl;
            }
            // mac
            struct ifreq ifr;
            strncpy(ifr.ifr_name, interface->ifa_name, IFNAMSIZ-1);
            ifr.ifr_name[IFNAMSIZ-1] = '\0';

            if (ioctl(socket, SIOCGIFHWADDR, &ifr) == 0) {
                unsigned char* mac = reinterpret_cast<unsigned char*>(ifr.ifr_hwaddr.sa_data);
                char macStr[18];
                std::snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
                              mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
                ifInfo.macAddress = macStr;

                std::cout << "Interface " << ifInfo.name
                          << " MAC: " << ifInfo.macAddress << std::endl;
            } else {
                perror("ioctl SIOCGIFHWADDR");
            }

            inetInterfaces.push_back(ifInfo);
        }
    }

    freeifaddrs(interfaceListStart);
    return inetInterfaces;
}

void setup() {
    ServiceNode testObject("127.0.0.1", "testMacAddress");
    aliveNeighbours.insert({testObject.getMacAddress(), testObject});

    inetSocket = getUdpBroadcastSocket();
    broadcastCapableInterfaces = getInetInterfaces(inetSocket);
    
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

