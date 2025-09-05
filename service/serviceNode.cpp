#include "serviceNode.h"

#include <chrono>
#include <cstdint>

using namespace std;
using namespace std::chrono;
using TimePoint = chrono::time_point<chrono::steady_clock>;

static uint32_t getSecondsPassedBetweenTwoPoints(TimePoint from, TimePoint to) {
    if (to > from) return std::chrono::duration<double>(to - from).count();
    else return std::chrono::duration<double>(from - to).count();
}

ServiceNode::ServiceNode(string ipAddress, string macAddress): ipAddress(ipAddress), macAddress(macAddress), whenWasLastAlive(steady_clock::now()) {

}

uint32_t ServiceNode::getSecondsPassedSinceLastActivity() {
    return getSecondsPassedBetweenTwoPoints(whenWasLastAlive, steady_clock::now());
}

void ServiceNode::resetLastAliveTimeStamp() { whenWasLastAlive = std::chrono::steady_clock::now(); }

string ServiceNode::getIpAddress() { return ipAddress; }
string ServiceNode::getMacAddress() { return macAddress; }

