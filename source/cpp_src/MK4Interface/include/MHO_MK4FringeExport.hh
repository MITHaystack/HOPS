#ifndef MHO_MK4FringeExport_HH__
#define MHO_MK4FringeExport_HH__

/*
*File: MHO_MK4FringeExport.hh
*Class: MHO_MK4FringeExport
*Author: J. Barrett
*Email: barrettj@mit.edu
*Date: 2020-05-19T18:54:28.140Z
*Description:
*/

#include <iostream>
#include <string>

//mk4 IO library
#ifndef HOPS3_USE_CXX
extern "C"
{
#endif
    #include "mk4_data.h"
    #include "mk4_dfio.h"
#ifndef HOPS3_USE_CXX
}
#endif

//data/config passing classes
#include "MHO_ParameterStore.hh"
#include "MHO_ContainerStore.hh"
#include "MHO_OperatorToolbox.hh"
#include "MHO_JSONHeaderWrapper.hh"
#include "MHO_Tokenizer.hh"

//needed to read hops files and extract objects from scan dir
#include "MHO_ScanDataStore.hh"

namespace hops
{

class MHO_MK4FringeExport
{
    public:

        MHO_MK4FringeExport();
        virtual ~MHO_MK4FringeExport();

        void SetParameterStore(MHO_ParameterStore* pStore){fPStore = pStore;};
        void SetContainerStore(MHO_ContainerStore* cStore){fCStore = cStore;};

        void SetFilename(std::string filename){fFilename = filename;}

        void ExportFringeFile()
        {
            std::cout<<"filename = "<<fFilename<<std::endl;
            output(fFilename);
        }

    private:

        std::string fFilename;
        MHO_ParameterStore* fPStore;
        MHO_ContainerStore* fCStore;

        void char_clear(char* arr, std::size_t n){ for(std::size_t i=0; i<n; i++){arr[i] = '\0';} }

        //utilty to convert source coordinate strings to numerical values 
        int convert_sky_coords(struct sky_coord& coords, std::string ra, std::string dec);
        MHO_Tokenizer fTokenizer;

        //the fringe data to be filled and written
        struct mk4_fringe fringe;

        //utilities 
        void FillString(char* destination, std::string param_path, int max_length, std::string default_value="");
        void FillInt(int& destination, std::string param_path, int default_value = 0);
        void FillDouble(double& destination, std::string param_path, double default_value = 0);
        void FillDate(struct date* destination, std::string param_path);
        void FillDate(struct date* destination, struct legacy_hops_date& a_date);
        void FillChannels(struct ch_struct* chan_array, std::size_t nchannels);

        //the filling functions for each type
        int fill_200( struct type_200 *t200);
        int fill_201( struct type_201 *t201);
        int fill_202( struct type_202 *t202);
        int fill_203( struct type_203 *t203);
        int fill_204( struct type_204 *t204);
        int fill_205( struct type_203 *t203, struct type_205 *t205);
        int fill_206( struct type_206 *t206);
        int fill_207( struct type_207 *t207);
        int fill_208( struct type_202 *t202, struct type_208 *t208);
        int fill_210( struct type_210 *t210);
        int fill_212( int fr, struct type_212 *t212);
        int fill_221( struct type_221* t221);

        int fill_222( struct type_222 **t222);
        int fill_230( int fr, int ap, struct type_230 *t230);

        int fill_fringe_info(char *filename);
        int output(std::string filename);

        //original function prototypes (these also use a varying number of 'extern' global variables)
        //these are listed here for reference (showing some of the objects the require for data retrieval)

        // int fill_200( struct scan_struct *root, struct type_param *param, struct type_200 *t200);
        // int fill_201( struct scan_struct *root, struct type_param *param, struct type_201 *t201);
        // int fill_202( struct vex *root, struct type_param *param, struct type_202 *t202);
        // int fill_203( struct scan_struct *root, struct type_param *param, struct type_203 *t203);
        // int fill_204( struct type_204 *t204);
        // int fill_205( struct scan_struct *root, struct type_pass *pass, struct type_param *param, struct type_203 *t203, struct type_205 *t205);
        // int fill_206( struct scan_struct *root, struct type_pass *pass, struct type_param *param, struct type_status *status, struct type_206 *t206);
        // int fill_207(struct type_pass *pass, struct type_status *status, struct type_param *param, struct type_207 *t207);
        // int fill_208( struct type_pass *pass, struct type_param *param, struct type_status *status, struct type_202 *t202, struct type_208 *t208);
        // int fill_210( struct type_pass *pass, struct type_status *status, struct type_210 *t210);
        // int fill_212( struct type_pass *pass, struct type_status *status, struct type_param *param, int fr, struct type_212 *t212);
        // int fill_222( struct type_param *param, struct type_222 **t222);
        // int fill_230( struct type_pass *pass, struct type_param *param, int fr, int ap, struct type_230 *t230);
        // int fill_fringe_info( struct vex *root, struct type_pass *pass, char *filename);
        //int output (struct vex* root, struct type_pass* pass)

};

}//end of hops namespace

#endif /* end of include guard: MHO_MK4FringeExport */
