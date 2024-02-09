#include "MHO_LockFileHandler.hh"

#include <sys/types.h>
#include <sys/time.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <stdlib.h>

//wait time allowed before a lock file is declared stale (5 min)
#define LOCK_STALE_SEC 300

//number of lock attempts before time-out error
//this is roughly 15 minutes...probably much longer than needed
#define LOCK_TIMEOUT 9000

//struct validity
#define LOCK_VALID 0
#define LOCK_INVALID -1


namespace hops
{



//initialization to nullptr
MHO_LockFileHandler* MHO_LockFileHandler::fInstance = nullptr;

//function to handle signals (to ensure we clean up lockfiles if we get interrupted)
void 
MHO_LockFileHandler::HandleSignal(int signal_value) 
{
    MHO_LockFileHandler::GetInstance().RemoveWriteLock();
    signal(signal_value, SIG_DFL); //reset the handler for this particular signal to default
    kill(getpid(), signal_value); //re-send the signal to this process
}

//set the write directory
void 
MHO_LockFileHandler::SetDirectory(std::string dir)
{
    fDirectory = dir;
    //make sure our directory is terminated with a "/"
    //this needs to be the case for the dir/file parsing code
    if(fDirectory.size() !=0)
    {
        if( fDirectory[fDirectory.size()-1] != '/'){fDirectory += "/";}
    }
}

void 
MHO_LockFileHandler::RemoveWriteLock()
{
    remove_lockfile(&fProcessLockFileData);
}

int 
MHO_LockFileHandler::WaitForWriteLock(std::string directory, int& next_seq_no)
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
    for(i=0; i<256; i++){data->hostname[i] = '\0';}
    for(i=0; i<MAX_LOCKNAME_LEN; i++){data->active_directory[i] = '\0';}
    for(i=0; i<MAX_LOCKNAME_LEN; i++){data->lockfile_name[i] = '\0';}
}

