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


class MHO_LegacyRootCodeGenerator
{
    public:

        MHO_LegacyRootCodeGenerator(){};
        virtual ~MHO_LegacyRootCodeGenerator(){};

        //get a single code corresponding to the current time
        std::string GetCode();

        //get a pre-assigned sequential list of N root codes
        //to avoid collisions/duplicates if sub-Delta-T time intervals
        //are encountered when processing a single experiment;
        std::vector<std::string> GetCodes(std::size_t N);

    private:

        time_t fNow;
        int fYear;
        int fDay;
        int fHour;
        int fMin;
        int fSec;

        //essentially copied directly from the c utils library, to avoid introducing a library dependency
        //and converted to return a string
        int root_id_delta(time_t now);
        std::string root_id_later(time_t now);
        /*! original implementation follows */
        std::string root_id(int year, int day, int hour, int min, int sec);
        std::string root_id_break(time_t now, int year, int day, int hour, int min, int sec);

};

}//end namespace

#endif /*! end of include guard: MHO_LegacyRootCodeGenerator */
