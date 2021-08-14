CXX := g++
CFLAGS := `pkg-config --libs --cflags opencv4` -std=c++11 -O3

all:
	$(CXX) $(CFLAGS) main.cpp -o main