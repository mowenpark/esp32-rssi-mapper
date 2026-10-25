CXX ?= c++
CXXFLAGS = -std=c++17 -Wall -Wextra -Irssi_meter
SOURCES = $(wildcard test/*.cpp) rssi_meter/signal_level.cpp rssi_meter/selection.cpp rssi_meter/scan_state.cpp

.PHONY: test
test: build/tests
	./build/tests

build/tests: $(SOURCES) $(wildcard test/*.h) $(wildcard rssi_meter/*.h)
	@mkdir -p build
	$(CXX) $(CXXFLAGS) $(SOURCES) -o $@
