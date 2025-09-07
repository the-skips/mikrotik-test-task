#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <iostream>
#include <cstring>

using namespace std;

// globals
constexpr const char* CLI_SOCKET_PATH = "/tmp/neighbourService.sock";
constexpr size_t BUFFER_SIZE = 1024;

// function prototypes
static int connectToService();
static bool sendRequest(int sock);
static string receiveResponse(int sock);

// functions themselves
int main() {
    int sock = connectToService();
    if (sock < 0) return 1;
    if (!sendRequest(sock)) {
        close(sock);
        return 1;
    }
    string response = receiveResponse(sock);
    if (response.empty()) {
        cout << "No active neighbours.\n";
    } else {
        cout << "Active neighbours:\n" << response << endl;
    }

    close(sock);
    return 0;
}

static int connectToService() {
    int sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("socket");
        return -1;
    }

    struct sockaddr_un addr{};
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, CLI_SOCKET_PATH, sizeof(addr.sun_path) - 1);

    if (connect(sock, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr)) < 0) {
        perror("connect");
        close(sock);
        return -1;
    }

    return sock;
}

static bool sendRequest(int sock) {
    if (write(sock, "", 0) < 0) {
        perror("write");
        return false;
    }
    return true;
}

string receiveResponse(int sock) {
    char buffer[BUFFER_SIZE];
    ssize_t bytesRead = read(sock, buffer, sizeof(buffer) - 1);
    if (bytesRead < 0) {
        perror("read");
        return string();
    }
    buffer[bytesRead] = '\0';
    return string(buffer);
}


