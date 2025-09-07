#ifndef SERVICE_NODE_H
#define SERVICE_NODE_H

#include <string>
#include <chrono>
#include <cstdint>
#include <netinet/in.h>

class ServiceNode {

public:
    ServiceNode(std::string ipAddress, std::string macAddress);

    void resetLastAliveTimeStamp();

    std::string getIpAddress();
    std::string getMacAddress();
    uint32_t getSecondsPassedSinceLastActivity();


protected:

private:
    in_addr ipAddress;
    std::string macAddress;
    std::chrono::time_point<std::chrono::steady_clock> whenWasLastAlive;
};

#endif // SERVICE_NODE_H
