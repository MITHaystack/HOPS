#ifndef MHO_DiFXScanFileSet_HH__
#define MHO_DiFXScanFileSet_HH__

#include "MHO_Message.hh"
#include <string>
#include <vector>

namespace hops
{

/*!
 *@file  MHO_DiFXScanFileSet.hh
 *@class  MHO_DiFXScanFileSet
 *@author  J. Barrett - barrettj@mit.edu
 *@date Mon Jan 31 15:48:42 2022 -0500
 *@brief stores the names of all the assorted files related to a difx scan
 */

/**
 * @brief Class MHO_DiFXScanFileSet
 */
class MHO_DiFXScanFileSet
{
    public:
        MHO_DiFXScanFileSet()
        {
            //direct public access since all we are doing is get/set anyways
            fIndex = 0;
            fLocalIndex = 0;
            fScanName = "";
            fInputBaseDirectory = "";
            fOutputBaseDirectory = "";
            fScanDirectory = "";
            fInputFile = "";
            fIMFile = "";
            fCalcFile = "";
            fFlagFile = "";
            fV2DFile = "";
            fVexFile = "";
            fVisibilityFileList.clear();
            fPCALFileList.clear();
        };

        virtual ~MHO_DiFXScanFileSet(){};

        //direct public access since all we are doing is get/set anyways
        std::size_t fIndex; //numerical index
        std::size_t fLocalIndex;
        std::string fScanName;                          //difx scan name
        std::string fInputBaseDirectory;                //root input directory for this experiment
        std::string fOutputBaseDirectory;               //root output directory for this experiment
        std::string fScanDirectory;                     //.difx directory
        std::string fInputFile;                         //.input file associated with this scan
        std::string fIMFile;                            //.im file associated with this scan
        std::string fCalcFile;                          //.calc file associated with this scan
        std::string fFlagFile;                          //.flag file associated with this scan
        std::string fV2DFile;                           //.v2d file for this experiment
        std::string fVexFile;                           //.vex file for this experiment
        std::vector< std::string > fVisibilityFileList; //list of all DIFX_ files under the .difx directory
        std::vector< std::string > fPCALFileList;       //list of all PCAL_ files under the .difx directory

        //flag which indicates if this file set has all the necessary bits to be converted
        /**
         * @brief Checks if all necessary files and directories are set for conversion.
         *
         * @return True if complete, false otherwise.
         */
        bool IsComplete() const
        {
            //fFlagFile and fV2DFile are optional
            if(fScanName == "")
            {
                return false;
            }
            if(fInputBaseDirectory == "")
            {
                return false;
            }
            if(fOutputBaseDirectory == "")
            {
                return false;
            }
            if(fScanDirectory == "")
            {
                return false;
            }
            if(fInputFile == "")
            {
                return false;
            }
            if(fIMFile == "")
            {
                return false;
            }
            if(fCalcFile == "")
            {
                return false;
            }
            if(fVexFile == "")
            {
                return false;
            }
            if(fVisibilityFileList.size() == 0)
            {
                return false;
            }
            if(fPCALFileList.size() == 0)
            {
                return false;
            }
            return true;
        }

        /**
         * @brief Function PrintSummary
         */
        void PrintSummary() const
        {
            msg_debug("difx_interface", "scan summary: " << eol);
            msg_debug("difx_interface", "index: " << fIndex << eol);
            msg_debug("difx_interface", "name: " << fScanName << eol);
            msg_debug("difx_interface", ".input file: " << fInputFile << eol);
            msg_debug("difx_interface", ".im file: " << fIMFile << eol);
            msg_debug("difx_interface", ".calc file: " << fCalcFile << eol);
            msg_debug("difx_interface", ".flag file: " << fFlagFile << eol);
            msg_debug("difx_interface", ".v2d file: " << fV2DFile << eol);
            msg_debug("difx_interface", ".vex file: " << fVexFile << eol);
            for(std::size_t i = 0; i < fVisibilityFileList.size(); i++)
            {
                msg_debug("difx_interface", "visibility file # " << i << " : " << fVisibilityFileList[i] << eol);
            }
            for(std::size_t i = 0; i < fPCALFileList.size(); i++)
            {
                msg_debug("difx_interface", "pcal file # " << i << " : " << fPCALFileList[i] << eol);
            }
            msg_debug("difx_interface", eom);
        }
};

} // namespace hops

#endif /*! end of include guard: MHO_DiFXScanFileSet */
