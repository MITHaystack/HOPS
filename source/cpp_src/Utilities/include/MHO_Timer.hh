#ifndef MHO_Timer_HH__
#define MHO_Timer_HH__

#include <string>
#include <ctime>

namespace hops
{

/**
*@file MHO_Timer.hh
*@class MHO_Timer
*@author J. Barrett - barrettj@mit.edu
* A timer class responsible for thread and process benchmarking
* For more information on the linux system variables and functions that were leveraged see: https://linux.die.net/man/3/clock_gettime
*/

class MHO_Timer
{
    public:
        MHO_Timer();
        MHO_Timer(std::string name);
        virtual ~MHO_Timer();

        /* Set the timer name
        * @param name string
        * @return None
        */
        void SetName(std::string name){fName = name;};

        /* Get the timer name
        * @param None
        * @return fName string timer name
        */
        std::string GetName() const {return fName;};

        //set the clock type used
        /* Set the timer's clock ID to that of the linux system wide realtime clock
        * @param None
        * @returns None
        */
        void MeasureWallclockTime(); //CLOCK_REALTIME

        /* Get the ID for the process clock
        * @param None
        * @returns None
        */
        void MeasureProcessTime(); //CLOCK_PROCESS_CPUTIME_ID

        /* Get the ID for the thread clock which indicates the time used by all threads in the process
        * @param None
        * @returns None
        */
        void MeasureThreadTime(); //CLOCK_THREAD_CPUTIME_ID

        /* Store the current linux system time as the start time of a timer in a timespec struct
        * @param None
        * @returns None
        */
        void Start();

        /* Store the current linux system time as the stop time of a timer in a timespec struct
        * @param None
        * @returns None
        */
        void Stop();

        /* Call GetTimeDifference and return the result as a timespec struct
        * @param None
        * @reurns ret_val timespec which is a struct containing the start and stop time in seconds and nanoseconds as two seperate values
        */
        timespec GetDurationAsTimeSpec() const;

        /* Call GetTimeDifference and return the result as a double
        * @param None
        * @reurns ret_val double which contains the start and stop time in seconds 
        */
        double GetDurationAsDouble() const;

    protected:
        /* Calculate total compute time spent on a process or thread and store the result in a timespec struct 
        * @param start timespec which is the start time for a process or thread
        * @param stop timespec which is the start time for a process or thread
        * @returns ret_val timespec which contains the time spent on a given process or thread in seconds and nanoseconds
        */
        timespec GetTimeDifference(const timespec& start, const timespec& stop) const;

        std::string fName;
        clockid_t fClockID;
        timespec fStart;
        timespec fStop;

};

}

#endif /* end of include guard: MHO_Timer */
