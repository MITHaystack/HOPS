#ifndef HTimer_HH__
#define HTimer_HH__

#include <string>
#include <ctime>


/*
*File: HTimer.hh
*Class: HTimer
*Author: J. Barrett
*Email: barrettj@mit.edu
*Date:
*Description: time for debugging/performance test, uses clock_gettime
*/

namespace hops
{

class HTimer
{
    public:
        HTimer();
        HTimer(std::string name);
        virtual ~HTimer();

        void SetName(std::string name){fName = name;};
        std::string GetName() const {return fName;};

        //set the clock type used
        void MeasureWallclockTime(); //CLOCK_REALTIME
        void MeasureProcessTime(); //CLOCK_PROCESS_CPUTIME_ID
        void MeasureThreadTime(); //CLOCK_THREAD_CPUTIME_ID

        void Start();
        void Stop();

        timespec GetDurationAsTimeSpec() const;
        double GetDurationAsDouble() const;

    protected:

        timespec GetTimeDifference(const timespec& start, const timespec& stop) const;

        std::string fName;
        clockid_t fClockID;
        timespec fStart;
        timespec fStop;

};

}

#endif /* end of include guard: HTimer */
