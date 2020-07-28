#ifndef HkTimer_HH__
#define HkTimer_HH__

#include <string>
#include <ctime>

namespace hops
{

class HkTimer
{
    public:
        HkTimer();
        HkTimer(std::string name);
        virtual ~HkTimer();

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

#endif /* end of include guard: HkTimer */
