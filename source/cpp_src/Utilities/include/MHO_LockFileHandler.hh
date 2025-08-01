#ifndef MHO_LockFileHandler_HH__
#define MHO_LockFileHandler_HH__

#include <csignal>
#include <cstdlib>
#include <unistd.h>

#include "MHO_DirectoryInterface.hh"
#include "MHO_Tokenizer.hh"

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

/*!
 *@file MHO_LockFileHandler.hh
 *@class MHO_LockFileHandler
 *@author J. Barrett - barrettj@mit.edu
 *@date Tue Jan 30 23:48:59 2024 -0500
 *@brief ported version of fourfit write lock mechanism
 * the only three functions user needs to call via the instance:
 * (1) enable/disable legacy mode
 * (2) wait for lock
 * - write out the data file
 * (3) remove lock
 */

/**
 * @brief struct lockfile_data - struct for holding data about the lock file's creation
 */
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

/**
 * @brief Class MHO_LockFileHandler uses the singleton pattern
 */
class MHO_LockFileHandler
{
    public:
        //since this is a singleton we need to remove ability to copy/move
        MHO_LockFileHandler(MHO_LockFileHandler const&) = delete;
        MHO_LockFileHandler(MHO_LockFileHandler&&) = delete;
        MHO_LockFileHandler& operator=(MHO_LockFileHandler const&) = delete;
        MHO_LockFileHandler& operator=(MHO_LockFileHandler&&) = delete;

        /**
         * @brief provide public access to the only static instance
         *
         * @return Reference to the singleton instance of MHO_LockFileHandler
         * @note This is a static function.
         */
        static MHO_LockFileHandler& GetInstance()
        {
            if(fInstance == nullptr)
            {
                fInstance = new MHO_LockFileHandler();
            }
            return *fInstance;
        }

        /**
         * @brief configure the lock handler to write legacy type_2xx file (e.g. GE.X.1.ABCDEF) naming convention.
         * or to use the new file naming convention (.frng extension)
         * legacy mode is enabled by default
         */
        void EnableLegacyMode() { fEnableLegacyMode = true; };

        /**
         * @brief Disables legacy mode by setting fEnableLegacyMode to false.
         */
        void DisableLegacyMode() { fEnableLegacyMode = false; };

        /**
         * @brief Waits for and acquires a write lock on the specified directory, setting it as the current directory.
         *
         * @param directory The directory to acquire the write lock on.
         * @param next_seq_no (int&)
         * @return An integer representing the result of the operation.
         */
        int WaitForWriteLock(std::string directory, int& next_seq_no);
        /**
         * @brief Removes a write lock from the file.
         */
        void RemoveWriteLock();

    private:
        /**
         * @brief Removes write lock and resets signal handler to default before re-sending the signal to the process.
         *
         * @param signal_value Signal value to be handled
         * @note This is a static function.
         */
        static void HandleSignal(int signal_value);

        /**
         * @brief Initializes lockfile_data struct to default values.
         *
         * @param data Pointer to lockfile_data struct to initialize
         * @note This is a static function.
         */
        static void init_lockfile_data(lockfile_data* data);

        /**
         * @brief Parses a lockfile name into its constituent components and stores them in result.
         *
         * @param lockfile_name_base Input lockfile name to be parsed
         * @param result (lockfile_data*)
         * @return 0 on success, LOCK_PARSE_ERROR on failure
         * @note This is a static function.
         */
        static int parse_lockfile_name(char* lockfile_name_base, lockfile_data* result);

        /**
         * @brief Creates a lockfile in the specified directory with given name and data, using current process ID, hostname, and timestamp.
         *
         * @param directory Input directory path where the lockfile will be created
         * @param lockfile_name Output buffer for the generated lockfile name
         * @param lock_data Pointer to store lockfile metadata (validity, seq_number, pid, time_sec)
         * @param max_seq_no Maximum file extent number seen at time of lock file creation
         * @return 0 on success, non-zero on failure
         * @note This is a static function.
         */
        static int create_lockfile(const char* directory, char* lockfile_name, lockfile_data* lock_data, int max_seq_no);

        /**
         * @brief Checks if another lockfile is stale and returns appropriate status.
         *
         * @param other Pointer to another lockfile_data structure.
         * @return LOCK_STATUS_OK if stale, LOCK_STALE_ERROR otherwise.
         * @note This is a static function.
         */
        static int check_stale(lockfile_data* other);

        /**
         * @brief Determines priority between two lock processes based on PID and timestamps.
         *
         * @param ours Pointer to our lockfile_data structure
         * @param other Pointer to another process's lockfile_data structure
         * @return LOCK_PROCESS_HAS_PRIORITY, LOCK_PROCESS_NO_PRIORITY or LOCK_STALE_ERROR based on comparison results.
         * @note This is a static function.
         */
        static int lock_has_priority(lockfile_data* ours, lockfile_data* other);
        /**
         * @brief Checks if a process has priority to create a lock file in a given directory.
         *
         * @param directory Input directory path where lock files are stored
         * @param lockfile_name Output lock file name if created
         * @param lock_data Output lock file data if created
         * @param cand_seq_no Candidate sequence number for the new lock file
         * @return Status code indicating success or error (LOCK_STATUS_OK, LOCK_PARSE_ERROR, LOCK_STALE_ERROR, LOCK_FILE_ERROR)
         * @note This is a static function.
         */
        static int at_front(const char* directory, char* lockfile_name, lockfile_data* lock_data, int cand_seq_no);

        /**
         * @brief Removes a lockfile if it's valid and outputs debug message.
         *
         * @param other (lockfile_data*)
         * @note This is a static function.
         */
        static void remove_lockfile(lockfile_data* other);

        /**
         * @brief Waits for this process to be at the front of the write queue and returns the next sequence number.
         *
         * @param next_seq_no Output parameter: Next sequence number after acquiring the lock
         * @return LOCK_STATUS_OK on success, error codes otherwise
         */
        int wait_for_write_lock(int& next_seq_no);

        /**
         * @brief Finds and returns the maximum sequence number among fringe files in the given directory.
         *
         * @param dir Input directory path where fringe files are located
         * @return Maximum sequence number found among fringe files
         */
        int get_max_seq_number(std::string dir);

        /**
         * @brief Setter for directory
         *
         * @param dir The new directory path to set.
         */
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

} // namespace hops

#endif /*! end of include guard: MHO_LockFileHandler */
