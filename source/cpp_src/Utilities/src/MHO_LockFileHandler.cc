#include "MHO_LockFileHandler.hh"

#include <cerrno>
#include <ctime>
#include <dirent.h>
#include <random>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>

//wait time allowed before a lock file is declared stale (2 min)
#define LOCK_STALE_SEC 120

//number of lock attempts before time-out error
//this is roughly 5 minutes...probably much longer than needed
#define LOCK_TIMEOUT 3000

//struct validity
#define LOCK_VALID 0
#define LOCK_INVALID -1

namespace hops
{

//initialization to nullptr
MHO_LockFileHandler* MHO_LockFileHandler::fInstance = nullptr;

//function to handle signals (to ensure we clean up lockfiles if we get interrupted)
void MHO_LockFileHandler::HandleSignal(int signal_value)
{
    MHO_LockFileHandler::GetInstance().RemoveWriteLock();
    // Call exit() so that static-duration destructors (e.g. temp-dir cleanup) run.
    // Use 128 + signal_value to preserve the conventional shell exit code for signal termination.
    exit(128 + signal_value);
}

//set the write directory
void MHO_LockFileHandler::SetDirectory(std::string dir)
{
    fDirectory = dir;
    //make sure our directory is terminated with a "/"
    //this needs to be the case for the dir/file parsing code
    if(fDirectory.size() != 0)
    {
        if(fDirectory[fDirectory.size() - 1] != '/')
        {
            fDirectory += "/";
        }
    }
}

void MHO_LockFileHandler::RemoveWriteLock()
{
    remove_lockfile(&fProcessLockFileData);
}

int MHO_LockFileHandler::WaitForWriteLock(std::string directory, int& next_seq_no)
{
    SetDirectory(directory);
    return wait_for_write_lock(next_seq_no);
}

void MHO_LockFileHandler::init_lockfile_data(lockfile_data* data)
{
    data->validity = LOCK_INVALID;
    data->seq_number = 0;
    data->pid = 0;
    data->time_sec = 0;
    data->time_usec = 0;
    int i = 0;
    for(i = 0; i < 256; i++)
    {
        data->hostname[i] = '\0';
    }
    for(i = 0; i < MAX_LOCKNAME_LEN; i++)
    {
        data->active_directory[i] = '\0';
    }
    for(i = 0; i < MAX_LOCKNAME_LEN; i++)
    {
        data->lockfile_name[i] = '\0';
    }
}

void MHO_LockFileHandler::remove_lockfile(lockfile_data* data)
{
    if(data->validity == LOCK_VALID)
    {
        msg_debug("utilities", "removing write lock file: " << std::string(data->lockfile_name) << eom);
        remove(data->lockfile_name);
    }
    init_lockfile_data(data);
}

int MHO_LockFileHandler::parse_lockfile_name(char* lockfile_name_base, lockfile_data* result)
{

    init_lockfile_data(result);

    strcpy(result->lockfile_name, lockfile_name_base);

    //tokenize the lockfile name base, (note: this modifies the input)
    char* ptr;
    ptr = strtok(lockfile_name_base, ".");

    if(ptr != NULL)
    {
        sscanf(ptr, "%u", &(result->pid));
    }
    else
    {
        return LOCK_PARSE_ERROR;
    }

    ptr = strtok(NULL, ".");

    if(ptr != NULL)
    {
        sscanf(ptr, "%u", &(result->seq_number));
    }
    else
    {
        return LOCK_PARSE_ERROR;
    }

    ptr = strtok(NULL, ".");

    if(ptr != NULL)
    {
        sscanf(ptr, "%lx", &(result->time_sec));
    }
    else
    {
        return LOCK_PARSE_ERROR;
    }

    ptr = strtok(NULL, ".");

    if(ptr != NULL)
    {
        sscanf(ptr, "%lx", &(result->time_usec));
    }
    else
    {
        return LOCK_PARSE_ERROR;
    }

    ptr = strtok(NULL, ".");

    if(ptr != NULL)
    {
        sscanf(ptr, "%s", &(result->hostname[0]));
    }
    else
    {
        return LOCK_PARSE_ERROR;
    }

    result->validity = LOCK_VALID;
    return 0;
}

int MHO_LockFileHandler::check_stale(lockfile_data* other)
{
    //returns LOCK_STALE_ERROR if the other lock is stale (its owner appears dead,
    //  so the lock may be reclaimed)
    //returns LOCK_STATUS_OK if the lock is still considered live (must be respected)
    //
    //A lock is treated as stale if EITHER:
    //  (a) it was created by a process on THIS host whose pid is no longer alive
    //      (probed with kill(pid,0)); a dead holder is reclaimed immediately, or
    //  (b) it is older than LOCK_STALE_SEC. This is the backstop for locks held on
    //      other hosts (whose liveness we cannot check locally) and for the
    //      pid-recycling case where a dead holder's pid has been reused here.

    //(a) same-host liveness check: only reclaim on ESRCH ("no such process").
    //EPERM (process exists but owned by another user) is treated as alive.
    char host_name[256] = {'\0'};
    if(gethostname(host_name, 256) == 0)
    {
        if(strncmp(host_name, other->hostname, 256) == 0 && other->pid != 0)
        {
            if(kill(static_cast< pid_t >(other->pid), 0) != 0 && errno == ESRCH)
            {
                msg_warn("utilities", "lock holder pid " << other->pid << " on this host is gone; reclaiming lock: "
                                                          << std::string(other->lockfile_name) << eom);
                return LOCK_STALE_ERROR;
            }
        }
    }

    //(b) age-based backstop (handles other hosts and pid recycling)
    struct timeval timevalue;
    gettimeofday(&timevalue, NULL);
    unsigned long int epoch_sec = timevalue.tv_sec;

    if(other->time_sec < epoch_sec)
    {
        if((epoch_sec - other->time_sec) > LOCK_STALE_SEC)
        {
            msg_warn("utilities", "stale lock file detected (age > " << LOCK_STALE_SEC << "s): "
                                      << std::string(other->active_directory) + std::string(other->lockfile_name) << eom);
            return LOCK_STALE_ERROR;
        }
    }

    return LOCK_STATUS_OK;
}

int MHO_LockFileHandler::lock_has_priority(lockfile_data* ours, lockfile_data* other)
{
    //returns LOCK_PROCESS_HAS_PRIORITY if this process has priority
    //  over the other's lock
    //returns LOCK_PROCESS_NO_PRIORITY if this process does not have priority
    //returns LOCK_STALE_ERROR if there is a stale lock error

    //no need to check for stale-ness, this function is only called
    //if the lock is stolen

    //Priority is decided purely by creation time, and on an exact tie we YIELD
    //(return no-priority) rather than picking a winner by pid. This is what makes
    //the create-then-recheck scheme in at_front() correct:
    //
    //  - "other" is a lock we observed during our re-scan, so it necessarily
    //    existed before our scan, i.e. it was created no later than us.
    //  - If "other" was created strictly earlier, it is the rightful winner and
    //    we yield. Its lock existed before we created ours, so we are guaranteed
    //    to have seen it, the earlier process may not have seen us, but it is
    //    correctly the winner.
    //  - If our timestamps tie to the microsecond, at least one of the two tied
    //    processes must see the other (both cannot miss each other: a process can
    //    only miss a competitor that it created before, and that ordering cannot
    //    hold both ways). Whichever process sees the tie yields here, so at most
    //    one process ever proceeds. In the symmetric case both yield, remove their
    //    locks, and retry with fresh (hopefully distinct!) timestamps.
    //
    //Cross-host clock skew on NFS can still misorder timestamps, but not our problem!

    if(ours->time_sec != other->time_sec)
    {
        return (ours->time_sec < other->time_sec) ? LOCK_PROCESS_HAS_PRIORITY : LOCK_PROCESS_NO_PRIORITY;
    }
    if(ours->time_usec != other->time_usec)
    {
        return (ours->time_usec < other->time_usec) ? LOCK_PROCESS_HAS_PRIORITY : LOCK_PROCESS_NO_PRIORITY;
    }
    //exact timestamp tie: yield so that no two processes can both proceed
    return LOCK_PROCESS_NO_PRIORITY;
}

int MHO_LockFileHandler::create_lockfile(const char* directory, char* lockfile_name, lockfile_data* lock_data, int max_seq_no)
{
    //max_seq_no is the max file extent number seen at time of lock file creation
    //e.g. the '2' in GE.X.2.ABCDEF

    //get the host name, need to track this
    //in case we have multiple machines modifying the same NFS space
    char host_name[256] = {'\0'};
    int ret_val = gethostname(host_name, 256);
    if(ret_val != 0)
    {
        msg_error("utilities", "error retrieving host name in create_lockfile, using 'localhost' as substitute" << eom);
        snprintf(host_name, 10, "localhost");
    };

    //get the process id,
    pid_t this_pid = getpid();
    unsigned int pid = this_pid;

    //then get the epoch second
    struct timeval timevalue;
    gettimeofday(&timevalue, NULL);
    unsigned long int epoch_sec = timevalue.tv_sec;
    unsigned long int micro_sec = timevalue.tv_usec;
    unsigned int sequence_to_reserve = max_seq_no + 1;

    //dump the process id into a string to create the filename
    unsigned int i = 0;
    for(i = 0; i < MAX_LOCKNAME_LEN; i++)
    {
        lockfile_name[i] = '\0';
    }

    //copy in the scan directory and append the filename

    strcpy(lockfile_name, directory); //fDirectory.c_str());
    char* end_ptr = strrchr(lockfile_name, '/');
    end_ptr++;
    sprintf(end_ptr, "%u.%u.%lx.%lx.%s.lock", pid, sequence_to_reserve, epoch_sec, micro_sec, host_name);

    FILE* lockfile = fopen(lockfile_name, "w+");
    if(lockfile != NULL)
    {
        char time_buffer[100] = {'\0'};
        sprintf(time_buffer, "%lu\n", epoch_sec);
        fputs(time_buffer, lockfile);
        char temp_buffer[100] = {'\0'};
        sprintf(temp_buffer, "%d\n", pid);
        fputs(temp_buffer, lockfile);
        fclose(lockfile);

        //variables so that the signal handler can remove the lock
        //file if an interrupt is caught
        lock_data->validity = LOCK_VALID;
        lock_data->seq_number = sequence_to_reserve;
        lock_data->pid = this_pid;
        lock_data->time_sec = epoch_sec;
        lock_data->time_usec = micro_sec;
        strcpy(lock_data->hostname, host_name);
        strcpy(lock_data->active_directory, directory);
        strcpy(lock_data->lockfile_name, lockfile_name);
        msg_debug("utilities", "creating write lock file: " << std::string(lock_data->lockfile_name) << eom);
    }
    else
    {
        return LOCK_FILE_ERROR;
    }

    return LOCK_STATUS_OK;
}

//returns LOCK_PROCESS_HAS_PRIORITY if this process is at the front of the queue
//returns LOCK_PROCESS_NO_PRIORITY if otherwise
//returns and error code (various, see write_lock_mechanism.h) if an error
int MHO_LockFileHandler::at_front(const char* directory, char* lockfile_name, lockfile_data* lock_data, int cand_seq_no)
{

    //figure out root directory
    char root_dir[MAX_LOCKNAME_LEN] = {'\0'};
    strcpy(root_dir, directory);
    char* end_ptr_a = strrchr(root_dir, '/');
    end_ptr_a++;
    *end_ptr_a = '\0';
    int process_priority = LOCK_PROCESS_HAS_PRIORITY;
    char temp_lock_name[MAX_LOCKNAME_LEN] = {'\0'};
    lockfile_data temp_lock_struct;

    //scan the list of all files in the directory
    //for ones with the ".lock" extension
    DIR* d;
    struct dirent* dir;
    d = opendir(root_dir);
    if(d)
    {
        while((dir = readdir(d)) != NULL)
        {
            if(strstr(dir->d_name, ".lock") != NULL)
            {
                //found a lock file already in the directory
                strcpy(temp_lock_name, dir->d_name);
                init_lockfile_data(&temp_lock_struct);
                int error_code = parse_lockfile_name(temp_lock_name, &temp_lock_struct);
                strcpy(temp_lock_struct.active_directory, root_dir);
                if(error_code != LOCK_STATUS_OK)
                {
                    msg_error("utilities", "un-parsable lock file name: " << std::string(dir->d_name) << eom);
                    closedir(d);
                    return LOCK_PARSE_ERROR;
                }
                if(check_stale(&temp_lock_struct) != LOCK_STATUS_OK)
                {
                    //the holder appears dead: reclaim the directory by removing the
                    //stale lock. We do NOT yield on a stale lock, so if every
                    //lock found is stale we can proceed to create our own on
                    //this pass
                    std::string stale_path = std::string(root_dir) + dir->d_name;
                    remove(stale_path.c_str());
                    continue;
                }
                //a live lock is present, so this process cannot have priority
                process_priority = LOCK_PROCESS_NO_PRIORITY;
            }
        }
        closedir(d);
    }
    else
    {
        msg_error("utilities", "cannot access the directory: " << root_dir << eom);
        return LOCK_FILE_ERROR;
    }

    //don't have priority, bail out
    if(process_priority != LOCK_PROCESS_HAS_PRIORITY)
    {
        return process_priority;
    };

    //no other locks present, so go ahead and try to create a lock file
    int lock_retval = create_lockfile(directory, lockfile_name, lock_data, cand_seq_no);

    if(lock_retval == LOCK_STATUS_OK)
    {
        //created the lock file OK, but now we need to make sure
        //that no other process stole it out from under us

        //strip out the lockfile base name
        char lockfile_base[MAX_LOCKNAME_LEN] = {'\0'};
        char* end_ptr_b = strrchr(lockfile_name, '/');
        end_ptr_b++;
        strcpy(lockfile_base, end_ptr_b);

        //look for other lock files that may have snuck in
        d = opendir(root_dir);
        if(d)
        {
            while((dir = readdir(d)) != NULL)
            {
                if(strstr(dir->d_name, ".lock") != NULL)
                {
                    strcpy(temp_lock_name, dir->d_name);
                    if(strcmp(lockfile_base, temp_lock_name) != 0) //not our own lock file
                    {
                        init_lockfile_data(&temp_lock_struct);
                        int error_code = parse_lockfile_name(temp_lock_name, &temp_lock_struct);
                        if(error_code != 0)
                        {
                            //msg ("Error: un-parsable lock file name: %s ", 3, dir->d_name);
                            return LOCK_PARSE_ERROR;
                        }
                        process_priority = lock_has_priority(lock_data, &temp_lock_struct);
                        if(process_priority != LOCK_PROCESS_HAS_PRIORITY)
                        {
                            //either we don't have write priority or an error occured
                            break;
                        }
                    }
                }
            }
            closedir(d);
        }
        else
        {
            //something went wrong reading the directory, clean up
            remove_lockfile(lock_data);
            lockfile_name[0] = '\0';
            //msg ("Error: can't access directory: %s ", 3, root_dir);
            return LOCK_FILE_ERROR;
        }

        if(process_priority != LOCK_PROCESS_HAS_PRIORITY)
        {
            //some other process created a lock file at almost the same time
            //and we don't have priority, so defer to the other process, delete our lock
            //and return 0 (we don't have priority)
            remove_lockfile(lock_data);
            lockfile_name[0] = '\0';
            return process_priority;
        }

        //made it here so this process has write priority
        return process_priority;
    }
    else
    {
        //msg ("Error: could not create write lock on directory: %s ", 3, root_dir);
        return LOCK_FILE_ERROR;
    }
}

int MHO_LockFileHandler::wait_for_write_lock(int& next_seq_no)
{
    next_seq_no = -1;
    char lockfile_name[MAX_LOCKNAME_LEN] = {'\0'};
    //wait until this process is at the front of the write queue,
    //then return with the next sequence number
    int is_at_front = 0;
    int n_checks = 0;
    int max_seq_no = 0;
    do
    {
        max_seq_no = get_max_seq_number(fDirectory);
        if(n_checks == 0)
        {
            msg_debug("utilities", "detected max sequence number of: " << max_seq_no << ", in: " << fDirectory << eom);
        }
        //provisionally fset->maxfile is the largest fringe number on disk
        //but we need to check that WE are allowed to take the successor:
        is_at_front = at_front(fDirectory.c_str(), lockfile_name, &fProcessLockFileData, max_seq_no + 1);
        n_checks++;
        if(is_at_front == LOCK_PROCESS_NO_PRIORITY)
        {
            //randomized backoff: jitter the sleep so that two processes which tied
            //on an identical lock timestamp (and therefore both yielded in
            //lock_has_priority) do not retry in lock-step and collide again.
            static std::mt19937 rng(static_cast< unsigned >(getpid()) ^ static_cast< unsigned >(time(nullptr))); //seed pid-time
            static std::uniform_int_distribution< int > jitter(0, 50000); // microseconds
            usleep(100000 + jitter(rng));
        }
    }
    while(is_at_front == LOCK_PROCESS_NO_PRIORITY && n_checks < LOCK_TIMEOUT);

    //couldn't get a write lock because of time out
    if(n_checks >= LOCK_TIMEOUT)
    {
        msg_error("utilities", "lock file time-out error associated with dir: " << fDirectory << eom);
        return LOCK_TIMEOUT_ERROR;
    }

    if(is_at_front != LOCK_PROCESS_HAS_PRIORITY)
    {
        //some other error has occurred
        return is_at_front;
    }

    max_seq_no = get_max_seq_number(fDirectory);
    next_seq_no = max_seq_no + 1;

    msg_debug("utilities", "acquired write lock for sequence number: " << next_seq_no << eom);

    return LOCK_STATUS_OK;
}

int MHO_LockFileHandler::get_max_seq_number(std::string dir)
{

    int max_seq_no = 0;
    std::vector< std::string > files;
    std::vector< std::string > fringe_files;
    std::vector< std::string > tokens;
    ///check for max sequence number on disk
    //point the directory interface to where we plan to write the file
    //and check for the max sequency number seen in fringe files
    fDirInterface.SetCurrentDirectory(dir);
    fDirInterface.ReadCurrentDirectory();
    if(fEnableLegacyMode)
    {
        //look for old-style finge files (legacy mode)
        max_seq_no = 0;
        fDirInterface.GetFileList(files);
        fDirInterface.GetFringeFiles(files, fringe_files, max_seq_no);
    }
    else
    {
        //look for new style fringe files
        max_seq_no = 0;
        fDirInterface.GetFilesMatchingExtention(fringe_files, "frng");
        for(auto it = fringe_files.begin(); it != fringe_files.end(); it++)
        {
            std::string basename = MHO_DirectoryInterface::GetBasename(*it);
            //format looks like "GE.Gs-Wf.X.XY.0VSI1M.1.frng"
            tokens.clear();
            fTokenizer.SetString(&basename);
            fTokenizer.GetTokens(&tokens);
            if(tokens.size() >= 7)
            {
                std::string seq_tok = tokens[tokens.size() - 2];
                int seq_no = std::atoi(seq_tok.c_str()); //2nd to last token is extent no.
                if(seq_no > max_seq_no)
                {
                    max_seq_no = seq_no;
                }
            }
        }
    }
    return max_seq_no;
}

} // namespace hops
