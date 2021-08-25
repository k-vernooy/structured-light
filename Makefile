CXX := g++
CXXFLAGS := -std=c++11 -O3

SRC := src
BIN := bin
BUILD := build

OPENCV := `pkg-config --libs --cflags opencv4`
LIBS := $(OPENCV)
CXXFLAGS += $(LIBS)

OBJECTS := $(shell find $(SRC) -name *.cpp)
OBJECTS_DESTS := $(patsubst $(SRC)/%.cpp, $(BUILD)/%.o, $(OBJECTS))

all: bin/capture

bin/capture: src/Capture.cpp $(OBJECTS_DESTS)
	$(CXX) $(CXXFLAGS) $(OBJECTS_DESTS) -o $@

$(BUILD)/%.o: $(SRC)/%.cpp | setup
	$(CXX) $(CXXFLAGS) $^ -c -o $@

setup:
	mkdir -p $(BIN) $(BUILD)

clean:
	rm -rf $(BIN) $(BUILD)