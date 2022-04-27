#ifndef MHO_LegacyRootCodeGenerator_HH__
#define MHO_LegacyRootCodeGenerator_HH__

/*
*@file: MHO_LegacyRootCodeGenerator.hh
*@class: MHO_LegacyRootCodeGenerator
*@author: J. Barrett
*@email: barrettj@mit.edu
*@date:
*@brief: generate the 6-character timestamp-like root codes for converted filenames
*/

namespace hops 
{


class MHO_LegacyRootCodeGenerator
{
    public:

        MHO_LegacyRootCodeGenerator();
        virtual ~MHO_LegacyRootCodeGenerator();

        //get a single code corresponding to the current time
        std::string GetCode(time_t now);

        //get a pre-assigned sequential list of N root codes
        //(to avoid collisions at sub-Delta-T time intervals for a single experiment;
        std::vector<std::string> GetCodes(time_t now, std::size_t N);

    private:

        //copied wholly from the c utils library, to avoid introducing a library dependency
        int root_id_delta(time_t now)
        char* root_id_later(time_t now);
        /* original implementation follows */
        char* root_id(int year, int day, int hour, int min, int sec);
        char* root_id_break(time_t now, int year, int day, int hour, int min, int sec);

};

}//end namespace

#endif /* end of include guard: MHO_LegacyRootCodeGenerator */