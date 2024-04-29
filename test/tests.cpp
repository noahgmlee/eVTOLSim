#include <gtest/gtest.h>
#include <stdint.h>
#include "SimTypes.hpp"
#include "eVTOL.hpp"
#include "ChargeManager.hpp"
#include "Simulation.hpp"

//TODO add testhelpers header and implementation to abstract helpful test sequences

TEST(EVTOL, instantiateEVTOL)
{
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

TEST(EVTOL, cruiseUpdate)
{
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

TEST(EVTOL, chargeUpdate)
{
    TimeS dt = 60;
    eVTOL alpha = eVTOL(EVTOL_TYPE::ALPHA, 0);
    alpha.setState(EVTOL_STATE::GROUNDED_CHARGING);
    alpha.setBatteryCap(0.0);
    alpha.update(dt); //run 1 minute
    float batteryCap = 0.0;
    batteryCap += SEC_TO_HRS(dt) * 1.0/(float)0.6 * (float)320.0;
    EXPECT_TRUE(abs(alpha.getBatteryCapacity() - batteryCap) < 0.00001) << "wrong battery capacity: "
                                                               << alpha.getBatteryCapacity()
                                                               << "\nexpected: "
                                                               << batteryCap;
    for (int i = 0; i < 3; i ++){
        alpha.update(dt);
        batteryCap += SEC_TO_HRS(dt) * 1.0/(float)0.6 * (float)320.0;
        EXPECT_TRUE(abs(alpha.getBatteryCapacity() - batteryCap) < 0.00001) << "wrong battery capacity: "
                                                                << alpha.getBatteryCapacity()
                                                                << "\nexpected: "
                                                                << batteryCap;
    }
    while (batteryCap < 320.0){
        alpha.update(dt);
        batteryCap += SEC_TO_HRS(dt) * 1.0/(float)0.6 * (float)320.0;
    }
    EXPECT_TRUE(abs(alpha.getBatteryCapacity() - 320.0) < 0.00001) << "wrong battery capacity: "
                                                     << alpha.getBatteryCapacity()
                                                     << "\nexpected: "
                                                     << 320.0;
}

void tickSim(std::vector<SimObj*>& components, TimeS dt)
{
    for (auto component: components)
    {
        component->update(dt);
    }
}

TEST(CHARGEMANAGER, chargeQueue)
{
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

TEST(INTEGRATION, SimEngineSimple)
{
    Simulation simEngine = Simulation(60, 60 * 10); //run 10 ticks
    int i1 = 0, i2 = 0, i3 = 0;
    std::function<void(TimeS)> lambda1 = [&i1](TimeS dt) { std::cout << i1 << ": I am lambda 1, batch1!" << std::endl; i1++; };
    std::function<void(TimeS)> lambda2 = [&i2](TimeS dt) { std::cout << i2 << ": I am lambda 2, batch2!" << std::endl; i2++; };
    std::function<void(TimeS)> lambda3 = [&i3](TimeS dt) { std::cout << i3 << ": I am lambda 3, batch1!" << std::endl; i3++; };
    simEngine.addCallable(lambda1, SIM_BATCH::BATCH1);
    simEngine.addCallable(lambda2, SIM_BATCH::BATCH2);
    simEngine.addCallable(lambda3, SIM_BATCH::BATCH1);
    simEngine.start();
}

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



int main(int argc, char** argv)
{
    uint64_t seed = static_cast<uint64_t>( time(NULL) );
    srand( seed );
    std::cout << "The prbs seed for this sim is: " << seed << std::endl;
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}