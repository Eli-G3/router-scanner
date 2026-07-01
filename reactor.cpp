#include "reactor.h"
#include <iostream>
#include <algorithm>

Reactor::Reactor() : running_(false) {}

void Reactor::register_fd(int fd, Callback callback) {
    callbacks_[fd] = std::move(callback);

    struct pollfd pfd;
    pfd.fd = fd;
    pfd.events = POLLIN;
    pfd.revents = 0;
    pollfds_.push_back(pfd);
}

void Reactor::unregister_fd(int fd) {
    callbacks_.erase(fd);
    pollfds_.erase(std::remove_if(pollfds_.begin(), pollfds_.end(),
                                  [fd](const struct pollfd& pfd) { return pfd.fd == fd; }),
                   pollfds_.end());
}

void Reactor::run() {
    running_ = true;
    while (running_) {
        if (pollfds_.empty()) {
            break; // Nothing to wait for
        }

        // Wait for events indefinitely (or until stopped)
        // using timeout 1000ms so we can periodically check running_ flag.
        int ret = poll(pollfds_.data(), pollfds_.size(), 1000);

        if (ret < 0) {
            std::cerr << "Poll error" << std::endl;
            break;
        }

        if (ret == 0) {
            // Timeout, loop around to check running_
            continue;
        }

        // Create a copy of the current FDs we need to process,
        // because callbacks might modify pollfds_ via unregister_fd.
        std::vector<struct pollfd> current_fds = pollfds_;

        for (const auto& pfd : current_fds) {
            if (pfd.revents & POLLIN) {
                auto it = callbacks_.find(pfd.fd);
                if (it != callbacks_.end()) {
                    it->second(pfd.fd);
                }
            }
        }
    }
}

void Reactor::stop() {
    running_ = false;
}
