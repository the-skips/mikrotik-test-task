#include "serviceNode.h"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unordered_map>
#include <unistd.h>
#include <iostream>
#include <ifaddrs.h>
#include <arpa/inet.h>
#include <vector>

using namespace std;



static unordered_map<string, ServiceNode> aliveNeighbours;
static vector<int> inetSockets;

static void removeOldNeighbours(unordered_map<string, ServiceNode> map) {
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

void setup() {
    ServiceNode testObject("testIpAddress", "testMacAddress");
    aliveNeighbours.insert({testObject.getMacAddress(), testObject});
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

