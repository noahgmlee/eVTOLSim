#ifndef SIMTYPES_HPP_
#define SIMTYPES_HPP_

#define NUM_AIRCRAFTS 20
#define NUM_CHARGERS 3
#define SEC_TO_HRS(s) (1.0/60.0) * (1.0/60.0) * s

#include <stdint.h>
typedef uint64_t TimeS;

// eVTOL class
enum class EVTOL_TYPE
{
    ALPHA = 0,
    BRAVO,
    CHARLIE,
    DELTA,
    ECHO,
};

enum class EVTOL_STATE
{
    CRUISING = 0,
    GROUNDED_WAITING,
    GROUNDED_CHARGING,
};

enum class CHARGE_STATE
{
    UNTRACKED = 0,
    QUEUED,
    CHARGING,
};

enum class SIM_BATCH
{
    BATCH1 = 0,
    BATCH2,
};

#endif // SIMTYPES_HPP_
