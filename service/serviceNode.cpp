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

uint32_t ServiceNode::getSecondsPassedSinceLastActivity() {
    return getSecondsPassedBetweenTwoPoints(whenWasLastAlive, steady_clock::now());
}

void ServiceNode::resetLastAliveTimeStamp() { whenWasLastAlive = std::chrono::steady_clock::now(); }

string ServiceNode::getIpAddress() {
    char ipAddr[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &ipAddress, ipAddr, INET_ADDRSTRLEN);
    return string(ipAddr);
}
string ServiceNode::getMacAddress() { return macAddress; }

