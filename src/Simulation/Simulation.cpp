#include "Simulation.hpp"

Simulation::Simulation(TimeS dt, TimeS duration) : dt(dt), duration(duration)
{
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
    for (auto callable: batch1){
        callable(dt);
    }
    for (auto callable: batch2){
        callable(dt);
    }
}
