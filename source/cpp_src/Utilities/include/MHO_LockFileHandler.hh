#ifndef MHO_LockFileHandler_HH__
#define MHO_LockFileHandler_HH__

#include <cstdlib>
#include <csignal>

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// #include "fourfit_signal_handler.h"
// #include "write_lock_mechanism.h"
// #include "msg.h"

/*
*File: MHO_LockFileHandler.hh
*Class: MHO_LockFileHandler
*Author: J. Barrett
*Email: barrettj@mit.edu
*Date:
*Description:
*/

//number of chars in lock file name
#define MAX_LOCKNAME_LEN 512

//wait time allowed before a lock file is declared stale
//this is 5 minutes...probably much longer than needed
#define LOCK_STALE_SEC 300

//number of lock attempts before time-out error
//this is roughly 15 minutes...probably much longer than needed
#define LOCK_TIMEOUT 9000

//struct validity
#define LOCK_VALID 0
#define LOCK_INVALID -1

//return error codes
#define LOCK_FILESET_FAIL -6
#define LOCK_TIMEOUT_ERROR -5
#define LOCK_FILE_ERROR -4
#define LOCK_STALE_ERROR -3
#define LOCK_PARSE_ERROR -2
#define LOCK_PROCESS_NO_PRIORITY -1
#define LOCK_STATUS_OK 0
#define LOCK_PROCESS_HAS_PRIORITY 1

namespace hops
{

//uses the singleton pattern (as we only have one terminal)
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

        //function to handle signals (to ensure we clean up lockfiles if we get interrupted)
        static void HandleSignal(int signal_value) 
        {
            remove_lockfile(); //just make sure we remove the lock file
            signal(signal_value, SIG_DFL); //reset the handler for this signal to be the default
            kill(getpid(), signal_value); //re-send the signal to this process
        }
        
        void init_lockfile_data(lockfile_data_struct* data);
        void clear_global_lockfile_data();
        void remove_lockfile(); //must go through global variables
        int parse_lockfile_name(char* lockfile_name_base, lockfile_data_struct* result);
        int create_lockfile(char *rootname, char* lockfile_name, int cand_seq_no);
        int check_stale(lockfile_data_struct* other);
        int lock_has_priority(lockfile_data_struct* other);
        int at_front(char* rootname, char* lockfile_name, int cand_seq_no);
        int wait_for_write_lock(char* rootname, char* lockfile_name, struct fileset *fset);


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
        };


        virtual ~MHO_LockFileHandler(){};

        //struct of holding data about the lock file's creation
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




};


}//end of namespace

#endif /* end of include guard: MHO_LockFileHandler */
