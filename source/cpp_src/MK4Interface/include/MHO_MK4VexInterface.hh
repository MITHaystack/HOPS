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

/**
 * @brief Class MHO_MK4VexInterface
 */
class MHO_MK4VexInterface
{
    public:
        MHO_MK4VexInterface();
        virtual ~MHO_MK4VexInterface();

        /**
         * @brief Opens a VEX file and parses its contents into an internal representation.
         * 
         * @param file_path Path to the VEX file to be opened and parsed
         */
        void OpenVexFile(std::string file_path);
        /**
         * @brief Getter for vex
         * 
         * @return mho_json containing vex data or an empty json object.
         */
        mho_json GetVex();

        /**
         * @brief Exports Vex file data to a JSON object if available.
         * 
         * @param json_obj Reference to an mho_json object that will receive the exported data.
         * @return True if successful, false if no Vex file data is available.
         */
        bool ExportVexFileToJSON(mho_json& json_obj);

    private:
        bool fHaveVex;
        mho_json fVex;
};

} // namespace hops

#endif /*! end of include guard: MHO_MK4VexInterface */
