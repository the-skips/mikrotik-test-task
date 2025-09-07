#include "serviceNode.h"

#include <arpa/inet.h>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>

using namespace std;
using namespace std::chrono;
using TimePoint = chrono::time_point<chrono::steady_clock>;

static uint32_t getSecondsPassedBetweenTwoPoints(TimePoint from, TimePoint to) {
    if (to > from) return std::chrono::duration<double>(to - from).count();
    else return std::chrono::duration<double>(from - to).count();
}

ServiceNode::ServiceNode(string ipAddr, string macAddress): macAddress(macAddress), whenWasLastAlive(steady_clock::now()) {
    if (inet_aton(ipAddr.c_str(), &ipAddress) == 0) {
        cerr << "Invalid ip address supplied to ServiceNode: " << ipAddr << std::endl;
        inet_aton("0.0.0.0", &ipAddress); // at least it will be some known state, instead of garbage
    }
}

ServiceNode::ServiceNode(in_addr ipAddr, std::string macAddr): ipAddress(ipAddr), macAddress(macAddr) {
}

uint32_t ServiceNode::getSecondsPassedSinceLastActivity() {
    return getSecondsPassedBetweenTwoPoints(whenWasLastAlive, steady_clock::now());
}

void ServiceNode::resetLastAliveTimeStamp() { whenWasLastAlive = std::chrono::steady_clock::now(); }

void ServiceNode::updateIpAddress(in_addr newIp) { ipAddress = newIp; }

string ServiceNode::getIpAddress() const {
    char ipAddr[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &ipAddress, ipAddr, INET_ADDRSTRLEN);
    return string(ipAddr);
}
string ServiceNode::getMacAddress() const { return macAddress; }

