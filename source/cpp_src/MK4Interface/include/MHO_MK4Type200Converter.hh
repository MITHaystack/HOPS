#ifndef MHO_MKType200Converter_HH__
#define MHO_MKType200Converter_HH__

//include the mk4 IO library
//The messy ifndef guards around the brackets are needed because the dfio library
//can optionally be compiled as either c or c++
#ifndef HOPS3_USE_CXX
extern "C"
{
#endif

    #include "mk4_data.h"
    #include "mk4_dfio.h"

#ifndef HOPS3_USE_CXX
}
#endif

//include the json library stuff through the header wrapper 
//that lives inc the utilities library 
#include "MHO_JSONHeaderWrapper.hh"


namespace hops 
{

/**
*@file MHO_MK4Type200Converter.hh
*@class MHO_MK4Type200Converter
*@author J. Barrett - barrettj@mit.edu, V. Pfeiffer - violetp@mit.edu
* A class that accepts the data from a type 200 file in the form of a struct and converts it to a JSON object.
**/

class MHO_MKType200Converter
{
    public:

        //boilerplate constructors/destructors
        MHO_MKType200Converter();
        virtual ~MHO_MKType200Converter();

        //sets the pointer to the type_200 object we want to convert
        void SetType200(type_200* type200ptr);

        //does the actual 'work' of extracting the information from the type_200 struct
        void ConvertToJSON();

        //returns a copy of the internal json object (filled from the type_200)
        //NOTE: we could also return a pointer instead (to save on the copy)
        //but for the time being we won't worry about that sort of optimization
        json GetJSON();

    private:

        //a pointer to an external type_200 structure we're going to convert
        type_200* fPtr;

        //a json object which we'll fill
        json fJSON;
};

}

#endif /* end of include guard: MHO_MKType200Converter */
