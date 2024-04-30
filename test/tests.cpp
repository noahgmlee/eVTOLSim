#include <gtest/gtest.h>
#include <stdint.h>
#include "SimTypes.hpp"
#include "eVTOL.hpp"
#include "ChargeManager.hpp"
#include "Simulation.hpp"

//can rewrite these tests to be much cleaner in the following ways:
//TODO refactor by adding testhelpers header and implementation to abstract helpful test sequences
//TODO split this file out into src files for test types
//TODO learn gtest parameterized tests to reduce code duplication
//for example a parameterized test that takes EVTOL_TYPE can test each type with one code body

// Sequence:
// 1. instantiate all 5 types of eVTOLs
// 2. ensure through getters characteristic is properly set (cruise speed)
TEST(EVTOL, instantiateEVTOL)
{
    sharedMemory.init();
    eVTOL alpha = eVTOL(EVTOL_TYPE::ALPHA, 0);
    eVTOL bravo = eVTOL(EVTOL_TYPE::BRAVO, 1);
    eVTOL charlie = eVTOL(EVTOL_TYPE::CHARLIE, 2);
    eVTOL delta = eVTOL(EVTOL_TYPE::DELTA, 3);
    eVTOL echo = eVTOL(EVTOL_TYPE::ECHO, 4);
    EXPECT_TRUE(alpha.getSpeed() == 120.0);
    EXPECT_TRUE(bravo.getSpeed() == 100.0);
    EXPECT_TRUE(charlie.getSpeed() == 160.0);
    EXPECT_TRUE(delta.getSpeed() == 90.0);
    EXPECT_TRUE(echo.getSpeed() == 30.0);
}

//fault test too large a dt - battery depletes on a single update
TEST(EVTOL, dtTooBig)
{
    sharedMemory.init();
    eVTOL alpha = eVTOL(EVTOL_TYPE::ALPHA, 0);
    TimeS dt = 2 * 3600; //2 hour dt
    alpha.update(dt);
    EXPECT_FALSE(alpha.getBatteryCapacity() > 0.0);
}

//fault test too small a dt - random fault injections impossible
TEST(EVTOL, dtTooSmall)
{
    sharedMemory.init();
    eVTOL alpha = eVTOL(EVTOL_TYPE::ALPHA, 0);
    alpha.setFaultProb(0.035);
    TimeS dt = 1; //1 sec dt
    alpha.update(dt);
    EXPECT_FALSE(alpha.getFaults() > 0.0);
    //should also log issue to stderr
}

// Sequence:
// 1. instantiate an eVTOL
// 2. set fault probability to 1/2 and ensure full battery capacity at initialization
// 3. simulate 1 minute "tick"
// 4. confirm battery depletion is consistent with 1 minute at cruise
// 5. simulate 59 more minutes and log numFaults - statistically, half of these tests should have a fault
TEST(EVTOL, cruiseUpdate)
{
    sharedMemory.init();
    eVTOL alpha = eVTOL(EVTOL_TYPE::ALPHA, 0);
    alpha.setFaultProb(0.50);
    EXPECT_TRUE(alpha.getBatteryCapacity() == 320.0);
    float expectedBattery = alpha.getBatteryCapacity() - SEC_TO_HRS((TimeS)60) * alpha.getSpeed() * alpha.getEnergyUsage();
    alpha.update(60); //run 1 minute
    EXPECT_TRUE(alpha.getBatteryCapacity() == expectedBattery) << "wrong battery capacity: "
                                                               << alpha.getBatteryCapacity()
                                                               << "\nexpected: "
                                                               << expectedBattery;
    for (int i = 0; i < 59; i ++){
        alpha.update(60);
    }
    std::cout << "fault probability per hour is: " << 0.50 << std::endl;
    std::cout << "num faults recorded after one hour is: " << alpha.getFaults() << std::endl;
}

