#include <vector>
#include "SimObj.hpp"
#include "SimTypes.hpp"

class Simulation
{
public:
    Simulation(TimeS dt, TimeS duration);
    void addObject(SimObj* component);
    void addCallable(std::function<void(TimeS)> callable, SIM_BATCH batch);
    void start();
    void update(TimeS dt);

private:
    std::vector<std::function<void(TimeS)>> batch1;
    std::vector<std::function<void(TimeS)>> batch2;
    TimeS currTime;
    TimeS dt;
    TimeS duration;
};
