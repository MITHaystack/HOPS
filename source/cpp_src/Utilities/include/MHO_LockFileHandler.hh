#ifndef MHO_LockFileHandler_HH__
#define MHO_LockFileHandler_HH__

/*!
*@file MHO_LockFileHandler.hh
*@class MHO_LockFileHandler
*@author J. Barrett - barrettj@mit.edu
*@date
*@brief ported version of fourfit write lock mechanism
*/

#include <cstdlib>
#include <csignal>
#include <unistd.h>

#include "MHO_Tokenizer.hh"
#include "MHO_DirectoryInterface.hh"

//return error codes
#define LOCK_FILESET_FAIL -6
#define LOCK_TIMEOUT_ERROR -5
#define LOCK_FILE_ERROR -4
#define LOCK_STALE_ERROR -3
#define LOCK_PARSE_ERROR -2
#define LOCK_PROCESS_NO_PRIORITY -1
#define LOCK_STATUS_OK 0
#define LOCK_PROCESS_HAS_PRIORITY 1

//number of chars in lock file name
#define MAX_LOCKNAME_LEN 512

namespace hops
{

//struct for holding data about the lock file's creation
struct lockfile_data
{
    int validity;
    unsigned int seq_number;
    unsigned int pid;
    unsigned long int time_sec;
    unsigned long int time_usec;
    char hostname[256];
    char active_directory[MAX_LOCKNAME_LEN];
    char lockfile_name[MAX_LOCKNAME_LEN];
};



//uses the singleton pattern
class MHO_LockFileHandler
{
    public:
        //since this is a singleton we need to remove ability to copy/move
        MHO_LockFileHandler(MHO_LockFileHandler const&) = delete;
        MHO_LockFileHandler(MHO_LockFileHandler&&) = delete;
        MHO_LockFileHandler& operator=(MHO_LockFileHandler const&) = delete;
        MHO_LockFileHandler& operator=(MHO_LockFileHandler&&) = delete;

        //provide public access to the only static instance
        static MHO_LockFileHandler& GetInstance()
        {
            if(fInstance == nullptr){fInstance = new MHO_LockFileHandler();}
            return *fInstance;
        }

        //configure the lock handler to write legacy type_2xx files (e.g. GE.X.1.ABCDEF)
        //or to use the new file naming convention (.frng extension)
        //legacy mode is enabled by default
        void EnableLegacyMode(){fEnableLegacyMode = true;};
        void DisableLegacyMode(){fEnableLegacyMode = false;};

        //the only three functions user needs to call via the instance:
        //(1) enable/disable legacy mode
        //(2) wait for lock
        // write out the data file
        //(3) remove lock
        int WaitForWriteLock(std::string directory, int& next_seq_no);
        void RemoveWriteLock();

    private:

        static void HandleSignal(int signal_value);

        static void init_lockfile_data(lockfile_data* data);
        static int parse_lockfile_name(char* lockfile_name_base, lockfile_data* result);
        static int create_lockfile(const char* directory, char* lockfile_name, lockfile_data* lock_data, int max_seq_no);
        static int check_stale(lockfile_data* other);
        static int lock_has_priority(lockfile_data* ours, lockfile_data* other);
        static int at_front(const char* directory, char* lockfile_name, lockfile_data* lock_data, int cand_seq_no);
        static void remove_lockfile(lockfile_data* other);

        int wait_for_write_lock(int& next_seq_no);
        int get_max_seq_number(std::string dir);

        void SetDirectory(std::string dir);

        MHO_LockFileHandler()
        {
            //register the various signals
            std::signal(SIGINT, &HandleSignal);
            std::signal(SIGTERM, &HandleSignal);
            std::signal(SIGQUIT, &HandleSignal);
            std::signal(SIGSEGV, &HandleSignal);
            std::signal(SIGBUS, &HandleSignal);
            std::signal(SIGHUP, &HandleSignal);
            std::signal(SIGABRT, &HandleSignal);
            fDirectory = "./";
            fEnableLegacyMode = true;
            fTokenizer.SetDelimiter(".");
            fTokenizer.SetIncludeEmptyTokensFalse();
            init_lockfile_data(&fProcessLockFileData);
        };
        virtual ~MHO_LockFileHandler(){};

        //static global class instance
        static MHO_LockFileHandler* fInstance;
        //info of the current process
        lockfile_data fProcessLockFileData;
        //directory interface
        std::string fDirectory;
        MHO_DirectoryInterface fDirInterface;
        MHO_Tokenizer fTokenizer;

        bool fEnableLegacyMode;

};


}//end of namespace

#endif /*! end of include guard: MHO_LockFileHandler */
