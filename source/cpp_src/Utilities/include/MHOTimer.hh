#ifndef MHO_Timer_HH__
#define MHO_Timer_HH__

#include <string>
#include <ctime>

namespace hops
{

class MHO_Timer
{
    public:
        MHO_Timer();
        MHO_Timer(std::string name);
        virtual ~MHO_Timer();

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

#endif /* end of include guard: MHO_Timer */
