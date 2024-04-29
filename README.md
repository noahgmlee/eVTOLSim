# eVTOLSim
eVTOL-simulation

# architectural diagrams
<img src="diagrams/prelimDesign.png" alt="drawing" width="300"/>

# assumptions/decisions
for this problem eVTOL's of type Alpha <-> Echo have the same properties so one eVTOL class is used
- realistically the properties would vary and an inheritance structure out of eVTOL would make sense

# build & run instructions
a Makefile is provided to build and run the eVTOL simulation
make - build the simulation binary
make test - build the test binary (unit tests, integration tests)
make run - run the simulation binary (must build it first)
make runtests - run the test binary

# testability
building for the test target depends on the googletest (gtest) framework
unit tests are written for each object (all member functions)
