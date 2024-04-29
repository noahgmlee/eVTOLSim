#include <iostream>
#include "SimTypes.hpp"
#include "eVTOL.hpp"
#include "ChargeManager.hpp"
#include "Simulation.hpp"
#include "Logger.hpp"

int main()
{
    uint64_t seed = static_cast<uint64_t>( time(NULL) );
    srand( seed );
    std::cout << "The prbs seed for this sim is: " << seed << std::endl;

    sharedMemory.init();

    Simulation simEngine = Simulation(60, HRS_TO_SEC(3));

    int histo[5] = {0};
    //random generation of aircraft
    for (int i = 0; i < NUM_AIRCRAFTS; i++)
    {
        EVTOL_TYPE type = (EVTOL_TYPE) (rand() % 5); //0-4 covers the raw range of EVTOL_TYPE enum class
        histo[static_cast<int>(type)] += 1;
        eVTOL evtol(type, i);
        simEngine.addObject(&evtol);
    }

    std::cout << "eVTOL distribution is: " <<
                 "\nALPHA: " << histo[0] <<
                 "\nBRAVO: " << histo[1] <<
                 "\nCHARLIE: " << histo[2] <<
                 "\nDELTA: " << histo[3] <<
                 "\nECHO: " << histo[4] << std::endl;

    ChargeManager chgMngr = ChargeManager();
    simEngine.addObject(&chgMngr);

    Logger simLogger;
    simEngine.addObject(&simLogger);

    simEngine.start();

    return 0;
}
