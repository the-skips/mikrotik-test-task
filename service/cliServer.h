#ifndef CLI_SERVER_H
#define CLI_SERVER_H

#include "serviceNode.h"

#include <unordered_map>
#include <string>

struct CliServer {
    CliServer(const std::string& socketPath);
    ~CliServer();

    void setup();
    void serverLoop(const std::unordered_map<std::string, ServiceNode>& neighbours) const;

private:
    int cliSocket;
    std::string path;
    void sendNeighbourList(int clientFd, const std::unordered_map<std::string, ServiceNode>& neighbours) const;
};

#endif // CLI_SERVER_H

