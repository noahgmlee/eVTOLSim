// eVTOL class
enum class eVTOL_TYPE
{
    ALPHA = 0,
    BRAVO,
    CHARLIE,
    DELTA,
    ECHO,
};

struct eVTOLProperties
{
    float cruiseSpeed_mph;
    float batteryCapacity_kwh;
    float chargeTime_hs;
    float energyAtCruise_kwhpm;
    int passengerCount;
    float faultProb;
};

class shmem
{
    int data;
};

class eVTOL
{
    public:
        void update();
        static shmem mem;

    private:
        float cruiseSpeed_mph;
        float batteryCapacity_kwh;
        float chargeTime_hs;
        float energyAtCruise_kwhpm; //kilo watt hours per mile
};
