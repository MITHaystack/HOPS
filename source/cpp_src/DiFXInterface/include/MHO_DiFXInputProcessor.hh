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
 *@date Mon Feb 21 00:33:14 2022 -0500
 *@brief
 */

/**
 * @brief Class MHO_DiFXInputProcessor
 */
class MHO_DiFXInputProcessor
{
    public:
        MHO_DiFXInputProcessor();
        virtual ~MHO_DiFXInputProcessor();

        /**
         * @brief Loads DiFX input file and stores it in member variable fD.
         * 
         * @param filename Path to the DiFX input file.
         */
        void LoadDiFXInputFile(std::string filename);
        /**
         * @brief Converts DiFX input data to JSON format and populates the provided mho_json object.
         * 
         * @param input Reference to an mho_json object that will be populated with converted data
         */
        void ConvertToJSON(mho_json& input);

    private:
        /**
         * @brief Copies base struct quantities from fD to input mho_json object.
         * 
         * @param input Reference to an mho_json object to store extracted quantities.
         */
        void ExtractBaseStructQuantities(mho_json& input);
        /**
         * @brief Extracts configuration quantities from DifxConfig at index n and returns as mho_json.
         * 
         * @param n Index of DifxConfig to extract quantities from
         * @return mho_json containing extracted configuration quantities
         */
        mho_json ExtractConfigQuantities(int n);
        /**
         * @brief Extracts frequency quantities from DifxFreq struct at index n and returns as mho_json.
         * 
         * @param n Index of DifxFreq struct to extract quantities from
         * @return mho_json containing extracted frequency quantities
         */
        mho_json ExtractFreqQuantities(int n);
        /**
         * @brief Extracts antenna quantities from DifxAntenna struct and returns as mho_json.
         * 
         * @param n Index of antenna to extract
         * @return mho_json containing extracted antenna quantities
         */
        mho_json ExtractAntennaQuantities(int n);
        /**
         * @brief Extracts scan quantities from DifxScan at index n and returns as mho_json.
         * 
         * @param n Index of DifxScan to extract quantities from
         * @return mho_json containing extracted scan quantities
         */
        mho_json ExtractScanQuantities(int n);
        /**
         * @brief Extracts source quantities from DifxSource at index n and returns as mho_json.
         * 
         * @param n Index of DifxSource to extract quantities from
         * @return mho_json containing extracted source quantities
         */
        mho_json ExtractSourceQuantities(int n);
        /**
         * @brief Extracts and returns EOP quantities as a JSON object for a given index.
         * 
         * @param n Index of the DifxEOP struct to extract quantities from
         * @return mho_json containing extracted EOP quantities (mjd, tai_utc, ut1_utc, xPole, yPole)
         */
        mho_json ExtractEOPQuantities(int n);
        /**
         * @brief Extracts and returns a JSON object containing various quantities from the specified datastream.
         * 
         * @param n Index of the datastream to extract quantities from.
         * @return A mho_json object containing extracted quantities.
         */
        mho_json ExtractDatastreamQuantities(int n);
        /**
         * @brief Extracts baseline quantities from DifxBaseline at index n and returns them as a JSON object.
         * 
         * @param n Index of the baseline to extract quantities from
         * @return JSON object containing dsA, dsB, nFreq, nPolProd arrays, bandA and bandB matrices
         */
        mho_json ExtractBaselineQuantities(int n);
        /**
         * @brief Extracts DifxPolyModel data and converts it to mho_json format.
         * 
         * @param m Input DifxPolyModel pointer
         * @return mho_json object containing extracted DifxPolyModel data
         */
        mho_json ExtractDifxPolyModel(DifxPolyModel* m);

        /**
         * @brief Getter for antenna mount type string
         * 
         * @param type Input AntennaMountType enum value
         * @return Encoded string for the input mount type
         */
        std::string GetAntennaMountTypeString(AntennaMountType type);
        /**
         * @brief Getter for antenna site type string
         * 
         * @param type Input AntennaSiteType enum value
         * @return std::string containing the corresponding antenna site type name
         */
        std::string GetAntennaSiteTypeString(AntennaSiteType type);

        DifxInput* fD;
        std::string fFilename;
};

} // namespace hops

#endif /*! end of include guard: MHO_DiFXInputProcessor */