// Sequence:
// 1. instantiate an eVTOL
// 2. set battery cap to 0 and simulate 1 minute
// 3. confirm battery cap is consistent with 1 minute of charging
// 4. simulate rest of charge and confirm battery cap is consistent with simulation step
TEST(EVTOL, chargeUpdate)
{
    sharedMemory.init();
    TimeS dt = 60;
    eVTOL alpha = eVTOL(EVTOL_TYPE::ALPHA, 0);
    alpha.setState(EVTOL_STATE::GROUNDED_CHARGING);
    alpha.setBatteryCap(0.0);
    alpha.update(dt); //run 1 minute
    float batteryCap = 0.0;
    batteryCap += (float)SEC_TO_HRS(dt) * 1.0/(float)0.6 * (float)320.0;
    EXPECT_TRUE(abs(alpha.getBatteryCapacity() - batteryCap) < 0.00001) << "wrong battery capacity: "
                                                               << alpha.getBatteryCapacity()
                                                               << "\nexpected: "
                                                               << batteryCap;
    for (int i = 0; i < 3; i ++){
        alpha.update(dt);
        batteryCap += (float)SEC_TO_HRS(dt) * 1.0/(float)0.6 * (float)320.0;
        EXPECT_TRUE(abs(alpha.getBatteryCapacity() - batteryCap) < 0.00001) << "wrong battery capacity: "
                                                                << alpha.getBatteryCapacity()
                                                                << "\nexpected: "
                                                                << batteryCap;
    }
    while (batteryCap < (320.0 * 0.99)){
        alpha.update(dt);
        batteryCap += (float)SEC_TO_HRS(dt) * 1.0/(float)0.6 * (float)320.0;
    }
    EXPECT_TRUE(abs(alpha.getBatteryCapacity() - 320.0) < 0.00001) << "wrong battery capacity: "
                                                     << alpha.getBatteryCapacity()
                                                     << "\nexpected: "
                                                     << 320.0;
}

//TODO at this point I pushed to get the SimEngine done and was more lenient with unit testing the rest of my eVTOL object
//in practice I would ensure full code coverage of the eVTOL object

void tickSim(std::vector<SimObj*>& components, TimeS dt)
{
    for (auto component: components)
    {
        component->update(dt);
    }
}

// Sequence:
// 1. enqueue one of each aircraft in the simulation and instantiate the charge manager
// 2. place all aircrafts in the charge queue
// 3. ensure first three air crafts int the queue are given priority on available chargers
// 4. ensure remaining queued air crafts are given chargers as they become available
TEST(CHARGEMANAGER, chargeQueue)
{
    sharedMemory.init();
    TimeS dt = 60;
    std::vector<SimObj*> components;
    eVTOL alpha = eVTOL(EVTOL_TYPE::ALPHA, 0);
    eVTOL bravo = eVTOL(EVTOL_TYPE::BRAVO, 1);
    eVTOL charlie = eVTOL(EVTOL_TYPE::CHARLIE, 2);
    eVTOL delta = eVTOL(EVTOL_TYPE::DELTA, 3);
    eVTOL echo = eVTOL(EVTOL_TYPE::ECHO, 4);
    alpha.setState(EVTOL_STATE::GROUNDED_WAITING);
    bravo.setState(EVTOL_STATE::GROUNDED_WAITING);
    charlie.setState(EVTOL_STATE::GROUNDED_WAITING);
    delta.setState(EVTOL_STATE::GROUNDED_WAITING);
    echo.setState(EVTOL_STATE::GROUNDED_WAITING);
    components = {&alpha, &bravo, &charlie, &delta, &echo};

    ChargeManager chargeMngr = ChargeManager();

    //queue all chargers, ensure first three change state, last two stay queued
    tickSim(components, dt); //tick 1 enqueues and assigns chargers
    chargeMngr.update(dt);
    tickSim(components, dt); //tick 2 changes state to charging
    chargeMngr.update(dt);
    EXPECT_TRUE(alpha.getState() == EVTOL_STATE::GROUNDED_CHARGING);
    EXPECT_TRUE(bravo.getState() == EVTOL_STATE::GROUNDED_CHARGING);
    EXPECT_TRUE(charlie.getState() == EVTOL_STATE::GROUNDED_CHARGING);
    EXPECT_TRUE(delta.getState() == EVTOL_STATE::GROUNDED_WAITING);
    EXPECT_TRUE(echo.getState() == EVTOL_STATE::GROUNDED_WAITING);
    tickSim(components, dt);
    chargeMngr.update(dt);
    tickSim(components, dt);
    chargeMngr.update(dt);
    EXPECT_TRUE(alpha.getState() == EVTOL_STATE::CRUISING);
    EXPECT_TRUE(bravo.getState() == EVTOL_STATE::CRUISING);
    EXPECT_TRUE(charlie.getState() == EVTOL_STATE::CRUISING);
    EXPECT_TRUE(delta.getState() == EVTOL_STATE::GROUNDED_CHARGING);
    EXPECT_TRUE(echo.getState() == EVTOL_STATE::GROUNDED_CHARGING);
}

