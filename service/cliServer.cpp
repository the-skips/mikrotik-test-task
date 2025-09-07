#include "cliServer.h"

#include <iostream>

#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <fcntl.h>
#include <cerrno>

using namespace std;

CliServer::CliServer(const string& socketPath) : cliSocket(-1), path(socketPath) {
}

CliServer::~CliServer() {
    if (cliSocket >= 0) close(cliSocket);
    unlink(path.c_str());
}

void CliServer::setup() {
    unlink(path.c_str());

    cliSocket = socket(AF_UNIX, SOCK_STREAM, 0);
    if (cliSocket < 0) {
        perror("socket(AF_UNIX)");
        exit(1);
    }

    struct sockaddr_un addr{};
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, path.c_str(), sizeof(addr.sun_path) - 1);

    if (bind(cliSocket, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr)) < 0) {
        perror("bind(cliSocket)");
        close(cliSocket);
        exit(1);
    }

    if (listen(cliSocket, 5) < 0) {
        perror("listen(cliSocket)");
        close(cliSocket);
        exit(1);
    }

    int flags = fcntl(cliSocket, F_GETFL, 0);
    fcntl(cliSocket, F_SETFL, flags | O_NONBLOCK);

    cout << "CLI socket listening at " << path << endl;
}

void CliServer::serverLoop(const std::unordered_map<std::string, ServiceNode>& neighbours) const {
    int clientFd = accept(cliSocket, nullptr, nullptr);
    if (clientFd < 0) {
        if (errno != EAGAIN && errno != EWOULDBLOCK)
            perror("accept");
        return;
    }
    sendNeighbourList(clientFd, neighbours);
    close(clientFd);
}

void CliServer::sendNeighbourList(int clientFd, const unordered_map<string, ServiceNode>& neighbours) const {
    for (const auto& kv : neighbours) {
        string line = kv.second.getIpAddress() + " " + kv.second.getMacAddress() + "\n";
        if (write(clientFd, line.c_str(), line.length()) < 0)
            perror("write to CLI");
    }
}
