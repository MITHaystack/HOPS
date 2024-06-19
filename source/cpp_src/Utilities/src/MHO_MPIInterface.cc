#include "MHO_MPIInterface.hh"

#ifdef LOCAL_RANK_MPI

#include <stdint.h>
#include <limits.h>
#include <sstream>
//hostname utils, should be available on on POSIX systems
#include <unistd.h>
#endif

#define MSG_TAG                   999
#define HOST_DETERMINATION_TAG    998
#define LOCALID_DETERMINATION_TAG 997

//deal with the discrepancy in size between size_t and unsigned int in MPI comms
#if SIZE_MAX == UCHAR_MAX
   #define HOPS_MPI_SIZE_T MPI_UNSIGNED_CHAR
#elif SIZE_MAX == USHRT_MAX
   #define HOPS_MPI_SIZE_T MPI_UNSIGNED_SHORT
#elif SIZE_MAX == UINT_MAX
   #define HOPS_MPI_SIZE_T MPI_UNSIGNED
#elif SIZE_MAX == ULONG_MAX
   #define HOPS_MPI_SIZE_T MPI_UNSIGNED_LONG
#elif SIZE_MAX == ULLONG_MAX
   #define HOPS_MPI_SIZE_T MPI_UNSIGNED_LONG_LONG
#else
   #error "Cannot determine size of size_t for MPI Comms."
#endif



