#ifndef SIMOBJ_HPP_
#define SIMOBJ_HPP_

#include "SimTypes.hpp"
#include <iostream>

//base functor that Simulation API accepts
class SimObj
{
public:
    virtual void update(TimeS dt){
        (void)dt;
    }
    virtual void operator()(TimeS dt){
        update(dt);
    }
    SIM_BATCH batch;
};

#endif // SIMOBJ_HPP_
