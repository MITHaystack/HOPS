#ifndef MHO_Snapshot_HH__
#define MHO_Snapshot_HH__

#include <cstdlib>
#include <ostream>
#include <sstream>
#include <string>
#include <iostream>
#include <set>
#include <unistd.h>

//global messaging util
#include "MHO_Message.hh"

//handles reading directories, listing files etc.
#include "MHO_DirectoryInterface.hh"

//needed to read hops files and extract objects
#include "MHO_ContainerDefinitions.hh"
#include "MHO_ContainerStore.hh"
#include "MHO_ContainerDictionary.hh"
#include "MHO_ContainerFileInterface.hh"

//hops snapshot directory should be defined as <install_dir>/snapshot/
#ifndef HOPS_SNAPSHOT_DIR
    #define HOPS_SNAPSHOT_DIR_STR "./"
#else
    #define STR(str) #str
    #define STRING(str) STR(str)
    #define HOPS_SNAPSHOT_DIR_STR STRING(HOPS_SNAPSHOT_DIR)
#endif

#pragma message HOPS_SNAPSHOT_DIR_STR

/*
*File: MHO_Snapshot.hh
*Class: MHO_Snapshot
*Author: J. Barrett
*Email: barrettj@mit.edu
*Date:
*Description:
*/

namespace hops
{



//uses the singleton pattern (dumps to only one file)
//TODO make this class thread safe
class MHO_Snapshot
{

    public:
        //since this is a singleton we need to remove ability to copy/move
        MHO_Snapshot(MHO_Snapshot const&) = delete;
        MHO_Snapshot(MHO_Snapshot&&) = delete;
        MHO_Snapshot& operator=(MHO_Snapshot const&) = delete;
        MHO_Snapshot& operator=(MHO_Snapshot&&) = delete;

        //provide public access to the only static instance
        static MHO_Snapshot& GetInstance()
        {
            if(fInstance == nullptr){fInstance = new MHO_Snapshot();}
            return *fInstance;
        }

        void SetExecutableName(std::string exe_name){fExeName = exe_name;};
        void SetExecutableName(const char* exe_name){ SetExecutableName(std::string(exe_name) ); }

        void AcceptAllKeys(){fAcceptAllKeys = true;}
        void LimitToKeySet(){fAcceptAllKeys = false;}
        void AddKey(const std::string& key);
        void AddKey(const char* key);
        void RemoveKey(const std::string& key);
        void RemoveKey(const char* key);
        void RemoveAllKeys();

        template< typename XObjType >
        void DumpObject(XObjType* obj, const char* key, const char* name)
        {
            DumpObject(obj, std::string(key), std::string(name) );
        }

        template< typename XObjType >
        void DumpObject(XObjType* obj, std::string key, std::string name)
        {
            if( PassSnapshot(key) )
            {
                std::string output_file = fPrefix + fExeName + fPostfix;
                MHO_BinaryFileInterface inter;
                bool status = inter.OpenToAppend(output_file);

                if(status)
                {
                    uint32_t label = fCountLabel;
                    inter.Write(*obj, name, label);
                    fCountLabel++;
                }
                else
                {
                    msg_error("file", "error writing object "<< name << " to file: " << output_file << eom);
                }
                inter.Close();
            }
        }

    private:

        int GetPID()
        {
            pid_t pid = getppid();
            return (int) pid;
        }

        //no public access to constructor
        //set up the stream, for now just point to std::cout
        //but we may want to allow this to be configured post-construction
        //perhaps we should also pipe information into log file(s)
        MHO_Snapshot():
            fCurrentKeyIsAllowed(false),
            fAcceptAllKeys(false)
        {
            std::string dir_string = HOPS_SNAPSHOT_DIR_STR;

            //dump bl_data into a file for later inspection
            std::stringstream ss;
            ss << dir_string;
            ss << "/pid_";
            ss << GetPID();
            ss << "_";

            std::string fPrefix = ss.str();
            std::string fPostfix = ".snap";
            fExeName = "";
        };
        virtual ~MHO_Snapshot(){};

        bool PassSnapshot(std::string key);

        static MHO_Snapshot* fInstance; //static global class instance

        //used to construct filename in which to dump snapshots
        int fPID;
        std::string fPrefix;
        std::string fPostfix;
        std::string fExeName;

        //label is used to count the number of snapshots we have dumped
        uint32_t fCountLabel;

        std::set< std::string > fKeys; //keys of which messages we will accept for output
        bool fCurrentKeyIsAllowed; //current key is in allowed set
        bool fAcceptAllKeys;


};


//
//
// //
// // //usage macros
// // #define msg_fatal(xKEY, xCONTENT) MHO_Snapshot::GetInstance().SendMessage(eFatal,xKEY) << xCONTENT;
// // #define msg_error(xKEY, xCONTENT) MHO_Snapshot::GetInstance().SendMessage(eError,xKEY) << xCONTENT;
// // #define msg_warn(xKEY, xCONTENT) MHO_Snapshot::GetInstance().SendMessage(eWarning,xKEY) << xCONTENT;
// // #define msg_status(xKEY, xCONTENT) MHO_Snapshot::GetInstance().SendMessage(eStatus,xKEY) << xCONTENT;
// // #define msg_info(xKEY, xCONTENT) MHO_Snapshot::GetInstance().SendMessage(eInfo,xKEY) << xCONTENT;
//
// #ifdef HOPS_ENABLE_DEBUG_MSG  //this is defined as a compiler flag via build system
// //#warning "HOPS_ENABLE_DEBUG_MSG is defined -- this is not a bug!"
// //allow debug messages when debug flag is active
// #define msg_debug(xKEY, xCONTENT) MHO_Snapshot::GetInstance().SendMessage(eDebug,xKEY) << xCONTENT;
// #else
// //debug is not enabled, so we remove them from compilation
// #define msg_debug(xKEY, xCONTENT)
// #endif
//
//





}//end of namespace

#endif /* end of include guard: MHO_Snapshot */
