#pragma once

#include <string>

namespace http_client {
    // Parse the URL, send a GET request, and print the response to stdout.
    void fetch_and_print_xml(const std::string& url);
}
