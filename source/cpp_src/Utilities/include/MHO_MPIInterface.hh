#ifndef MHO_MPIInterface_HH__
#define MHO_MPIInterface_HH__

#include "mpi.h"

#include "MHO_Message.hh"
#include <string>
#include <vector>

#define LOCAL_RANK_MPI

namespace hops
{

/*!
 *@file MHO_MPIInterface.hh
 *@class MHO_MPIInterface
 *@date Sat Mar 11 18:35:02 2023 -0500
 *@brief interface functions for initialization of a MPI environment
 *@author J. Barrett - barrettj@mit.edu
 */

class MHO_MPIInterface
{
    public:
        //singleton interface
        /**
         * @brief Getter for instance
         * 
         * @return MHO_MPIInterface* singleton instance
         * @note This is a static function.
         */
        static MHO_MPIInterface* GetInstance();

        /**
         * @brief Initializes MPI environment and sets up process groups/communicators.
         * 
         * @param argc Pointer to integer for command line argument count
         * @param argv Double pointer to character array for command line arguments
         * @param split_mode Boolean flag indicating whether to split processes
         */
        void Initialize(int* argc, char*** argv, bool split_mode = true);
        /**
         * @brief Finalizes MPI by calling MPI_Finalize if not already finalized.
         */
        void Finalize();

        /**
         * @brief Checks if global process ID is non-negative and number of processes is greater than zero.
         * 
         * @return True if conditions are met, false otherwise.
         */
        bool Check() const { return (fGlobalProcessID >= 0) && (fNProcesses > 0); }

        /**
         * @brief Getter for global process id
         * 
         * @return The global process ID as an integer.
         */
        int GetGlobalProcessID() const { return fGlobalProcessID; }

        /**
         * @brief Getter for N processes
         * 
         * @return The number of processes as an integer.
         */
        int GetNProcesses() const { return fNProcesses; }

        /**
         * @brief Getter for local process id
         * 
         * @return Local process ID as an integer.
         */
        int GetLocalProcessID() const { return fLocalProcessID; }

        /**
         * @brief Getter for host name
         * 
         * @return Host name as a string
         */
        std::string GetHostName() const { return fHostName; };

        //use to isolate a section of code, so each process completes it
        //one at a time
        /**
         * @brief Isolates a section of code for sequential processing by each process one at a time.
         */
        void BeginSequentialProcess();
        /**
         * @brief Sends a flag to the next process and waits for all processes to finish.
         */
        void EndSequentialProcess();

        /**
         * @brief Waits for all processes in MPI_COMM_WORLD to reach this barrier.
         */
        void GlobalBarrier() const { MPI_Barrier(MPI_COMM_WORLD); }


        /**
         * @brief Collects and prints messages from all processes in a MPI parallel environment.
         * when called, this function must be encountered by all processes
         * or the program will lock up, treat it as a global barrier
         * use it to safely print messages from each process without clobbering the ouput
         * 
         * @param msg Message to be printed by all processes.
         */
        void PrintMessage(std::string msg);

        //broadcast a string message to all processes
        /**
         * @brief Broadcasts a string message to all processes from root/master process.
         * 
         * @param msg Reference to string message to be broadcasted.
         */
        void BroadcastString(std::string& msg);

        //routines to be used by programs which split the processes into two
        //groups bases on even/odd local process rank
        /**
         * @brief Checks if processes are split into two groups based on even/odd ranks.
         * 
         * @return True if in split mode, false otherwise.
         */
        bool SplitMode() { return fSplitMode; };

        /**
         * @brief Checks if even/odd split is valid.
         * 
         * @return True if split is valid, false otherwise.
         */
        bool IsSplitValid() { return fValidSplit; };

        /**
         * @brief Checks if the current process is a member of the even subgroup.
         * 
         * @return True if it's an even group member, false otherwise.
         */
        bool IsEvenGroupMember() { return fIsEvenGroupMember; };

        /**
         * @brief Getter for nsub group processes
         * 
         * @return Number of subgroup processes.
         */
        int GetNSubGroupProcesses() { return fNSubGroupProcesses; }

        /**
         * @brief Getter for sub group rank
         * 
         * @return The subgroup rank as an integer.
         */
        int GetSubGroupRank() { return fSubGroupRank; };

        /**
         * @brief Getter for partner process id
         * 
         * @return ID of the partner process as an integer.
         */
        int GetPartnerProcessID() { return fPartnerProcessID; };

        /**
         * @brief Getter for sub group
         * 
         * @return MPI_Group* representing the sub-group rank
         */
        MPI_Group* GetSubGroup()
        {
            if(fIsEvenGroupMember)
            {
                return &fEvenGroup;
            }
            else
            {
                return &fOddGroup;
            };
        }

        MPI_Comm* GetSubGroupCommunicator()
        {
            if(fIsEvenGroupMember)
            {
                return &fEvenCommunicator;
            }
            else
            {
                return &fOddCommunicator;
            };
        }

        MPI_Group* GetEvenGroup() { return &fEvenGroup; };

        MPI_Group* GetOddGroup() { return &fOddGroup; };

        MPI_Comm* GetEvenCommunicator() { return &fEvenCommunicator; };

        MPI_Comm* GetOddCommunicator() { return &fOddCommunicator; };

    protected:
        MHO_MPIInterface();
        virtual ~MHO_MPIInterface();

        static MHO_MPIInterface* fMPIInterface;

        int fGlobalProcessID;
        int fNProcesses;
        int fLocalProcessID;
        std::string fHostName;
        std::vector< int > fCoHostedProcessIDs;

        //groups and communicators for splitting processes into
        //two sets, based on whether they have even/odd (local) ranks
        bool fSplitMode;
        MPI_Group fEvenGroup;       //even process subgroup
        MPI_Group fOddGroup;        //odd process subgroup
        MPI_Comm fEvenCommunicator; //comm for even group
        MPI_Comm fOddCommunicator;  //comm for odd group
        bool fValidSplit;           //true if the size of the subgroups is equal
        bool fIsEvenGroupMember;    //true if this process is a member of the even subgroup
        int fSubGroupRank;          //rank of this process in its subgroup
        int fNSubGroupProcesses;    //number of processes in the subgroup this process belongs to
        int fPartnerProcessID;      //global rank of partner process in other subgroup

        void DetermineLocalRank();
        void SetupSubGroups();

        MPI_Status fStatus;
};

} //end of namespace hops

#endif /*! MHO_MPIInterface_HH__ */