void MHO_LockFileHandler::remove_lockfile(lockfile_data* data)
{
    if(data->validity == LOCK_VALID)
    {
        msg_debug("utilities", "removing write lock file: "<< std::string(data->lockfile_name) << eom);
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

    if(ptr != NULL){sscanf(ptr, "%u", &(result->pid));}
    else{return LOCK_PARSE_ERROR;}

    ptr = strtok(NULL, ".");

    if(ptr != NULL){sscanf(ptr, "%u", &(result->seq_number));}
    else{return LOCK_PARSE_ERROR;}

    ptr = strtok(NULL, ".");

    if(ptr != NULL){sscanf(ptr, "%lx", &(result->time_sec));}
    else{return LOCK_PARSE_ERROR;}

    ptr = strtok(NULL, ".");

    if(ptr != NULL){sscanf(ptr, "%lx", &(result->time_usec));}
    else{return LOCK_PARSE_ERROR;}

    ptr = strtok(NULL, ".");

    if(ptr != NULL){sscanf(ptr, "%s", &(result->hostname[0]));}
    else{return LOCK_PARSE_ERROR;}

    result->validity = LOCK_VALID;
    return 0;
}

int MHO_LockFileHandler::check_stale(lockfile_data* other)
{
    //returns LOCK_STATUS_OK if the other lock is stale 
    //returns LOCK_STALE_ERROR if the lock is not stale

    //check for stale-ness irrespective of priority or host
    struct timeval timevalue;
    gettimeofday(&timevalue, NULL);
    unsigned long int epoch_sec = timevalue.tv_sec;
    unsigned long int micro_sec = timevalue.tv_usec;

    if( other->time_sec < epoch_sec )
    {
        if( (epoch_sec - other->time_sec) > LOCK_STALE_SEC )
        {
            //issue warning, we do not have priority
            msg_warn("utilities", "stale lock file detected: "<<
                std::string(other->active_directory) + std::string(other->lockfile_name) << eom);
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

    //strict temporal ordering between the processes is not totally
    //necessary, however, we do need a robust mechanism for deciding
    //the ordering between processes, for this reason we use the pid
    //in the rare circumstances where the pid is the same, because
    //of different hosts or pid recycling, we defer to using time ordering
    //if that in turn fails, then the lock with be deleted and we try again

    if(ours->pid < other->pid)
    {
        return LOCK_PROCESS_HAS_PRIORITY;
    }
    else if( ours->pid == other->pid)    // tie-break w/time
    {
        if( ours->time_sec < other->time_sec )
        {
            return LOCK_PROCESS_HAS_PRIORITY;
        }
        else if ( ours->time_sec == other->time_sec)
        {
            if( ours->time_usec < other->time_usec)
            {
                return LOCK_PROCESS_HAS_PRIORITY;
            }
            else
            {
                return LOCK_PROCESS_NO_PRIORITY;
            }
        }
        else    // ours is > other->pid
        {
            return LOCK_PROCESS_NO_PRIORITY;
        }
    }
    else
    {
        return LOCK_PROCESS_NO_PRIORITY;
    }

}

int MHO_LockFileHandler::create_lockfile(const char* directory, char* lockfile_name, 
                                         lockfile_data* lock_data, int max_seq_no)
{
    //max_seq_no is the max file extent number seen at time of lock file creation 
    //e.g. the '2' in GE.X.2.ABCDEF
    
    //get the host name, need to track this 
    //in case we have multiple machines modifying the same NFS space
    char host_name[256] = {'\0'};
    int ret_val = gethostname(host_name, 256);
    if( ret_val != 0)
    {
        msg_fatal("utilities", "error retrieving host name in create_lockfile." << eom);
        std::exit(1);
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
    for(i=0; i<MAX_LOCKNAME_LEN; i++){lockfile_name[i] = '\0';}

    //copy in the scan directory and append the filename

    strcpy(lockfile_name, directory);//fDirectory.c_str());
    char* end_ptr = strrchr(lockfile_name, '/');
    end_ptr++;
    sprintf(end_ptr, "%u.%u.%lx.%lx.%s.lock",
    pid, sequence_to_reserve, epoch_sec, micro_sec, host_name);

    FILE* lockfile = fopen(lockfile_name, "w+" );
    if (lockfile!=NULL)
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
        msg_debug("utilities", "creating write lock file: "<< std::string(lock_data->lockfile_name) << eom);
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
int MHO_LockFileHandler::at_front(const char* directory, char* lockfile_name,
                                  lockfile_data* lock_data, int cand_seq_no)
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
        while( (dir = readdir(d)) != NULL)
        {
            if(strstr(dir->d_name, ".lock") != NULL)
            {
                //found a lock file already in the directory
                //so this process cannot have priority
                process_priority = LOCK_PROCESS_NO_PRIORITY;
                //check if the other file is stale (this is an error)
                strcpy(temp_lock_name, dir->d_name);
                init_lockfile_data(&temp_lock_struct);
                int error_code = parse_lockfile_name(temp_lock_name, &temp_lock_struct);
                strcpy(temp_lock_struct.active_directory, root_dir); 
                if(error_code != LOCK_STATUS_OK)
                {
                    msg_error("utilities", "un-parsable lock file name: "<< std::string(dir->d_name) << eom);
                    return LOCK_PARSE_ERROR;
                }
                int stale_lock = check_stale(&temp_lock_struct);
                if(stale_lock != LOCK_STATUS_OK)
                {
                    closedir(d);
                    return LOCK_STALE_ERROR;
                }
            }
        }
        closedir(d);
    }
    else
    {
        msg_error("utilities", "cannot access the directory: "<< root_dir << eom );
        return LOCK_FILE_ERROR;
    }

    //don't have priority, bail out
    if(process_priority != LOCK_PROCESS_HAS_PRIORITY){return process_priority;};
    
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
            while( (dir = readdir(d)) != NULL)
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
    std::vector< std::string > files;
    std::vector< std::string > fringe_files;
    do
    {
        ///check for max sequence number on disk
        //point the directory interface to where we plan to write the file
        //and check for the max sequency number seen in fringe files
        fDirInterface.SetCurrentDirectory(fDirectory);
        fDirInterface.ReadCurrentDirectory();
        fDirInterface.GetFileList(files);
        fDirInterface.GetFringeFiles(files, fringe_files, max_seq_no);
        if(n_checks == 0)
        {
            msg_debug("utilities", "detected max sequence number of: "<< max_seq_no << ", in: " << fDirectory << eom);
        }
        //provisionally fset->maxfile is the largest fringe number on disk
        //but we need to check that WE are allowed to take the successor:
        is_at_front = at_front(fDirectory.c_str(), lockfile_name, &fProcessLockFileData, max_seq_no+1);
        n_checks++;
        if(is_at_front == LOCK_PROCESS_NO_PRIORITY){usleep(100000);}
    }
    while( is_at_front == LOCK_PROCESS_NO_PRIORITY && n_checks < LOCK_TIMEOUT );
    
    //couldn't get a write lock because of time out
    if(n_checks >= LOCK_TIMEOUT)
    {
        msg_error("utilities", "lock file time-out error associated with dir: "<< fDirectory<< eom);
        return LOCK_TIMEOUT_ERROR;
    }
    
    if(is_at_front != LOCK_PROCESS_HAS_PRIORITY)
    {
        //some other error has occurred
        return is_at_front;
    }
    
    //made it here, so we have write priority now, just need to
    //check/update the extent number for type-2 files and return it
    fDirInterface.SetCurrentDirectory(fDirectory);
    fDirInterface.ReadCurrentDirectory();
    fDirInterface.GetFileList(files);
    fDirInterface.GetFringeFiles(files, fringe_files, max_seq_no);
    next_seq_no = max_seq_no+1;
    msg_debug("utilities", "acquired write lock for sequence number: "<< next_seq_no << eom);

    return LOCK_STATUS_OK;
}




}//end namesace
