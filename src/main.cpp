#include <iostream>
#include "SimTypes.hpp"
#include "eVTOL.hpp"
#include "ChargeManager.hpp"
#include "Simulation.hpp"
#include "Logger.hpp"

int main()
{
    //logging of rand seed allows a given sim to be reproduced
    uint64_t seed = static_cast<uint64_t>( time(NULL) );
    srand( seed );
    std::cout << "The prbs seed for this sim is: " << seed << std::endl;

    //initialize sharedMemory structure
    sharedMemory.init();

    //60 seconds dt, 3 hour total sim time
    Simulation simEngine = Simulation(60, HRS_TO_SEC(3));

    //build histogram of eVTOL types randomly generated for sim
    int histo[5] = {0};

    //random generation of aircraft
    eVTOL aircrafts[NUM_AIRCRAFTS];
    for (int i = 0; i < NUM_AIRCRAFTS; i++)
    {
        EVTOL_TYPE type = (EVTOL_TYPE) (rand() % 5); //0-4 covers the raw range of EVTOL_TYPE enum class
        histo[static_cast<int>(type)] += 1;
        aircrafts[i] = eVTOL(type, i);
        simEngine.addObject(&aircrafts[i]);
    }

    //print histogram
    std::cout << "eVTOL distribution is: " <<
                 "\nALPHA: " << histo[0] <<
                 "\nBRAVO: " << histo[1] <<
                 "\nCHARLIE: " << histo[2] <<
                 "\nDELTA: " << histo[3] <<
                 "\nECHO: " << histo[4] << std::endl;

    //attach ChargeManager and Logger as batch2 sim components
    ChargeManager chgMngr = ChargeManager();
    simEngine.addObject(&chgMngr);

    Logger simLogger;
    simEngine.addObject(&simLogger);

    //sharedMemory logger added to sim as lambda, test_output.txt can be analyzed
    simEngine.addCallable([](TimeS dt) {
        for (int i = 0; i < NUM_AIRCRAFTS; i++)
        {
            std::cout << "SEGMENT " << i << ": " << std::endl;
            std::cout << "\tNUM FAULTS: " << sharedMemory.messages[i].numFaults << std::endl;
            std::cout << "\tNUM FLIGHTS: " << sharedMemory.messages[i].numFlights << std::endl;
            std::cout << "\tNUM CHG SESSIONS: " << sharedMemory.messages[i].numChargeSessions << std::endl;
            std::cout << "\tPASSENGER MILES: " << sharedMemory.messages[i].passengerMiles << std::endl;
            std::cout << "\tTOTAL DISTANCE: " << sharedMemory.messages[i].distance << std::endl;
            std::cout << "\tTOTAL FLIGHT TIME: " << sharedMemory.messages[i].flightTime << std::endl;
            std::cout << "\tTOTAL CHG TIME: " << sharedMemory.messages[i].chgTime << std::endl;
            std::cout << "\tBATTERY CAPACITY: " << sharedMemory.messages[i].batteryCap << std::endl;
            std::cout << "\tEVTOL TYPE: " << static_cast<int>(sharedMemory.messages[i].type) << std::endl;
        }
    },
    SIM_BATCH::BATCH2);

    simEngine.start(); //run simulation

    return 0;
}
