#include "reactor.h"
#include "ssdp_client.h"
#include "constants.h"
#include <iostream>
#include <thread>
#include <chrono>

int main() {
    Reactor reactor;

    // Create multiple SSDP clients sharing the same reactor event loop
    SSDPClient client1(reactor, "Client-1");
    SSDPClient client2(reactor, "Client-2");

    // Send search requests
    client1.send_search_request();

    // Delay slightly to test independence
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    client2.send_search_request();

    // In a real application, you might stop the reactor based on a condition
    // For this example, we'll run it in a thread and stop it after our timeout
    std::thread reactor_thread([&reactor]() {
        reactor.run();
    });

    std::this_thread::sleep_for(std::chrono::seconds(constants::RECEIVE_TIMEOUT_SEC));
    std::cout << "Discovery timeout reached. Stopping reactor." << std::endl;
    reactor.stop();

    if (reactor_thread.joinable()) {
        reactor_thread.join();
    }

    return 0;
}
