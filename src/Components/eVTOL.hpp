#include "SimTypes.hpp"
#include "SimObj.hpp"
#include "ShMem.hpp"

struct eVTOL_consts
{
    float cruiseSpeed_mph;
    float maxBatteryCapacity_kwh;
    float chargeTime_hs;
    float energyAtCruise_kwhpm;
    int passengerCount;
    float faultThresh;
    EVTOL_TYPE type;
    int id;
};

struct eVTOL_logging
{
    float batteryCapacity_kwh;
    int numFaults;
    int numFlights;
    int numChargeSessions;
    float passengerMiles;
    float totalDistance;
    TimeS flightTime;
    TimeS chgTime;
    EVTOL_STATE state;
    void clear() {
        batteryCapacity_kwh = 0.0;
        numFaults = 0;
        numFlights = 0;
        numChargeSessions = 0;
        passengerMiles = 0;
        totalDistance = 0;
        flightTime = 0;
        chgTime = 0;
        state = EVTOL_STATE::CRUISING;
    }
};

//TODO as plane types evolve making eVTOL a base class would make sense
//alpha, bravo, charlie, delta and echo would become derived classes
//physical battery models could be exclusive to each, for example
//could leverage static members for per aircraft statistic updates
class eVTOL : public SimObj
{
public:
    eVTOL(eVTOL_consts& eVTOL_data);
    eVTOL(EVTOL_TYPE type, int id);
    void operator()(TimeS dt){
        update(dt);
    }
    void update(TimeS dt);

//getters & setters for testability
    void setState(EVTOL_STATE state){
        runningData.state = state;
    }
    void setFaultProb(float faultProb){
        characteristics.faultThresh = faultProb;
    }
    void setBatteryCap(float batteryCap){
        runningData.batteryCapacity_kwh = batteryCap;
    }
    EVTOL_STATE getState(){
        return runningData.state;
    }
    int getFaults(){
        return runningData.numFaults;
    }
    float getSpeed()
    {
        return characteristics.cruiseSpeed_mph;
    }
    float getBatteryCapacity()
    {
        return runningData.batteryCapacity_kwh;
    }
    float getEnergyUsage()
    {
        return characteristics.energyAtCruise_kwhpm;
    }

private:
    void updateStateMachine(TimeS dt);
    void updateBattery(TimeS dt);
    void updateBatteryCharging(TimeS dt);
    bool updateFault(TimeS dt);
    void updateChargeQueue();
    bool inQueue;
    eVTOL_consts characteristics;
    eVTOL_logging runningData;
    ShMem* shMemPtr;
};
