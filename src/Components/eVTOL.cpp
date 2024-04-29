#include <iostream>
#include "eVTOL.hpp"

//empty constructor for implicit initialization
eVTOL::eVTOL()
{
    characteristics.clear();
    runningData.clear();
    inQueue = false;
    timeCharging = 0;
    distanceFlying = 0.0;
    timeFlying = 0;
    shMemPtr = nullptr;
}

//scalability consideration to add more variance to eVTOL types
eVTOL::eVTOL(eVTOL_consts& eVTOL_data)
{
    characteristics.cruiseSpeed_mph = eVTOL_data.cruiseSpeed_mph;
    characteristics.maxBatteryCapacity_kwh = eVTOL_data.maxBatteryCapacity_kwh;
    characteristics.chargeTime_hs = eVTOL_data.chargeTime_hs;
    characteristics.energyAtCruise_kwhpm = eVTOL_data.energyAtCruise_kwhpm;
    characteristics.passengerCount = eVTOL_data.passengerCount;
    characteristics.faultThresh = eVTOL_data.faultThresh;
    characteristics.type = eVTOL_data.type;
    characteristics.id = eVTOL_data.id;
    runningData.clear();
    runningData.batteryCapacity_kwh = eVTOL_data.maxBatteryCapacity_kwh;
    inQueue = false;
    timeCharging = 0;
    distanceFlying = 0.0;
    timeFlying = 0;
    shMemPtr = &sharedMemory;
}

//abstraction of eVTOL characteristics in setup allows just an enum to determine aircraft type
eVTOL::eVTOL(EVTOL_TYPE type, int id)
{
    //TODO don't use literals define constants such as ALPHA_SPEED, ALPHA_BATTCAPACITY etc.
    switch (type) {
        case EVTOL_TYPE::ALPHA:
            characteristics.cruiseSpeed_mph = 120.0; characteristics.maxBatteryCapacity_kwh = 320.0; characteristics.chargeTime_hs = 0.6;
            characteristics.energyAtCruise_kwhpm = 1.6; characteristics.passengerCount = 4; characteristics.faultThresh = 0.25;
            break;
        case EVTOL_TYPE::BRAVO:
            characteristics.cruiseSpeed_mph = 100.0; characteristics.maxBatteryCapacity_kwh = 100.0; characteristics.chargeTime_hs = 0.2;
            characteristics.energyAtCruise_kwhpm = 1.5; characteristics.passengerCount = 5; characteristics.faultThresh = 0.10;
            break;
        case EVTOL_TYPE::CHARLIE:
            characteristics.cruiseSpeed_mph = 160.0; characteristics.maxBatteryCapacity_kwh = 220.0; characteristics.chargeTime_hs = 0.8;
            characteristics.energyAtCruise_kwhpm = 2.2; characteristics.passengerCount = 3; characteristics.faultThresh = 0.05;
            break;
        case EVTOL_TYPE::DELTA:
            characteristics.cruiseSpeed_mph = 90.0; characteristics.maxBatteryCapacity_kwh = 120.0; characteristics.chargeTime_hs = 0.62;
            characteristics.energyAtCruise_kwhpm = 0.8; characteristics.passengerCount = 2; characteristics.faultThresh = 0.22;
            break;
        case EVTOL_TYPE::ECHO:
            characteristics.cruiseSpeed_mph = 30.0; characteristics.maxBatteryCapacity_kwh= 150.0; characteristics.chargeTime_hs = 0.3;
            characteristics.energyAtCruise_kwhpm = 5.8; characteristics.passengerCount = 2; characteristics.faultThresh = 0.61;
            break;
        default:
            std::cerr << "invalid EVTOL_TYPE passed to constructor" << std::endl;
            break;
    }
    batch = SIM_BATCH::BATCH1;
    characteristics.type = type;
    characteristics.id = id; //sim identifier
    runningData.clear();
    runningData.batteryCapacity_kwh = characteristics.maxBatteryCapacity_kwh;
    inQueue = false;
    timeCharging = 0;
    distanceFlying = 0.0;
    timeFlying = 0;
    shMemPtr = &sharedMemory;
}

void eVTOL::update(TimeS dt)
{
    updateStateMachine(dt);
    updateLogging();
}

