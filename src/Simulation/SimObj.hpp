#ifndef SIMOBJ_HPP_
#define SIMOBJ_HPP_

#include "SimTypes.hpp"

class SimObj
{
public:
    virtual void update(TimeS dt) = 0;
    virtual void operator()(TimeS dt) = 0;
    SIM_BATCH batch;
};

#endif // SIMOBJ_HPP_
