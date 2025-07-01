#ifndef MHO_MK4FringeExport_HH__
#define MHO_MK4FringeExport_HH__

//forward declaration of mk4 structs
//we do this to keep the mk4 structures from 'leaking' into the new code via includes,
//We want to make sure any interface to the old mk4 IO libraries is kept only
//within the MK4Interface library.
#ifndef HOPS3_USE_CXX
extern "C"
{
#endif

    /**
     * @brief Class type_000
     */
    struct type_000;
    /**
     * @brief Class type_200
     */
    struct type_200;
    /**
     * @brief Class type_201
     */
    struct type_201;
    /**
     * @brief Class type_202
     */
    struct type_202;
    /**
     * @brief Class type_203
     */
    struct type_203;
    /**
     * @brief Class type_204
     */
    struct type_204;
    /**
     * @brief Class type_205
     */
    struct type_205;
    /**
     * @brief Class type_206
     */
    struct type_206;
    /**
     * @brief Class type_207
     */
    struct type_207;
    /**
     * @brief Class type_208
     */
    struct type_208;
    /**
     * @brief Class type_210
     */
    struct type_210;
    /**
     * @brief Class type_212
     */
    struct type_212;
    /**
     * @brief Class type_220
     */
    struct type_220;
    /**
     * @brief Class type_221
     */
    struct type_221;
    /**
     * @brief Class type_222
     */
    struct type_222;
    /**
     * @brief Class type_230
     */
    struct type_230;
    /**
     * @brief Class mk4_fringe
     */
    struct mk4_fringe;
    /**
     * @brief Class sky_coord
     */
    struct sky_coord;
    /**
     * @brief Class date
     */
    struct date;
    /**
     * @brief Class ch_struct
     */
    struct ch_struct;

#ifndef HOPS3_USE_CXX
}
#endif

#include <iostream>
#include <string>

//data/config passing classes
#include "MHO_ContainerStore.hh"
#include "MHO_DirectoryInterface.hh"
#include "MHO_JSONHeaderWrapper.hh"
#include "MHO_OperatorToolbox.hh"
#include "MHO_ParameterStore.hh"
#include "MHO_Tokenizer.hh"

//needed to read hops files and extract objects from scan dir
#include "MHO_ScanDataStore.hh"

namespace hops
{

/*!
 *@file MHO_MK4FringeExport.hh
 *@class MHO_MK4FringeExport
 *@author J. Barrett - barrettj@mit.edu
 *@date Thu Jan 25 21:05:22 2024 -0500
 *@brief
 */

class MHO_MK4FringeExport
{
    public:
        MHO_MK4FringeExport();
        virtual ~MHO_MK4FringeExport();

        void SetParameterStore(MHO_ParameterStore* pStore) { fPStore = pStore; };

        void SetContainerStore(MHO_ContainerStore* cStore) { fCStore = cStore; };

        void SetPlotData(mho_json& plot_data) { fPlotData.FillData(plot_data); }

        void ExportFringeFile() { output(); }

    private:
        std::string CreateFringeFileName(std::string directory, int seq_no);

        MHO_ParameterStore* fPStore;
        MHO_ContainerStore* fCStore;
        MHO_ParameterStore fPlotData;

        void char_clear(char* arr, std::size_t n)
        {
            for(std::size_t i = 0; i < n; i++)
            {
                arr[i] = '\0';
            }
        }

        //utilty to convert source coordinate strings to numerical values
        int convert_sky_coords(struct sky_coord& coords, std::string ra, std::string dec);
        MHO_Tokenizer fTokenizer;

        //the fringe data to be filled and written
        struct mk4_fringe* fringe;

        //utilities
        void FillString(char* destination, std::string param_path, int max_length, std::string default_value = "");
        void FillString(std::string& destination, std::string param_path, std::string default_value = "");
        void FillInt(int& destination, std::string param_path, int default_value = 0);
        void FillShort(short& destination, std::string param_path, int default_value = 0);
        void FillDouble(double& destination, std::string param_path, double default_value = 0);
        void FillFloat(float& destination, std::string param_path, float default_value = 0);
        void FillDate(struct date* destination, std::string param_path);
        void FillDate(struct date* destination, struct legacy_hops_date& a_date);
        void FillChannels(struct ch_struct* chan_array);

        //the filling functions for each type
        int fill_200(struct type_200* t200);
        int fill_201(struct type_201* t201);
        int fill_202(struct type_202* t202);
        int fill_203(struct type_203* t203);
        int fill_204(struct type_204* t204);
        int fill_205(struct type_203* t203, struct type_205* t205);
        int fill_206(struct type_206* t206);
        int fill_207(struct type_207* t207);
        int fill_208(struct type_202* t202, struct type_208* t208);
        int fill_210(struct type_210* t210);
        int fill_212(int fr, struct type_212* t212);

        int fill_221(struct type_221** t221);

        int fill_222(struct type_222** t222);
        int fill_230(int fr, int ap, struct type_230* t230);

        int output();
};

} // namespace hops

#endif /*! end of include guard: MHO_MK4FringeExport */
