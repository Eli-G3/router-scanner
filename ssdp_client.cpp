#include <iostream>
#include <string>
#include <cstring>
#include <vector>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/time.h>
#include <cerrno>

int main() {
    const std::string ssdp_ip = "239.255.255.250";
    const int ssdp_port = 1900;

    // Create a UDP socket
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        std::cerr << "Failed to create socket." << std::endl;
        return 1;
    }

    // Allow multiple sockets to use the same PORT number
    int reuse = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char*)&reuse, sizeof(reuse)) < 0) {
        std::cerr << "Setting SO_REUSEADDR error" << std::endl;
        close(sock);
        return 1;
    }
#ifdef SO_REUSEPORT
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEPORT, (char*)&reuse, sizeof(reuse)) < 0) {
        // Proceed even if it fails, as some systems might not support it
    }
#endif

    // Setup destination address
    struct sockaddr_in dest_addr;
    std::memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(ssdp_port);
    if (inet_pton(AF_INET, ssdp_ip.c_str(), &dest_addr.sin_addr) <= 0) {
        std::cerr << "Invalid address/ Address not supported" << std::endl;
        close(sock);
        return 1;
    }

    // Set receive timeout
    struct timeval tv;
    tv.tv_sec = 3;
    tv.tv_usec = 0;
    if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
        std::cerr << "Error setting timeout" << std::endl;
        close(sock);
        return 1;
    }

    // M-SEARCH payload
    std::string m_search_req =
        "M-SEARCH * HTTP/1.1\r\n"
        "HOST: 239.255.255.250:1900\r\n"
        "MAN: \"ssdp:discover\"\r\n"
        "MX: 1\r\n"
        "ST: ssdp:all\r\n"
        "\r\n";

    // Send the request
    if (sendto(sock, m_search_req.c_str(), m_search_req.length(), 0,
               (struct sockaddr*)&dest_addr, sizeof(dest_addr)) < 0) {
        std::cerr << "Failed to send M-SEARCH request" << std::endl;
        close(sock);
        return 1;
    }

    std::cout << "Sent SSDP M-SEARCH request. Listening for responses..." << std::endl;
    std::cout << "----------------------------------------------------" << std::endl;

    // Buffer to hold responses
    char buffer[4096];
    struct sockaddr_in sender_addr;
    socklen_t sender_addr_len = sizeof(sender_addr);

    // Loop to receive responses
    while (true) {
        std::memset(buffer, 0, sizeof(buffer));
        ssize_t bytes_received = recvfrom(sock, buffer, sizeof(buffer) - 1, 0,
                                          (struct sockaddr*)&sender_addr, &sender_addr_len);

        if (bytes_received < 0) {
            // Check if it's a timeout or an error
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                std::cout << "Discovery timeout reached. Exiting." << std::endl;
            } else {
                std::cerr << "Error receiving data. errno: " << errno << std::endl;
            }
            break;
        }

        // Print sender IP and port
        char sender_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &sender_addr.sin_addr, sender_ip, INET_ADDRSTRLEN);
        std::cout << "Response from " << sender_ip << ":" << ntohs(sender_addr.sin_port) << std::endl;

        // Print the received data
        std::cout << buffer << std::endl;
        std::cout << "----------------------------------------------------" << std::endl;
    }

    close(sock);
    return 0;
}
