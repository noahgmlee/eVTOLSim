# eVTOLSim
eVTOL-simulation

# architectural diagrams
preliminary design <br />
<img src="diagrams/prelimDesign.png" alt="drawing" width="300"/> <br />
final architecture <br />
<img src="diagrams/finalDesign.png" style="transform:rotate(90deg);" alt="drawing" width="300"/> <br />


# assumptions/decisions
- Calculations <br />
As per the instructions logging is done per vehicle TYPE <br />
for avg distance per flight, time per flight etc. these are only updated on flight completion. Otherwise a Sim that terminates mid flight would skew average calculations <br />
a dt of 1 minute is passed to "tick" the simulation forward. finer fidelity is easy to modify in the Simulation class but due to the simplicity/deterministic of the simulation: (always fly at constant speed, charge time is deterministic) a smaller time step does not offer much more in model validation. <br />

- Architectural <br />
for this problem eVTOL's of type Alpha <-> Echo have the same properties so one eVTOL class is used <br />
realistically the properties would vary and an inheritance structure out of eVTOL would make sense <br />
a shared memory struct is maintained in ShMem.hpp and instantiated in ShMem.cpp. This structure allows for message passing from each aircraft to a "Logger" module, as well as message passing between the ChargeManager and aircrafts. Thread safety was not added to shMem access, but is a TODO <br />
The simulation engine holds a vector of components to update at each tick. Components are attached to the simulation as std::function<void(TimeS)> objects. This allows simple lambdas to be passed in as "components" or polymorphism can be leveraged to pass children of SimObj through the addObject(SimObj*) API.

# build & run instructions
a Makefile is provided to build and run the eVTOL simulation <br />
make - build the simulation binary <br />
make test - build the test binary (unit tests, integration tests) <br />
make run - run the simulation binary (must build it first) <br />
make runtests - run the test binary <br />
I've added the launch.json file I use to run the sim or test binary with the integrated vscode debugger <br />

# testability
building for the test target depends on the googletest (gtest) framework <br />
unit tests are written for each object <br />
comments are left in the test.cpp file explaining refactoring decisions that are "TODO" <br />
