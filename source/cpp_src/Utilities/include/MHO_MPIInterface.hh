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
 *@brief
 *@author J. Barrett - barrettj@mit.edu
 */

class MHO_MPIInterface
{
    public:
        //singleton interface
        static MHO_MPIInterface* GetInstance();

        void Initialize(int* argc, char*** argv, bool split_mode = true);
        void Finalize();

        bool Check() const { return (fGlobalProcessID >= 0) && (fNProcesses > 0); }

        int GetGlobalProcessID() const { return fGlobalProcessID; }

        int GetNProcesses() const { return fNProcesses; }

        int GetLocalProcessID() const { return fLocalProcessID; }

        std::string GetHostName() const { return fHostName; };

        //use to isolate a section of code, so each process completes it
        //one at a time
        void BeginSequentialProcess();
        void EndSequentialProcess();

        void GlobalBarrier() const { MPI_Barrier(MPI_COMM_WORLD); }

        //when called, this function must be encountered by all processes
        //or the program will lock up, treat as a global barrier
        //use to safely print messages from each process without clobbering
        void PrintMessage(std::string msg);

        //broadcast a string message to all processes
        void BroadcastString(std::string& msg);

        //routines to be used by programs which split the processes into two
        //groups bases on even/odd local process rank
        bool SplitMode() { return fSplitMode; };

        bool IsSplitValid() { return fValidSplit; };

        bool IsEvenGroupMember() { return fIsEvenGroupMember; };

        int GetNSubGroupProcesses() { return fNSubGroupProcesses; }

        int GetSubGroupRank() { return fSubGroupRank; };

        int GetPartnerProcessID() { return fPartnerProcessID; };

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