namespace hops
{

MHO_MPIInterface* MHO_MPIInterface::fMPIInterface = nullptr;

MHO_MPIInterface::MHO_MPIInterface()
{
    fGlobalProcessID = -1;
    fNProcesses = -1;
    fLocalProcessID = -1;
    fSubGroupRank = -1;
    fNSubGroupProcesses = -1;
    fPartnerProcessID = -1;
    fSplitMode = false;
}

MHO_MPIInterface::~MHO_MPIInterface() = default;

void MHO_MPIInterface::Initialize(int* argc, char*** argv, bool split_mode)
{
    //init
    int initialized = 0;
    MPI_Initialized(&initialized);
    if(!initialized){ MPI_Init(argc, argv); }

    //global rank/ID of this process
    MPI_Comm_rank(MPI_COMM_WORLD, &fGlobalProcessID);

    /* Find out how many processes are being used */
    MPI_Comm_size(MPI_COMM_WORLD, &fNProcesses);

    if (fGlobalProcessID <= 0)
    {
        if (fNProcesses <= 0)
        {
            msg_warn("mpi_interface", "no MPI processes found, not running in an MPI context." << eom);
        }
        else if (!initialized)
        {
            msg_info("mpi_interface", "Running MPI, using " << fNProcesses << " processes." << eom );
        }
    }

    //now determine the local rank of this process (indexed from zero) on the local host
    //for example, if processes (0,2,5) are running on host A
    //and processes (1,3,4) are running on host B, then the
    //local rank of process 3 is 1, and the local rank of process 5 is 2
    //this is useful for when cooperative tasks need to be completed on a local host
    DetermineLocalRank();
    fSplitMode = split_mode;

    //construct groups/communicators to evenly split up processes
    SetupSubGroups();

    //cannot make a valid even/odd split, so revert to standard mode
    if (!fValidSplit) { fSplitMode = false;};
}

void MHO_MPIInterface::Finalize()
{
    //shut down MPI
    int finalized = 0;
    MPI_Finalized(&finalized);
    if (!finalized){ MPI_Finalize(); }
}


MHO_MPIInterface* MHO_MPIInterface::GetInstance()
{
    //singleton interface construction
    if (fMPIInterface == nullptr){ fMPIInterface = new MHO_MPIInterface();}
    return fMPIInterface;
}


void MHO_MPIInterface::BeginSequentialProcess()
{
    int flag = 1;
    GlobalBarrier();
    if (fGlobalProcessID > 0)
    {
        MPI_Recv(&flag, 1, MPI_INT, fGlobalProcessID - 1, 50, MPI_COMM_WORLD, &fStatus);
    }
}

void MHO_MPIInterface::EndSequentialProcess()
{
    int flag;
    if (fGlobalProcessID < (fNProcesses - 1))
    {
        MPI_Send(&flag, 1, MPI_INT, fGlobalProcessID + 1, 50, MPI_COMM_WORLD);
    }
    GlobalBarrier();
}


void MHO_MPIInterface::PrintMessage(std::string msg)
{
    if (fNProcesses < 0 || fGlobalProcessID < 0)
    {
        msg_error("mpi_interface", "MPI failed to initialize." << eom );
        msg_error("mpi_interface", "Message was: " << msg << eom );
        return;
    }

    std::size_t n_char = msg.size();

    std::vector<std::size_t> in_msg_sizes;
    std::vector<std::size_t> out_msg_sizes;
    in_msg_sizes.resize(fNProcesses, 0);
    out_msg_sizes.resize(fNProcesses, 0);
    in_msg_sizes[fGlobalProcessID] = n_char;

    //obtain the message sizes from all of the objects
    MPI_Allreduce(&(in_msg_sizes[0]), &(out_msg_sizes[0]), fNProcesses, HOPS_MPI_SIZE_T, MPI_SUM, MPI_COMM_WORLD);

    //compute the total message size
    std::size_t total_msg_size = 0;
    std::vector<std::size_t> msg_start_indexes;
    msg_start_indexes.resize(fNProcesses);
    for (int i = 0; i < fNProcesses; i++)
    {
        total_msg_size += out_msg_sizes[i];
    };

    for (int i = 0; i < fNProcesses; i++)
    {
        for (int j = 0; j < i; j++)
        {
            msg_start_indexes[i] += out_msg_sizes[j];
        }
    };

    //allocate buffers to reduce all of the messages
    std::vector<char> buf;
    buf.resize(total_msg_size);

    //fill the appropriate section of the buffer
    for (std::size_t i = 0; i < msg.size(); i++)
    {
        buf[msg_start_indexes[fGlobalProcessID] + i] = msg.at(i);
    }

    MPI_Status status;
    if (fGlobalProcessID == 0)
    {
        for (int i = 1; i < fNProcesses; i++)
        {
            MPI_Recv(&(buf[msg_start_indexes[i]]), out_msg_sizes[i], MPI_CHAR, i, MSG_TAG, MPI_COMM_WORLD, &status);
        }
    }
    else
    {
        MPI_Send(&(buf[msg_start_indexes[fGlobalProcessID]]), out_msg_sizes[fGlobalProcessID], MPI_CHAR, 0, MSG_TAG, MPI_COMM_WORLD);
    }

    if (fGlobalProcessID == 0)
    {
        //convert to string
        std::stringstream final_output;
        for (char c : buf)
        {
            final_output << c;
        }
        std::string full_message = final_output.str();

        //split the messages into separate lines so we can correctly use
        std::vector<std::string> lines;
        std::string::size_type pos = 0;
        std::string::size_type prev = 0;
        while ((pos = full_message.find('\n', prev)) != std::string::npos)
        {
            lines.push_back(full_message.substr(prev, pos - prev));
            prev = pos + 1;
        }

        if (lines.size() == 0)
        {
            //message has no newline characters, so push the whole message into one line
            lines.push_back(full_message);
        }

        //print message one line at a time
        for(std::size_t i=0; i<lines.size(); i++)
        {
            msg_info("mpi_interface", lines[i] << eom);
        }
    }
}


void
MHO_MPIInterface::BroadcastString(std::string& msg)
{
    //if we are the root/master process, figure out the size of the message
    std::size_t msg_size = msg.size();

    //first broadcast size of the message to the other processes
    MPI_Bcast(&msg_size, msg_size, MPI_UNSIGNED, 0, MPI_COMM_WORLD);

    //create some buffer space
    std::string buf;
    buf.resize(msg_size);
    if(fGlobalProcessID == 0){buf = msg;}

    //now every process should have the root/master's msg in the buffer, return
    msg = buf;
}


void MHO_MPIInterface::DetermineLocalRank()
{
    #ifdef LOCAL_RANK_MPI
        //get the machine's hostname
        char host_name[256];
        int ret_val = gethostname(host_name, 256);
        if (ret_val != 0)
        {
            msg_fatal("mpi_interface", "host name retrieval error!" << eom);
            MHO_MPIInterface::GetInstance()->Finalize();
            std::exit(1);
        };

        std::stringstream hostname_ss;
        int count = 0;
        do
        {
            hostname_ss << host_name[count];
            count++;
        }
        while (host_name[count] != '\0' && count < 256);
        std::string hostname = hostname_ss.str();
        fHostName = hostname;

        //first we have to collect all of the hostnames that are running a process
        std::size_t n_char = hostname.size();
        std::vector<std::size_t> in_msg_sizes;
        std::vector<std::size_t> out_msg_sizes;
        in_msg_sizes.resize(fNProcesses, 0);
        out_msg_sizes.resize(fNProcesses, 0);
        in_msg_sizes[fGlobalProcessID] = n_char;

        //obtain the message sizes from all of the processes
        MPI_Allreduce(&(in_msg_sizes[0]), &(out_msg_sizes[0]), fNProcesses, HOPS_MPI_SIZE_T, MPI_SUM, MPI_COMM_WORLD);

        //compute the total message size
        std::size_t total_msg_size = 0;
        std::vector<std::size_t> msg_start_indexes;
        msg_start_indexes.resize(fNProcesses,0);
        for (int i = 0; i < fNProcesses; i++)
        {
            total_msg_size += out_msg_sizes[i];
        };

        for (int i = 0; i < fNProcesses; i++)
        {
            for (int j = 0; j < i; j++)
            {
                msg_start_indexes[i] += out_msg_sizes[j];
            }
        };

        //allocate buffers to reduce all of the messages
        std::vector<char> buf;
        buf.resize(total_msg_size);

        //fill the appropriate section of the buffer
        for (std::size_t i = 0; i < hostname.size(); i++)
        {
            buf[msg_start_indexes[fGlobalProcessID] + i] = hostname.at(i);
        }

        //reduce the buffer across all processes
        MPI_Status status;
        if (fGlobalProcessID == 0)
        {
            for (int i = 1; i < fNProcesses; i++)
            {
                MPI_Recv(&(buf[msg_start_indexes[i]]),
                         out_msg_sizes[i],
                         MPI_CHAR,
                         i,
                         HOST_DETERMINATION_TAG,
                         MPI_COMM_WORLD,
                         &status);
            }
        }
        else
        {
            std::size_t root_rank = 0;
            MPI_Send(&(buf[msg_start_indexes[fGlobalProcessID]]),
                     out_msg_sizes[fGlobalProcessID],
                     MPI_CHAR,
                     root_rank,
                     HOST_DETERMINATION_TAG,
                     MPI_COMM_WORLD);
        }

        //now broadcast the complete list of hostnames to all processes
        MPI_Bcast(&(buf[0]), total_msg_size, MPI_CHAR, 0, MPI_COMM_WORLD);

        //now every node has a list of all host names
        //we now need to figure out how many other processes are also running on
        //the same host, and how many devices are available on this host
        //then we can distribute the processes equitably to each device

        std::vector<std::string> hostname_list;
        hostname_list.resize(fNProcesses);
        for(int i = 0; i < fNProcesses; i++)
        {
            hostname_list[i] = std::string("");
            for (std::size_t j = 0; j < out_msg_sizes[i]; j++)
            {
                hostname_list[i].push_back(buf[msg_start_indexes[i] + j]);
            }
        }

        //collect all the process ids of all the process running on this host
        fCoHostedProcessIDs.clear();
        for(int i = 0; i < fNProcesses; i++)
        {
            if (hostname == hostname_list[i])
            {
                fCoHostedProcessIDs.push_back(i);
            }
        }

        //determine the 'local' rank of this process
        for(std::size_t i = 0; i < fCoHostedProcessIDs.size(); i++)
        {
            if (fCoHostedProcessIDs[i] == fGlobalProcessID)
            {
                fLocalProcessID = i;
            };
        }

    #endif
}


void MHO_MPIInterface::SetupSubGroups()
{
    #ifdef LOCAL_RANK_MPI
        //we need to retrieve the local rank from each process
        //to make a associative map betweek global-rank and local-rank
        std::vector<int> local_ranks;
        local_ranks.resize(fNProcesses);
        local_ranks[fGlobalProcessID] = fLocalProcessID;

        //reduce the buffer across all processes
        MPI_Status status;
        if (fGlobalProcessID == 0)
        {
            for (int i = 1; i < fNProcesses; i++)
            {
                MPI_Recv(&(local_ranks[i]), 1, MPI_INT, i, LOCALID_DETERMINATION_TAG, MPI_COMM_WORLD, &status);
            }
        }
        else
        {
            std::size_t root_rank = 0;
            MPI_Send(&(local_ranks[fGlobalProcessID]), 1, MPI_INT, root_rank, LOCALID_DETERMINATION_TAG, MPI_COMM_WORLD);
        }

        //now broadcast the complete list of hostnames to all processes
        MPI_Bcast(&(local_ranks[0]), fNProcesses, MPI_INT, 0, MPI_COMM_WORLD);

        //now every process has a list of the local rank associated with every other process
        //now we can proceed to determine which group they below to
        std::vector<int> even_members;
        std::vector<int> odd_members;
        for (int i = 0; i < fNProcesses; i++)
        {
            if (local_ranks[i] % 2 == 0){ even_members.push_back(i); }
            else { odd_members.push_back(i); }
        }

        fValidSplit = false;
        if (even_members.size() == odd_members.size()){fValidSplit = true; }

        //get the world group
        MPI_Group world;
        MPI_Comm_group(MPI_COMM_WORLD, &world);

        //now we go ahead and construct the groups and communicators
        MPI_Group_incl(world, even_members.size(), &(even_members[0]), &fEvenGroup);
        MPI_Group_incl(world, odd_members.size(), &(odd_members[0]), &fOddGroup);

        MPI_Comm_create(MPI_COMM_WORLD, fEvenGroup, &fEvenCommunicator);
        MPI_Comm_create(MPI_COMM_WORLD, fOddGroup, &fOddCommunicator);

        //now we set things up for this process
        if (fLocalProcessID % 2 == 0)
        {
            fIsEvenGroupMember = true;
            MPI_Comm_rank(fEvenCommunicator, &fSubGroupRank);
            fNSubGroupProcesses = even_members.size();
        }
        else
        {
            fIsEvenGroupMember = false;
            MPI_Comm_rank(fOddCommunicator, &fSubGroupRank);
            fNSubGroupProcesses = odd_members.size();
        }

        //finally, if we have a valid split (equal numbers of even and odd process)
        //we can pair up processes so they can exchange data
        if (fValidSplit)
        {
            int status = 0;
            int result = 0;
            if (fCoHostedProcessIDs.size() % 2 == 0){ status = 1; };
            MPI_Allreduce(&status, &result, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);

            //to make things faster we first try to pair up processes
            //which share the same node/host
            if (result == fNProcesses)
            {
                //we can because each host has an even number of processes
                if (fIsEvenGroupMember)
                {
                    fPartnerProcessID = fCoHostedProcessIDs[fLocalProcessID + 1];
                }
                else
                {
                    fPartnerProcessID = fCoHostedProcessIDs[fLocalProcessID - 1];
                }
            }
            else
            {
                //this isn't possible so we have to pair up processes across nodes
                if (fIsEvenGroupMember)
                {
                    fPartnerProcessID = fGlobalProcessID + 1;
                }
                else
                {
                    fPartnerProcessID = fGlobalProcessID - 1;
                }
            }
        }
#endif
}

}  // namespace hops
