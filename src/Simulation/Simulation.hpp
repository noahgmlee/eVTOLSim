#include <vector>
#include "SimObj.hpp"
#include "SimTypes.hpp"

class Simulation
{
public:
    Simulation(TimeS dt, TimeS duration);
    void addObject(SimObj* component); //attach Functor as std::function
    void addCallable(std::function<void(TimeS)> callable, SIM_BATCH batch); //attach std::function
    void start();
    void update(TimeS dt);
    TimeS getDt(){
        return this->dt;
    }

private:
    std::vector<std::function<void(TimeS)>> batch1;
    std::vector<std::function<void(TimeS)>> batch2;
    TimeS currTime;
    TimeS dt;
    TimeS duration;
};
