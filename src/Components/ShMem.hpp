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
    float distance;
    float flightTime;
    float chgTime;
    float batteryCap;
    EVTOL_TYPE type; //vehicle identifier

//chargeManager <-> eVTOL exchange
    bool charging;

    Message()
    {
        clear();
    }
    void clear()
    {
        numFaults = 0;
        numFlights = 0;
        numChargeSessions = 0;
        passengerMiles = 0.0;
        distance = 0.0;
        flightTime = 0.0;
        chgTime = 0.0;
        type = (EVTOL_TYPE)0;
        charging = false;
    }
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
    //aircrafts are assigned to chargers based on wait time priority (max wait time = high priority)
    //a FIFO queue offers O(1) push and pop functions which are the only necessary interactions
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
    void init(){
        while (!chargingNetwork.availableChargers.empty())
        {
            chargingNetwork.availableChargers.pop();
        }
        for (int i = 0; i < NUM_CHARGERS; i++)
        {
            chargingNetwork.availableChargers.push(i);
        }
        while (!chargingNetwork.chargeQueue.empty())
        {
            chargingNetwork.chargeQueue.pop();
        }
        for (auto& charger: chargingNetwork.chargers)
        {
            charger.inUse = false;
            charger.planeid = INT32_MAX;
        }
        for (auto& message: messages)
        {
            message.clear();
        }
    }
};

extern ShMem sharedMemory;

#endif // SHMEM_HPP_
