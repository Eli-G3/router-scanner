#include "ssdp_client.h"
#include "constants.h"
#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cerrno>

SSDPClient::SSDPClient(Reactor& reactor, const std::string& name)
    : reactor_(reactor), name_(name), sock_(-1) {

    // Create a UDP socket
    sock_ = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock_ < 0) {
        std::cerr << "[" << name_ << "] Failed to create socket." << std::endl;
        return;
    }

    // Allow multiple sockets to use the same PORT number
    int reuse = 1;
    if (setsockopt(sock_, SOL_SOCKET, SO_REUSEADDR, (char*)&reuse, sizeof(reuse)) < 0) {
        std::cerr << "[" << name_ << "] Setting SO_REUSEADDR error" << std::endl;
    }
#ifdef SO_REUSEPORT
    if (setsockopt(sock_, SOL_SOCKET, SO_REUSEPORT, (char*)&reuse, sizeof(reuse)) < 0) {
        // Proceed even if it fails, as some systems might not support it
    }
#endif

    // Register with reactor
    reactor_.register_fd(sock_, [this](int fd) { this->handle_read(fd); });
}

SSDPClient::~SSDPClient() {
    if (sock_ >= 0) {
        reactor_.unregister_fd(sock_);
        close(sock_);
    }
}

void SSDPClient::send_search_request() {
    if (sock_ < 0) return;

    // Setup destination address
    struct sockaddr_in dest_addr;
    std::memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(constants::SSDP_PORT);

    if (inet_pton(AF_INET, constants::SSDP_IP.c_str(), &dest_addr.sin_addr) <= 0) {
        std::cerr << "[" << name_ << "] Invalid address/ Address not supported" << std::endl;
        return;
    }

    // Send the request
    if (sendto(sock_, constants::M_SEARCH_REQ.c_str(), constants::M_SEARCH_REQ.length(), 0,
               (struct sockaddr*)&dest_addr, sizeof(dest_addr)) < 0) {
        std::cerr << "[" << name_ << "] Failed to send M-SEARCH request" << std::endl;
        return;
    }

    std::cout << "[" << name_ << "] Sent SSDP M-SEARCH request. Listening for responses..." << std::endl;
    std::cout << "----------------------------------------------------" << std::endl;
}

void SSDPClient::handle_read(int fd) {
    char buffer[constants::BUFFER_SIZE];
    struct sockaddr_in sender_addr;
    socklen_t sender_addr_len = sizeof(sender_addr);

    std::memset(buffer, 0, sizeof(buffer));
    ssize_t bytes_received = recvfrom(fd, buffer, sizeof(buffer) - 1, 0,
                                      (struct sockaddr*)&sender_addr, &sender_addr_len);

    if (bytes_received < 0) {
        if (errno != EAGAIN && errno != EWOULDBLOCK) {
            std::cerr << "[" << name_ << "] Error receiving data. errno: " << errno << std::endl;
        }
        return;
    }

    // Print sender IP and port
    char sender_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &sender_addr.sin_addr, sender_ip, INET_ADDRSTRLEN);
    std::cout << "[" << name_ << "] Response from " << sender_ip << ":" << ntohs(sender_addr.sin_port) << std::endl;

    // Print the received data
    std::cout << buffer << std::endl;
    std::cout << "----------------------------------------------------" << std::endl;
}
