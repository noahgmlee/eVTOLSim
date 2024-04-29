CC = g++
CXXFLAGS = -g -Wall -std=c++14 -Isrc/Components -Isrc/Simulation
BIN_DIR = bin

AIRCRAFT_SRCDIR = src/Components
SIMULATION_SRCDIR = src/Simulation
AIRCRAFT_SRCS = $(wildcard src/Components/*.cpp)
SIMULATION_SRCS = $(wildcard src/Simulation/*.cpp)
MAIN_SRC=src/main.cpp
TEST_SRC=test/tests.cpp
AIRCRAFT_BINDIR = $(BIN_DIR)
SIMULATION_BINDIR = $(BIN_DIR)
AIRCRAFT_OBJS := $(patsubst $(AIRCRAFT_SRCDIR)/%.cpp,$(AIRCRAFT_BINDIR)/%.o,$(AIRCRAFT_SRCS))
SIMULATION_OBJS := $(patsubst $(SIMULATION_SRCDIR)/%.cpp,$(SIMULATION_BINDIR)/%.o,$(SIMULATION_SRCS))

all: $(BIN_DIR) $(BIN_DIR)/eVTOLsim

test: $(BIN_DIR) $(BIN_DIR)/test

$(BIN_DIR)/eVTOLsim: $(AIRCRAFT_OBJS) $(SIMULATION_OBJS)
	$(CC) $(CXXFLAGS) -c src/main.cpp -o bin/main.o
	$(CC) $(CXXFLAGS) -o $(BIN_DIR)/eVTOLsim $^ bin/main.o

$(BIN_DIR)/test: $(AIRCRAFT_OBJS) $(SIMULATION_OBJS)
	$(CC) $(CXXFLAGS) -c test/tests.cpp -o bin/tests.o
	$(CC) $(CXXFLAGS) -o $(BIN_DIR)/tests $^ bin/tests.o -lgtest

$(AIRCRAFT_BINDIR)/%.o: $(AIRCRAFT_SRCDIR)/%.cpp
	$(CC) $(CXXFLAGS) -c $< -o $@

$(SIMULATION_BINDIR)/%.o: $(SIMULATION_SRCDIR)/%.cpp
	$(CC) $(CXXFLAGS) -c $< -o $@

$(BIN_DIR):
	mkdir $@

runsim:
	./bin/eVTOLsim > outputs/sim_output.txt

runtests:
	./bin/tests > outputs/test_output.txt

clean:
	rm -rf $(BIN_DIR)
	rm -rf outputs/*