TEST(SIMULATION, SimEngineBaddt)
{
    Simulation faultSim(0, 3600);
    EXPECT_FALSE(faultSim.getDt() == 0);
    //stderr will also indicate improper use of Simulation class
}

// infra test of SimEngine
// Sequence:
// 1. attach 3 lambdas to the simulation: 1 and 3 as batch1 priority, 2 as batch2
// 2. run 10 ticks and ensure lambda 1 and 3 are run with batch 1 and lambda 2 with batch2
TEST(SIMULATION, SimEngineSimple)
{
    sharedMemory.init();
    Simulation simEngine = Simulation(60, 60 * 10); //run 10 ticks
    int i1 = 0, i2 = 0, i3 = 0;
    std::function<void(TimeS)> lambda1 = [&i1](TimeS dt) { std::cout << i1 << ": I am lambda 1, batch1!" << std::endl; i1++; };
    std::function<void(TimeS)> lambda2 = [&i2, &i3](TimeS dt) { std::cout << i2 << ": I am lambda 2, batch2!" << std::endl; EXPECT_TRUE(i3 > i2); i2++; };
    std::function<void(TimeS)> lambda3 = [&i3](TimeS dt) { std::cout << i3 << ": I am lambda 3, batch1!" << std::endl; i3++; };
    simEngine.addCallable(lambda1, SIM_BATCH::BATCH1);
    simEngine.addCallable(lambda2, SIM_BATCH::BATCH2);
    simEngine.addCallable(lambda3, SIM_BATCH::BATCH1);
    simEngine.start();
}

// Sequence
// 1. add 5 eVTOL's and charge manager to simEngine
// 2. queue the 5 eVTOLs
// 3. ensure charge manager appropriately assigns chargers
TEST(INTEGRATION, SimEngineEVTOL)
{
    sharedMemory.init();
    TimeS dt = 60;
    Simulation simEngine = Simulation(60, 60 * 10); //run 10 ticks
    eVTOL alpha = eVTOL(EVTOL_TYPE::ALPHA, 0);
    eVTOL bravo = eVTOL(EVTOL_TYPE::BRAVO, 1);
    eVTOL charlie = eVTOL(EVTOL_TYPE::CHARLIE, 2);
    eVTOL delta = eVTOL(EVTOL_TYPE::DELTA, 3);
    eVTOL echo = eVTOL(EVTOL_TYPE::ECHO, 4);
    ChargeManager chargeMngr = ChargeManager();
    simEngine.addObject(&alpha);
    simEngine.addObject(&bravo);
    simEngine.addObject(&charlie);
    simEngine.addObject(&delta);
    simEngine.addObject(&echo);
    simEngine.addObject(&chargeMngr);
    simEngine.update(dt);
    EXPECT_TRUE(alpha.getState() == EVTOL_STATE::CRUISING);
    EXPECT_TRUE(bravo.getState() == EVTOL_STATE::CRUISING);
    EXPECT_TRUE(charlie.getState() == EVTOL_STATE::CRUISING);
    EXPECT_TRUE(delta.getState() == EVTOL_STATE::CRUISING);
    EXPECT_TRUE(echo.getState() == EVTOL_STATE::CRUISING);
    alpha.setState(EVTOL_STATE::GROUNDED_WAITING);
    bravo.setState(EVTOL_STATE::GROUNDED_WAITING);
    charlie.setState(EVTOL_STATE::GROUNDED_WAITING);
    delta.setState(EVTOL_STATE::GROUNDED_WAITING);
    echo.setState(EVTOL_STATE::GROUNDED_WAITING);
    simEngine.update(dt);
    simEngine.update(dt);
    EXPECT_TRUE(alpha.getState() == EVTOL_STATE::GROUNDED_CHARGING);
    EXPECT_TRUE(bravo.getState() == EVTOL_STATE::GROUNDED_CHARGING);
    EXPECT_TRUE(charlie.getState() == EVTOL_STATE::GROUNDED_CHARGING);
    EXPECT_TRUE(delta.getState() == EVTOL_STATE::GROUNDED_WAITING);
    EXPECT_TRUE(echo.getState() == EVTOL_STATE::GROUNDED_WAITING);
    simEngine.update(dt);
    simEngine.update(dt);
    EXPECT_TRUE(alpha.getState() == EVTOL_STATE::CRUISING);
    EXPECT_TRUE(bravo.getState() == EVTOL_STATE::CRUISING);
    EXPECT_TRUE(charlie.getState() == EVTOL_STATE::CRUISING);
    EXPECT_TRUE(delta.getState() == EVTOL_STATE::GROUNDED_CHARGING);
    EXPECT_TRUE(echo.getState() == EVTOL_STATE::GROUNDED_CHARGING);
}

