#include "Simulation.hpp"
#include <iostream>

Simulation::Simulation(TimeS dt, TimeS duration) : duration(duration)
{
    if (dt == 0)
    {
        std::cerr << "must pass dt > 0 to the Simulation" << std::endl;
        this->dt = 60; //default to 1 minute dt
    }
    else
    {
        this->dt = dt;
    }
    currTime = 0;
}

void Simulation::addObject(SimObj* component)
{
    //ensure polymorphism is not lost through a hard copy in std::function construction
    addCallable(std::ref(*component), component->batch);
}

void Simulation::addCallable(std::function<void(TimeS)> callable, SIM_BATCH batch)
{
    if (batch == SIM_BATCH::BATCH1)
    {
        batch1.push_back(callable);
    }
    else if (batch == SIM_BATCH::BATCH2)
    {
        batch2.push_back(callable);
    }
}

void Simulation::start()
{
    while (currTime < duration)
    {
        currTime += dt;
        update(dt);
    }
}

void Simulation::update(TimeS dt)
{
    //TODO std::thread wrappers to concurrently run the batches. requires shared memory protections
    for (auto callable: batch1){
        callable(dt);
    }
    for (auto callable: batch2){
        callable(dt);
    }
}
