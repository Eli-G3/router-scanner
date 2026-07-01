#pragma once

#include <vector>
#include <functional>
#include <unordered_map>
#include <poll.h>

class Reactor {
public:
    using Callback = std::function<void(int)>;

    Reactor();
    ~Reactor() = default;

    // Register a file descriptor with a read callback
    void register_fd(int fd, Callback callback);

    // Unregister a file descriptor
    void unregister_fd(int fd);

    // Run the event loop
    void run();

    // Stop the event loop
    void stop();

private:
    std::unordered_map<int, Callback> callbacks_;
    std::vector<struct pollfd> pollfds_;
    bool running_;
};
