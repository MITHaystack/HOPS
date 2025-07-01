#ifndef MHO_LegacyRootCodeGenerator_HH__
#define MHO_LegacyRootCodeGenerator_HH__

#include <ctime>
#include <string>
#include <vector>

namespace hops
{

/*!
 *@file  MHO_LegacyRootCodeGenerator.hh
 *@class  MHO_LegacyRootCodeGenerator
 *@author  J. Barrett - barrettj@mit.edu
 *@date Wed Apr 27 11:59:40 2022 -0400
 *@brief  generate the 6-character timestamp-like root codes for converted filenames
 */

/**
 * @brief Class MHO_LegacyRootCodeGenerator
 */
class MHO_LegacyRootCodeGenerator
{
    public:
        MHO_LegacyRootCodeGenerator(){};
        virtual ~MHO_LegacyRootCodeGenerator(){};

        //get a single code corresponding to the current time
        /**
         * @brief Getter for code
         * 
         * @return std::string containing the generated code.
         */
        std::string GetCode();

        //get a pre-assigned sequential list of N root codes
        //to avoid collisions/duplicates if sub-Delta-T time intervals
        //are encountered when processing a single experiment;
        /**
         * @brief Getter for codes
         * 
         * @param N Number of root codes to generate
         * @return Vector of generated root codes as strings
         */
        std::vector< std::string > GetCodes(std::size_t N);

    private:
        time_t fNow;
        int fYear;
        int fDay;
        int fHour;
        int fMin;
        int fSec;

        //essentially copied directly from the c utils library, to avoid introducing a library dependency
        //and converted to return a string
        /**
         * @brief Calculates and returns delta value based on current time comparison with HOPS_ROOT_BREAK.
         * 
         * @param now Current time in seconds since epoch
         * @return Delta value (4 if now < HOPS_ROOT_BREAK, otherwise 1)
         */
        int root_id_delta(time_t now);
        /**
         * @brief Generates a root ID string based on the input time_t value.
         * 
         * @param now Input time_t value used to generate the root ID.
         * @return Generated root ID as std::string.
         */
        std::string root_id_later(time_t now);
        /*! original implementation follows */
        /**
         * @brief Calculates and returns a root ID string based on input time parameters.
         * 
         * @param value (int)
         * @param value2 (int)
         * @param value3 (int)
         * @param value4 (int)
         * @param value5 (int)
         * @return A string representing the root ID.
         */
        std::string root_id(int year, int day, int hour, int min, int sec);
        /**
         * @brief Determines and returns root ID based on time parameters and comparison with HOPS_ROOT_BREAK.
         * 
         * @param now Current time in seconds since epoch
         * @param year Year component of time
         * @param day Day of year component of time
         * @param hour Hour component of time
         * @param min Minute component of time
         * @param sec Second component of time
         * @return Root ID as a string
         */
        std::string root_id_break(time_t now, int year, int day, int hour, int min, int sec);
};

} // namespace hops

#endif /*! end of include guard: MHO_LegacyRootCodeGenerator */
