CXX := g++
CXXFLAGS := -std=c++11 -O3 -MMD -MP 

SRC := src
BIN := bin
PRG := prg

BUILDPRG := build/prg
BUILDSRC := build/src

OPENCV := `pkg-config --libs --cflags opencv4`

SPINDIR := /opt/spinnaker
SPINNAKER := -L$(SPINDIR)/lib/ -l Spinnaker -I$(SPINDIR)/include/

LIBS := $(OPENCV) $(SPINNAKER)
CXXFLAGS += $(LIBS)


OBJECTSSRC := $(shell find $(SRC) -name *.cpp)
OBJECTSPRG := $(shell find $(PRG) -name *.cpp)

OBJECTS_DESTS := $(patsubst $(SRC)/%.cpp, $(BUILDSRC)/%.o, $(OBJECTSSRC))

DEPENDS_DESTS := $(patsubst $(SRC)/%.cpp, $(BUILDSRC)/%.d, $(OBJECTSSRC))
DEPENDS_DESTS += $(patsubst $(PRG)/%.cpp, $(BUILDPRG)/%.d, $(OBJECTSPRG))



all: bin/capture


bin/%: $(BUILDPRG)/%.o $(OBJECTS_DESTS)
	$(CXX) $(CXXFLAGS) $(OBJECTS_DESTS) $< -o $@

$(BUILDPRG)/%.o: $(PRG)/%.cpp | setup
	$(CXX) $(CXXFLAGS) $< -MT $@ -c -o $@

$(BUILDSRC)/%.o: $(SRC)/%.cpp | setup
	$(CXX) $(CXXFLAGS) $< -MT $@ -c -o $@


setup:
	mkdir -p $(BIN) $(BUILDPRG) $(BUILDSRC)

clean:
	rm -rf $(BIN) $(BUILDPRG) $(BUILDSRC)

-include $(DEPENDS_DESTS)