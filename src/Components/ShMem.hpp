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

struct chargerNetwork
{
    //aircrafts are assigned to chargers based on wait time priority (max wait time = highest priority therefor FIFO)
    //a FIFO queue offers O(1) push and pop functions which are the only necessary interactions
    //TODO as num chargers scales a design improvement would be a hash table of planeIds replaces chargers array
    //availableChargers could be a "semaphore" esque resource counter that is decrement as planeIds get added
    //constant time lookup in the hash table allows easy element removal when a plane completes charge
    std::queue<int> availableChargers;
    std::queue<int> chargeQueue;
    int chargers[NUM_CHARGERS]; //holds planeId using this charger
    chargerNetwork()
    {
        for (int i = 0; i < NUM_CHARGERS; i++)
        {
            availableChargers.push(i);
        }
    }
};

//TODO refactor this to have a shared message queue system
//similar to CAN protocol each component can share the queue
//and receive a subset of message identifiers
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
            charger = INT32_MAX;
        }
        for (auto& message: messages)
        {
            message.clear();
        }
    }
};

extern ShMem sharedMemory;

#endif // SHMEM_HPP_
