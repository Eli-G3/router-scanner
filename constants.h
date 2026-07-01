#pragma once

#include <string>

namespace constants {
    inline const std::string SSDP_IP = "239.255.255.250";
    constexpr int SSDP_PORT = 1900;
    constexpr int RECEIVE_TIMEOUT_SEC = 3;
    constexpr int BUFFER_SIZE = 4096;

    inline const std::string M_SEARCH_REQ =
        "M-SEARCH * HTTP/1.1\r\n"
        "HOST: 239.255.255.250:1900\r\n"
        "MAN: \"ssdp:discover\"\r\n"
        "MX: 1\r\n"
        "ST: ssdp:all\r\n"
        "\r\n";
}
