CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -pthread -Iinclude

TARGET = build/ssdp_client
SRCS = src/main.cpp src/reactor.cpp src/ssdp_client.cpp src/http_client.cpp
OBJS = $(patsubst src/%.cpp,build/%.o,$(SRCS))

all: build_dir $(TARGET)

build_dir:
	mkdir -p build

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)

build/%.o: src/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf build/*
