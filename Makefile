CXX := g++
CXXFLAGS := -std=c++11 -O3 -MMD -MP 

SRC := src
BIN := bin
BUILD := build

OPENCV := `pkg-config --libs --cflags opencv4`
LIBS := $(OPENCV)
CXXFLAGS += $(LIBS)

OBJECTS := $(shell find $(SRC) -name *.cpp)
OBJECTS_DESTS := $(patsubst $(SRC)/%.cpp, $(BUILD)/%.o, $(OBJECTS))
DEPENDS_DESTS := $(patsubst $(SRC)/%.cpp, $(BUILD)/%.d, $(OBJECTS))

all: bin/capture

bin/capture: src/Capture.cpp $(OBJECTS_DESTS)
	$(CXX) $(CXXFLAGS) $(OBJECTS_DESTS) -o $@

$(BUILD)/%.o: $(SRC)/%.cpp | setup
	$(CXX) $(CXXFLAGS) $< -MT $@ -c -o $@

setup:
	mkdir -p $(BIN) $(BUILD)

clean:
	rm -rf $(BIN) $(BUILD)

-include $(DEPENDS_DESTS)