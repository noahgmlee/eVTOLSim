#include "SimObj.hpp"
#include "ShMem.hpp"

class ChargeManager : public SimObj
{
public:
    ChargeManager();
    void operator()(TimeS dt){
        update(dt);
    }
    void update(TimeS dt);

private:
    ShMem* shMemPtr;
};
