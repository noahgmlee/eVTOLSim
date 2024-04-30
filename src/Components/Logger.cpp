#include "Logger.hpp"

Logger::Logger()
{
    batch = SIM_BATCH::BATCH2;
    shMemPtr = &sharedMemory;
    simTime_s = 0;

    for (int i = 0; i < 5; i++)
    {
        perAircraftData[i].clear();
    }

    perAircraftFile[0].open("outputs/alphaData.csv");
    perAircraftFile[1].open("outputs/bravoData.csv");
    perAircraftFile[2].open("outputs/charlieData.csv");
    perAircraftFile[3].open("outputs/deltaData.csv");
    perAircraftFile[4].open("outputs/echoData.csv");

    perAircraftFile[0] << "ALPHA eVTOL logging file\n" << "SIM TIME (s), Time Per Flight (s), Distance Per Flight (mi), Time Per Charge (s), Total Faults, Total Passenger Miles\n";
    perAircraftFile[1] << "BRAVO eVTOL logging file\n" << "SIM TIME (s), Time Per Flight (s), Distance Per Flight (mi), Time Per Charge (s), Total Faults, Total Passenger Miles\n";
    perAircraftFile[2] << "CHARLIE eVTOL logging file\n" << "SIM TIME (s), Time Per Flight (s), Distance Per Flight (mi), Time Per Charge (s), Total Faults, Total Passenger Miles\n";
    perAircraftFile[3] << "DELTA eVTOL logging file\n" << "SIM TIME (s), Time Per Flight (s), Distance Per Flight (mi), Time Per Charge (s), Total Faults, Total Passenger Miles\n";
    perAircraftFile[4] << "ECHO eVTOL logging file\n" << "SIM TIME (s), Time Per Flight (s), Distance Per Flight (mi), Time Per Charge (s), Total Faults, Total Passenger Miles\n";
}

Logger::~Logger()
{
    for (int i = 0; i < 5; i++)
    {
        perAircraftFile[i].close();
    }
}

void Logger::update(TimeS dt)
{
    simTime_s += dt;
    for (int i = 0; i < 5; i++)
    {
        perAircraftData[i].clear(); //cumulative sum needs be reset each logging
    }
    for (int i = 0; i < NUM_AIRCRAFTS; i++)
    {
        EVTOL_TYPE type = shMemPtr->messages[i].type;
        perAircraftData[static_cast<int>(type)].numChargeSessions += shMemPtr->messages[i].numChargeSessions;
        perAircraftData[static_cast<int>(type)].numFlights += shMemPtr->messages[i].numFlights;
        perAircraftData[static_cast<int>(type)].numFaults += shMemPtr->messages[i].numFaults;
        perAircraftData[static_cast<int>(type)].passengerMiles += shMemPtr->messages[i].passengerMiles;
        perAircraftData[static_cast<int>(type)].totalDistance += shMemPtr->messages[i].distance;
        perAircraftData[static_cast<int>(type)].flightTime += shMemPtr->messages[i].flightTime;
        perAircraftData[static_cast<int>(type)].chgTime += shMemPtr->messages[i].chgTime;
    }
    for (int i = 0; i < 5; i++)
    {
        float timePerFlight, distancePerFlight, timePerCharge;
        if (perAircraftData[i].numFlights > 0) //division by 0 consideration
        {
            timePerFlight = (float) perAircraftData[i].flightTime / (float) perAircraftData[i].numFlights;
            distancePerFlight = (float) perAircraftData[i].totalDistance / (float) perAircraftData[i].numFlights;
        }
        else
        {
            timePerFlight = 0.0;
            distancePerFlight = 0.0;
        }
        if (perAircraftData[i].numChargeSessions > 0)
            timePerCharge = (float) perAircraftData[i].chgTime / (float) perAircraftData[i].numChargeSessions;
        else
            timePerCharge = 0.0;
        perAircraftFile[i] << simTime_s << " (s), " << timePerFlight << " (s), " << distancePerFlight << " (mi), "
                           << timePerCharge << " (s), " << perAircraftData[i].numFaults << ", " << perAircraftData[i].passengerMiles << " (mi * passengers)\n";
    }
}
