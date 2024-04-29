#ifndef LOGGER_HPP_
#define LOGGER_HPP_

#include "SimObj.hpp"
#include "ShMem.hpp"
#include <iostream>
#include <fstream>

struct eVTOL_logging
{
    float batteryCapacity_kwh;
    int numFaults;
    int numFlights;
    int numChargeSessions;
    float totalDistance;
    TimeS flightTime;
    TimeS chgTime;
    EVTOL_STATE state;
    void clear() {
        batteryCapacity_kwh = 0.0;
        numFaults = 0;
        numFlights = 0;
        numChargeSessions = 0;
        totalDistance = 0;
        flightTime = 0;
        chgTime = 0;
        state = EVTOL_STATE::CRUISING;
    }
};

class Logger : public SimObj
{
public:
    Logger();
    ~Logger();
    void operator()(TimeS dt){
        update(dt);
    }
    void update(TimeS dt);

private:
    TimeS simTime_s;
    eVTOL_logging perAircraftData[5];
    std::ofstream perAircraftFile[5];
    ShMem* shMemPtr;
};

#endif // LOGGER_HPP_
