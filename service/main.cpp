#include "cliServer.h"
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

// constants are here for now
static constexpr uint16_t servicePort = 9000;
static constexpr uint32_t serviceTimeoutInSeconds = 30;
static constexpr const char* cliSocketPath = "/tmp/neighbourService.sock";

// structs, enums (can be put into separate header file)
struct InterfaceInfo {
    string name;
    sockaddr_in broadcastAddr;
    string macAddress;
};

// globals
static vector<InterfaceInfo> broadcastCapableInterfaces;
static unordered_map<string, ServiceNode> aliveNeighbours;
static int inetSocket;
static CliServer cliServer(cliSocketPath);


// function prototypes
static void removeOldNeighbours(unordered_map<string, ServiceNode>& map);
static int getUdpBroadcastSocket();
static std::vector<InterfaceInfo> getInetInterfaces(int socket);
static void broadcastHeartbeat(const std::vector<InterfaceInfo>& interfaces, const int sockFd);
static void checkForUdpMessages(const int socket);
void setup();
void loop();


// functions themselves
int main () {
    setup();
    while (true) {
        loop();
        sleep(2);
    }
}

void setup() {
    inetSocket = getUdpBroadcastSocket();
    broadcastCapableInterfaces = getInetInterfaces(inetSocket);
    cliServer.setup();
}

void loop() {
    broadcastHeartbeat(broadcastCapableInterfaces, inetSocket);
    checkForUdpMessages(inetSocket);
    removeOldNeighbours(aliveNeighbours);
    cliServer.serverLoop(aliveNeighbours);
}

static void removeOldNeighbours(unordered_map<string, ServiceNode>& map) {
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
    // for sending messages
    int broadcastEnable = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &broadcastEnable, sizeof(broadcastEnable)) < 0) {
        perror("setsockopt(broadcast)");
        close(sock);
        std::exit(1);
    }
    // for receiving messages
    int reuse = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
        perror("setsockopt(reuse address)");
        close(sock);
        std::exit(1);
    }
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(servicePort);

    if (bind(sock, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
        perror("bind");
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
                ifInfo.broadcastAddr.sin_port = htons(servicePort);

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

static void broadcastHeartbeat(const std::vector<InterfaceInfo>& interfaces, const int sockFd) {
    for (const InterfaceInfo& iface : interfaces) {
        socklen_t addrLen = sizeof(iface.broadcastAddr);

        ssize_t sentBytes = sendto(sockFd,
            iface.macAddress.c_str(),
            iface.macAddress.length(),
            0,
            reinterpret_cast<const struct sockaddr*>(&iface.broadcastAddr),
            addrLen
        );
        if (sentBytes < 0) {
            perror(("sendto failed on interface " + iface.name).c_str());
        } else {
            std::cout << "Sent heartbeat from " << iface.macAddress
                      << " on interface " << iface.name << std::endl;
        }
    }
}

static void checkForUdpMessages(const int socket) {
    constexpr uint16_t bufferSize = 256;
    char buffer[bufferSize];
    sockaddr_in senderAddr{};
    socklen_t addrLen = sizeof(senderAddr);

    ssize_t bytesReceived = recvfrom(socket,
        buffer, bufferSize-1,
        MSG_DONTWAIT,  // non-blocking
        reinterpret_cast<sockaddr*>(&senderAddr),
        &addrLen
    );
    if (bytesReceived < 0) {
        if (errno != EAGAIN && errno != EWOULDBLOCK) {
            perror("recvfrom");
        }
        return; // nothing to read right now
    }
    buffer[bytesReceived] = '\0';
    std::string mac(buffer);

    // ignore own heartbeats
    for (const InterfaceInfo& iface : broadcastCapableInterfaces) {
        if (mac == iface.macAddress) {
            return;
        }
    }

    char ipStr[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &senderAddr.sin_addr, ipStr, sizeof(ipStr));

    // update or insert neighbour
    auto it = aliveNeighbours.find(mac);
    if (it == aliveNeighbours.end()) {
        ServiceNode newNode(senderAddr.sin_addr, mac);
        aliveNeighbours.emplace(mac, newNode);
        std::cout << "Discovered new neighbour " << mac
                  << " at " << ipStr << std::endl;
    } else {
        it->second.resetLastAliveTimeStamp();
        it->second.updateIpAddress(senderAddr.sin_addr); // just in case ip has dynamically changed
        std::cout << "Refreshed neighbour " << mac
                  << " at " << ipStr << std::endl;
    }
}


