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
        if (type == EVTOL_TYPE::ALPHA)
        {
            perAircraftData[0].numChargeSessions += shMemPtr->messages[i].numChargeSessions;
            perAircraftData[0].numFlights += shMemPtr->messages[i].numFlights;
            perAircraftData[0].numFaults += shMemPtr->messages[i].numFaults;
            perAircraftData[0].passengerMiles += shMemPtr->messages[i].passengerMiles;
            perAircraftData[0].totalDistance += shMemPtr->messages[i].distance;
            perAircraftData[0].flightTime += shMemPtr->messages[i].flightTime;
            perAircraftData[0].chgTime += shMemPtr->messages[i].chgTime;
        }
        else if (type == EVTOL_TYPE::BRAVO)
        {
            perAircraftData[1].numChargeSessions += shMemPtr->messages[i].numChargeSessions;
            perAircraftData[1].numFlights += shMemPtr->messages[i].numFlights;
            perAircraftData[1].numFaults += shMemPtr->messages[i].numFaults;
            perAircraftData[1].passengerMiles += shMemPtr->messages[i].passengerMiles;
            perAircraftData[1].totalDistance += shMemPtr->messages[i].distance;
            perAircraftData[1].flightTime += shMemPtr->messages[i].flightTime;
            perAircraftData[1].chgTime += shMemPtr->messages[i].chgTime;
        }
        else if (type == EVTOL_TYPE::CHARLIE)
        {
            perAircraftData[2].numChargeSessions += shMemPtr->messages[i].numChargeSessions;
            perAircraftData[2].numFlights += shMemPtr->messages[i].numFlights;
            perAircraftData[2].numFaults += shMemPtr->messages[i].numFaults;
            perAircraftData[2].passengerMiles += shMemPtr->messages[i].passengerMiles;
            perAircraftData[2].totalDistance += shMemPtr->messages[i].distance;
            perAircraftData[2].flightTime += shMemPtr->messages[i].flightTime;
            perAircraftData[2].chgTime += shMemPtr->messages[i].chgTime;
        }
        else if (type == EVTOL_TYPE::DELTA)
        {
            perAircraftData[3].numChargeSessions += shMemPtr->messages[i].numChargeSessions;
            perAircraftData[3].numFlights += shMemPtr->messages[i].numFlights;
            perAircraftData[3].numFaults += shMemPtr->messages[i].numFaults;
            perAircraftData[3].passengerMiles += shMemPtr->messages[i].passengerMiles;
            perAircraftData[3].totalDistance += shMemPtr->messages[i].distance;
            perAircraftData[3].flightTime += shMemPtr->messages[i].flightTime;
            perAircraftData[3].chgTime += shMemPtr->messages[i].chgTime;
        }
        else if (type == EVTOL_TYPE::ECHO)
        {
            perAircraftData[4].numChargeSessions += shMemPtr->messages[i].numChargeSessions;
            perAircraftData[4].numFlights += shMemPtr->messages[i].numFlights;
            perAircraftData[4].numFaults += shMemPtr->messages[i].numFaults;
            perAircraftData[4].passengerMiles += shMemPtr->messages[i].passengerMiles;
            perAircraftData[4].totalDistance += shMemPtr->messages[i].distance;
            perAircraftData[4].flightTime += shMemPtr->messages[i].flightTime;
            perAircraftData[4].chgTime += shMemPtr->messages[i].chgTime;
        }
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
                           << timePerCharge << " (s), " << perAircraftData[i].numFaults << ", " << perAircraftData[i].passengerMiles << "\n";
    }
}
