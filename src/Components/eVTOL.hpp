#include "SimTypes.hpp"
#include "SimObj.hpp"
#include "ShMem.hpp"
#include "Logger.hpp"

//struct of characteristic data to an eVTOL type
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

    void clear()
    {
        cruiseSpeed_mph = 0.0;
        maxBatteryCapacity_kwh = 0.0;
        chargeTime_hs = 0.0;
        energyAtCruise_kwhpm = 0.0;
        passengerCount = 0;
        faultThresh = 0.0;
        type = EVTOL_TYPE(0);
        id = 0;
    }
};

//TODO as plane types evolve making eVTOL a base class would make sense
//alpha, bravo, charlie, delta and echo would become derived classes
//physical battery models could be exclusive to each, for example
//could leverage static members for per aircraft statistic updates
class eVTOL : public SimObj
{
public:
    eVTOL();
    eVTOL(eVTOL_consts& eVTOL_data);
    eVTOL(EVTOL_TYPE type, int id);

//SimObj overrides
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
    void updateLogging();
    TimeS timeCharging; //per charge session tracker
    TimeS timeFlying; //per flight time tracker
    float distanceFlying; //per flight distance tracker
    bool inQueue;
    eVTOL_consts characteristics;
    eVTOL_logging runningData;
    ShMem* shMemPtr;
};
