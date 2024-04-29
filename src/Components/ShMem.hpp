#ifndef SHMEM_HPP_
#define SHMEM_HPP_

#include "SimTypes.hpp"
#include <queue>

//TODO shared memory, messages, queue & chargers are not thread safe, should add protection
//so Simulation engine can schedule component updates as threads

struct Message
{
//eVTOL -> Logger
    int numFaults;
    int numFlights;
    int numChargeSessions;
    float passengerMiles;
    float Distance;
    float FlightTime;
    float ChgTime;
    EVTOL_TYPE type; //vehicle identifier

//chargeManager <-> eVTOL exchange
    bool charging;
};

struct charger
{
    int  planeid;
    bool inUse;
    charger()
    {
        planeid = INT32_MAX;
        inUse = false;
    }
};

struct chargerNetwork
{
    std::queue<int> availableChargers;
    std::queue<int> chargeQueue;
    charger chargers[NUM_CHARGERS];
    chargerNetwork()
    {
        for (int i = 0; i < NUM_CHARGERS; i++)
        {
            availableChargers.push(i);
        }
    }
};

struct ShMem
{
    Message messages[NUM_AIRCRAFTS];
    chargerNetwork chargingNetwork;
};

extern ShMem sharedMemory;

#endif // SHMEM_HPP_
