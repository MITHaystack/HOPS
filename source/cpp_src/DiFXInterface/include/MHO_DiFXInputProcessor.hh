#ifndef MHO_DiFXInputProcessor_HH__
#define MHO_DiFXInputProcessor_HH__

#include "MHO_JSONHeaderWrapper.hh"

/*
*@file: MHO_DiFXInputProcessor.hh
*@class: MHO_DiFXInputProcessor
*@author: J. Barrett
*@email: barrettj@mit.edu
*@date:
*@brief:
*/

#include "difxio/difx_input.h"

namespace hops 
{

class MHO_DiFXInputProcessor
{
    public:

        MHO_DiFXInputProcessor();
        virtual ~MHO_DiFXInputProcessor();

        void LoadDiFXInputFile(std::string filename);
        void ConvertToJSON(json& input);

    private:

        void ExtractBaseStructQuantities(json& input);
        json ExtractConfigQuantities(int n);

        DifxInput* fD;


};

}

#endif /* end of include guard: MHO_DiFXInputProcessor */