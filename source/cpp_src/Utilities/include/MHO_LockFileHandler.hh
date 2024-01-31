#ifndef MHO_LockFileHandler_HH__
#define MHO_LockFileHandler_HH__

#include <cstdlib>
#include <csignal>
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


/*
*File: MHO_LockFileHandler.hh
*Class: MHO_LockFileHandler
*Author: J. Barrett
*Email: barrettj@mit.edu
*Date:
*Description: ported version of fourfit write lock mechanism
*/


//number of chars in lock file name
#define MAX_LOCKNAME_LEN 512

namespace hops
{

//uses the singleton pattern
class MHO_LockFileHandler
{
    public:
        //since this is a singleton we need to remove ability to copy/move
        MHO_LockFileHandler(MHO_LockFileHandler const&) = delete;
        MHO_LockFileHandler(MHO_LockFileHandler&&) = delete;
        MHO_LockFileHandler& operator=(MHO_LockFileHandler const&) = delete;
        MHO_LockFileHandler& operator=(MHO_LockFileHandler&&) = delete;

        //struct for holding data about the lock file's creation
        struct lockfile_data
        {
            int validity;
            unsigned int seq_number;
            unsigned int pid;
            unsigned long int time_sec;
            unsigned long int time_usec;
            char hostname[256];
            char lockfile_name[MAX_LOCKNAME_LEN];
        };

        //provide public access to the only static instance
        static MHO_LockFileHandler& GetInstance()
        {
            if(fInstance == nullptr){fInstance = new MHO_LockFileHandler();}
            return *fInstance;
        }

        //function to handle signals (to ensure we clean up lockfiles if we get interrupted)
        static void HandleSignal(int signal_value) 
        {
            MHO_LockFileHandler::GetInstance().remove_lockfile(); //just make sure we remove the lock file
            signal(signal_value, SIG_DFL); //reset the handler for this particular signal to default
            kill(getpid(), signal_value); //re-send the signal to this process
        }
        
        //set the write directory
        void SetDirectory(std::string dir){fDirectory = dir;}
        
        void init_lockfile_data(lockfile_data* data);
        void clear();
        void remove_lockfile();
        int parse_lockfile_name(char* lockfile_name_base, lockfile_data* result);
        int create_lockfile(char* lockfile_name, int cand_seq_no);
        int check_stale(lockfile_data* other);
        int lock_has_priority(lockfile_data* other);
        int at_front(char* lockfile_name, int cand_seq_no);
        int wait_for_write_lock(char* lockfile_name, int& next_seq_no);


    private:

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
        };
        virtual ~MHO_LockFileHandler(){};

        //static global class instance
        static MHO_LockFileHandler* fInstance;
        //info of the current process
        lockfile_data fProcessLockFileData;
        //directory interface 
        std::string fDirectory;
        MHO_DirectoryInterface fDirInterface;

};


}//end of namespace

#endif /* end of include guard: MHO_LockFileHandler */
