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

#include "fill_fblock.hh"


namespace hops 
{

class MHO_DiFXInputProcessor
{
    public:

        MHO_DiFXInputProcessor();
        virtual ~MHO_DiFXInputProcessor();

        void LoadDiFXInputFile(std::string filename);
        void ConvertToJSON(json& input);
        void FillFrequencyTable();

    private:

        void ExtractBaseStructQuantities(json& input);
        json ExtractConfigQuantities(int n);
        json ExtractFreqQuantities(int n);
        json ExtractAntennaQuantities(int n);
        json ExtractScanQuantities(int n);
        json ExtractSourceQuantities(int n);
        json ExtractEOPQuantities(int n);
        json ExtractDatastreamQuantities(int n);
        json ExtractBaselineQuantities(int n);
        json ExtractDifxPolyModel(DifxPolyModel* m);

        std::string GetAntennaMountTypeString(AntennaMountType type);
        std::string GetAntennaSiteTypeString(AntennaSiteType type);

        DifxInput* fD;


};

}

#endif /* end of include guard: MHO_DiFXInputProcessor */