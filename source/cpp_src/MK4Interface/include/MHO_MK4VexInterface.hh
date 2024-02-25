#ifndef MHO_MK4VexInterface_HH__
#define MHO_MK4VexInterface_HH__


#include <iostream>
#include <string>

#include "MHO_JSONHeaderWrapper.hh"
#include "MHO_VexParser.hh"

namespace hops
{

/*!
*@file MHO_MK4VexInterface.hh
*@class MHO_MK4VexInterface
*@author J. Barrett - barrettj@mit.edu
*@date Tue May 19 01:47:28 2020 -0400 
*@brief
*/


class MHO_MK4VexInterface
{
    public:

        MHO_MK4VexInterface();
        virtual ~MHO_MK4VexInterface();

        void OpenVexFile(std::string file_path);
        mho_json GetVex();

        bool ExportVexFileToJSON(mho_json& json_obj);

    private:

        bool fHaveVex;
        mho_json fVex;



};


}//end of hops namespace

#endif /*! end of include guard: MHO_MK4VexInterface */
