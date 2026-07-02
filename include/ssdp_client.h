#pragma once

#include "reactor.h"
#include <string>

class SSDPClient {
public:
    SSDPClient(Reactor& reactor, const std::string& name);
    ~SSDPClient();

    // Sends the M-SEARCH broadcast
    void send_search_request();

private:
    void handle_read(int fd);

    Reactor& reactor_;
    std::string name_;
    int sock_;
};
