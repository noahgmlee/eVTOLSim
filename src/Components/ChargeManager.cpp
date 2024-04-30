#include "ChargeManager.hpp"

ChargeManager::ChargeManager()
{
    batch = SIM_BATCH::BATCH2;
    shMemPtr = &sharedMemory;
}

void ChargeManager::update(TimeS dt)
{
    //add any chargers back to availableChargers queue when an aircraft is done charging
    for (int i = 0; i < NUM_CHARGERS; i++)
    {
        if (shMemPtr->chargingNetwork.chargers[i] != INT32_MAX)
        {
            if (!shMemPtr->messages[shMemPtr->chargingNetwork.chargers[i]].charging)
            {
                shMemPtr->chargingNetwork.chargers[i] = INT32_MAX;
                shMemPtr->chargingNetwork.availableChargers.push(i);
            }
        }
    }

    //add any aircrafts at front of queue to available chargers
    while (!shMemPtr->chargingNetwork.chargeQueue.empty() &&
           !shMemPtr->chargingNetwork.availableChargers.empty())
    {

        int aircraftId = shMemPtr->chargingNetwork.chargeQueue.front();
        shMemPtr->chargingNetwork.chargeQueue.pop();
        int chargerId = shMemPtr->chargingNetwork.availableChargers.front();
        shMemPtr->chargingNetwork.availableChargers.pop();
        shMemPtr->messages[aircraftId].charging = true;
        shMemPtr->chargingNetwork.chargers[chargerId] = aircraftId;
    }
}
