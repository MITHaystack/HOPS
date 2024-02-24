#ifndef MHO_DiFXInputProcessor_HH__
#define MHO_DiFXInputProcessor_HH__

#include "MHO_JSONHeaderWrapper.hh"
#include "MHO_Message.hh"


#include "difxio/difx_input.h"

namespace hops
{

/*!
*@file  MHO_DiFXInputProcessor.hh
*@class  MHO_DiFXInputProcessor
*@author  J. Barrett - barrettj@mit.edu
*@date
*@brief
*/

class MHO_DiFXInputProcessor
{
    public:

        MHO_DiFXInputProcessor();
        virtual ~MHO_DiFXInputProcessor();

        void LoadDiFXInputFile(std::string filename);
        void ConvertToJSON(mho_json& input);

    private:

        void ExtractBaseStructQuantities(mho_json& input);
        mho_json ExtractConfigQuantities(int n);
        mho_json ExtractFreqQuantities(int n);
        mho_json ExtractAntennaQuantities(int n);
        mho_json ExtractScanQuantities(int n);
        mho_json ExtractSourceQuantities(int n);
        mho_json ExtractEOPQuantities(int n);
        mho_json ExtractDatastreamQuantities(int n);
        mho_json ExtractBaselineQuantities(int n);
        mho_json ExtractDifxPolyModel(DifxPolyModel* m);

        std::string GetAntennaMountTypeString(AntennaMountType type);
        std::string GetAntennaSiteTypeString(AntennaSiteType type);

        DifxInput* fD;


};

}

#endif /*! end of include guard: MHO_DiFXInputProcessor */
