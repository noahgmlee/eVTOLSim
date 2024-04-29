#include "ChargeManager.hpp"

ChargeManager::ChargeManager()
{
    batch = SIM_BATCH::BATCH2;
    shMemPtr = &sharedMemory;
}

void ChargeManager::update(TimeS dt)
{
    for (int i = 0; i < NUM_CHARGERS; i++)
    {
        if (shMemPtr->chargingNetwork.chargers[i].inUse)
        {
            if (!shMemPtr->messages[shMemPtr->chargingNetwork.chargers[i].planeid].charging)
            {
                shMemPtr->chargingNetwork.chargers[i].inUse = false;
                shMemPtr->chargingNetwork.chargers[i].planeid = INT32_MAX;
                shMemPtr->chargingNetwork.availableChargers.push(i);
            }
        }
    }
    while (!shMemPtr->chargingNetwork.chargeQueue.empty() &&
           !shMemPtr->chargingNetwork.availableChargers.empty())
    {

        int aircraftId = shMemPtr->chargingNetwork.chargeQueue.front();
        shMemPtr->chargingNetwork.chargeQueue.pop();
        int chargerId = shMemPtr->chargingNetwork.availableChargers.front();
        shMemPtr->chargingNetwork.availableChargers.pop();
        shMemPtr->messages[aircraftId].charging = true;
        shMemPtr->chargingNetwork.chargers[chargerId].planeid = aircraftId;
        shMemPtr->chargingNetwork.chargers[chargerId].inUse = true;
    }
}