// Sequence:
// 1. add 5 eVTOL's, chargeManager, and Logger to the simEngine
// 2. add two batch2 callables to log charger data and aircraft data (shared memory validation)
// 3. simulate 100 ticks
TEST(INTEGRATION, SimEngineOutputCheck)
{
    sharedMemory.init();
    TimeS dt = 60;
    Simulation simEngine = Simulation(dt, dt * 120); //run 120 ticks
    eVTOL alpha = eVTOL(EVTOL_TYPE::ALPHA, 0);
    eVTOL bravo = eVTOL(EVTOL_TYPE::BRAVO, 1);
    eVTOL charlie = eVTOL(EVTOL_TYPE::CHARLIE, 2);
    eVTOL delta = eVTOL(EVTOL_TYPE::DELTA, 3);
    eVTOL echo = eVTOL(EVTOL_TYPE::ECHO, 4);
    ChargeManager chargeMngr = ChargeManager();
    Logger simLogger;
    simEngine.addObject(&alpha);
    simEngine.addObject(&bravo);
    simEngine.addObject(&charlie);
    simEngine.addObject(&delta);
    simEngine.addObject(&echo);
    simEngine.addObject(&chargeMngr);
    simEngine.addObject(&simLogger);
    simEngine.addCallable([](TimeS dt) {
        for (int i = 0; i < NUM_CHARGERS; i++)
        {
            if (sharedMemory.chargingNetwork.chargers[i] != INT32_MAX)
                std::cout << "CHARGER " << i << "-> PLANE_ID: " << sharedMemory.chargingNetwork.chargers[i] << std::endl;
            else
                std::cout << "CHARGER " << i << "-> AVAILABLE" << std::endl;
        }
        std::queue<int> copy(sharedMemory.chargingNetwork.chargeQueue);
        int size = copy.size();
        if (size > 0)
            std::cout << "\nCHARGE QUEUE:" << std::endl;
        for (int i = 0; i < size; i++)
        {
            std::cout << copy.front() << std::endl;
            copy.pop();
        }
        std::cout << std::endl;
    },
    SIM_BATCH::BATCH2); //adding a custom stdout logger to keep eye on chargers

    simEngine.addCallable([](TimeS dt) {
        for (int i = 0; i < NUM_AIRCRAFTS; i++)
        {
            std::cout << "SEGMENT " << i << ": " << std::endl;
            std::cout << "\tNUM FAULTS: " << sharedMemory.messages[i].numFaults << std::endl;
            std::cout << "\tNUM FLIGHTS: " << sharedMemory.messages[i].numFlights << std::endl;
            std::cout << "\tNUM CHG SESSIONS: " << sharedMemory.messages[i].numChargeSessions << std::endl;
            std::cout << "\tPASSENGER MILES: " << sharedMemory.messages[i].passengerMiles << std::endl;
            std::cout << "\tTOTAL DISTANCE: " << sharedMemory.messages[i].distance << std::endl;
            std::cout << "\tTOTAL FLIGHT TIME: " << sharedMemory.messages[i].flightTime << std::endl;
            std::cout << "\tTOTAL CHG TIME: " << sharedMemory.messages[i].chgTime << std::endl;
            std::cout << "\tBATTERY CAPACITY: " << sharedMemory.messages[i].batteryCap << std::endl;
            std::cout << "\tEVTOL TYPE: " << static_cast<int>(sharedMemory.messages[i].type) << std::endl;
        }
    },
    SIM_BATCH::BATCH2); //adding a custom stdout logger to keep eye sharedMemory

    //enqueue all aircrafts at chargers
    alpha.setState(EVTOL_STATE::GROUNDED_WAITING);
    bravo.setState(EVTOL_STATE::GROUNDED_WAITING);
    charlie.setState(EVTOL_STATE::GROUNDED_WAITING);
    delta.setState(EVTOL_STATE::GROUNDED_WAITING);
    echo.setState(EVTOL_STATE::GROUNDED_WAITING);

    simEngine.start();

    EXPECT_TRUE(((float)sharedMemory.messages[0].distance / (float)sharedMemory.messages[0].numFlights) == 200.0);
}

int main(int argc, char** argv)
{
    //log prbs seed so a test execution can be reproduced
    uint64_t seed = static_cast<uint64_t>( time(NULL) );
    srand( seed );
    std::cout << "The prbs seed for this sim is: " << seed << std::endl;
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}