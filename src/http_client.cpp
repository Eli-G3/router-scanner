#include "http_client.h"
#include <iostream>
#include <string>
#include <regex>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <cstring>

namespace http_client {

void fetch_and_print_xml(const std::string& url) {
    // Basic regex to parse http://host:port/path
    std::regex url_regex(R"(^http://([^:/]+)(?::(\d+))?(/.*)?$)");
    std::smatch url_match;

    if (!std::regex_match(url, url_match, url_regex)) {
        std::cerr << "Invalid or unsupported URL format: " << url << std::endl;
        return;
    }

    std::string host = url_match[1];
    std::string port_str = url_match[2].matched ? url_match[2].str() : "80";
    std::string path = url_match[3].matched ? url_match[3].str() : "/";

    struct addrinfo hints, *res;
    std::memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(host.c_str(), port_str.c_str(), &hints, &res) != 0) {
        std::cerr << "Failed to resolve host: " << host << std::endl;
        return;
    }

    int sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sock < 0) {
        std::cerr << "Failed to create TCP socket." << std::endl;
        freeaddrinfo(res);
        return;
    }

    // Set timeout to prevent hanging forever
    struct timeval tv;
    tv.tv_sec = 5;
    tv.tv_usec = 0;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);
    setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (const char*)&tv, sizeof tv);

    if (connect(sock, res->ai_addr, res->ai_addrlen) < 0) {
        std::cerr << "Failed to connect to " << host << ":" << port_str << std::endl;
        close(sock);
        freeaddrinfo(res);
        return;
    }
    freeaddrinfo(res);

    std::string request = "GET " + path + " HTTP/1.1\r\n" +
                          "Host: " + host + "\r\n" +
                          "Connection: close\r\n\r\n";

    if (send(sock, request.c_str(), request.length(), 0) < 0) {
        std::cerr << "Failed to send HTTP GET request." << std::endl;
        close(sock);
        return;
    }

    std::cout << "\n--- Fetching XML from " << url << " ---\n";

    char buffer[4096];
    ssize_t bytes_received;
    while ((bytes_received = recv(sock, buffer, sizeof(buffer) - 1, 0)) > 0) {
        buffer[bytes_received] = '\0';
        std::cout << buffer;
    }

    std::cout << "\n--- End XML ---\n" << std::endl;

    close(sock);
}

} // namespace http_client
