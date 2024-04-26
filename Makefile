CC = g++
CXXFLAGS = -g -Wall -std=c++14
BIN_DIR = bin

AIRCRAFT_SRCS = $(wildcard src/Aircrafts/*.cpp)
SIMULATION_SRCS = $(wildcard src/Simulation/*.cpp)
MAIN_SRC=src/main.cpp
TEST_SRC=test/tests.cpp
AIRCRAFT_OBJS := $(patsubst src/Aircrafts/%.cpp,$(BIN_DIR)/%.o,$(AIRCRAFT_SRCS))
SIMULATION_OBJS := $(patsubst src/Simulation/%.cpp,$(BIN_DIR)/%.o,$(SIMULATION_SRCS))

all: $(BIN_DIR) $(BIN_DIR)/eVTOLsim

test: $(BIN_DIR) $(BIN_DIR)/test

$(BIN_DIR)/eVTOLsim: $(AIRCRAFT_OBJS) $(SIMULATION_OBJS)
	$(CC) $(CXXFLAGS) -c src/main.cpp -o bin/main.o
	$(CC) $(CXXFLAGS) -o $(BIN_DIR)/eVTOLsim $^ bin/main.o

$(BIN_DIR)/test: $(AIRCRAFT_OBJS) $(SIMULATION_OBJS)
	$(CC) $(CXXFLAGS) -c test/tests.cpp -o bin/tests.o
	$(CC) $(CXXFLAGS) -o $(BIN_DIR)/tests $^ bin/tests.o -lgtest

$(AIRCRAFT_OBJS): $(AIRCRAFT_SRCS)
	$(CC) $(CXXFLAGS) -c $< -o $@

$(SIMULATION_OBJS): $(SIMULATION_SRCS)
	$(CC) $(CXXFLAGS) -c $< -o $@

$(BIN_DIR):
	mkdir $@

runsim:
	./bin/eVTOLsim

runtests:
	./bin/tests

clean:
	rm -rf $(BIN_DIR)