void eVTOL::updateStateMachine(TimeS dt)
{
    switch (runningData.state) {
        case EVTOL_STATE::CRUISING:
            updateBattery(dt);
            runningData.numFaults = updateFault(dt) ? runningData.numFaults + 1 : runningData.numFaults;
            break;
        case EVTOL_STATE::GROUNDED_CHARGING:
            updateBatteryCharging(dt);
            break;
        case EVTOL_STATE::GROUNDED_WAITING:
            updateChargeQueue();
        default:
            break;
    }
}

void eVTOL::updateBattery(TimeS dt)
{
    //power consumption = kwh/mile * miles flown
    float distance = characteristics.cruiseSpeed_mph * SEC_TO_HRS(dt); //in miles
    runningData.batteryCapacity_kwh -= distance * characteristics.energyAtCruise_kwhpm;
    distanceFlying += distance;
    timeFlying += dt;
    //only update logging data when a flight completes (battery is 0%)
    if (runningData.batteryCapacity_kwh < 0.0)
    {
        runningData.totalDistance += distanceFlying;
        runningData.passengerMiles += distanceFlying * (float)characteristics.passengerCount;
        runningData.flightTime += timeFlying;
        runningData.numFlights += 1;
        distanceFlying = 0.0;
        timeFlying = 0;
        runningData.batteryCapacity_kwh = 0.0;
        runningData.state = EVTOL_STATE::GROUNDED_WAITING;
    }
}

void eVTOL::updateBatteryCharging(TimeS dt)
{
    //convert chargeTime to a delta power charged with respect to dt
    //rate = 1/timeToCharge * hours * battery_capacity_constant
    float ratekwhdt = SEC_TO_HRS(dt) * 1.0/characteristics.chargeTime_hs * characteristics.maxBatteryCapacity_kwh; //in miles
    runningData.batteryCapacity_kwh += ratekwhdt;
    timeCharging += dt;
    if (runningData.batteryCapacity_kwh > characteristics.maxBatteryCapacity_kwh)
    {
        runningData.batteryCapacity_kwh = characteristics.maxBatteryCapacity_kwh;
        shMemPtr->messages[characteristics.id].charging = false;
        runningData.chgTime += timeCharging;
        runningData.numChargeSessions += 1;
        timeCharging = 0;
        runningData.state = EVTOL_STATE::CRUISING;
    }
}

void eVTOL::updateChargeQueue()
{
    if (!inQueue) //queue an eVTOL for charge once landed
    {
        shMemPtr->chargingNetwork.chargeQueue.push(characteristics.id);
        inQueue = true;
    }
    else
    {
        if (shMemPtr->messages[characteristics.id].charging) //transition to charging when chargeManager indicates
        {
            runningData.state = EVTOL_STATE::GROUNDED_CHARGING;
            inQueue = false;
        }
    }
}

void eVTOL::updateLogging() //write stats to sharedMemory for Logger
{
    shMemPtr->messages[characteristics.id].numFaults = runningData.numFaults;
    shMemPtr->messages[characteristics.id].numFlights = runningData.numFlights;
    shMemPtr->messages[characteristics.id].numChargeSessions = runningData.numChargeSessions;
    shMemPtr->messages[characteristics.id].passengerMiles = runningData.passengerMiles;
    shMemPtr->messages[characteristics.id].distance = runningData.totalDistance;
    shMemPtr->messages[characteristics.id].flightTime = runningData.flightTime;
    shMemPtr->messages[characteristics.id].chgTime = runningData.chgTime;
    shMemPtr->messages[characteristics.id].batteryCap = runningData.batteryCapacity_kwh;
    shMemPtr->messages[characteristics.id].type = characteristics.type;
}

//Note: fault projection to int range 1-10,000 assumes faultThreshdt always > 0.0000X
bool eVTOL::updateFault(TimeS dt)
{
    float faultThreshdt = characteristics.faultThresh * ((float)dt / 3600.0);
    if (faultThreshdt < 0.0001) {
        std::cerr << "Must simulate aircrafts with a dt to maintain a faultThreshdt > 0.0000X" << std::endl;
    }
    float randNum = (float)(rand() % 10000 + 1); //pseudo-random number between 1 and 100 inclusive
    randNum = randNum / 10000.0;
    return (randNum < faultThreshdt);
}
