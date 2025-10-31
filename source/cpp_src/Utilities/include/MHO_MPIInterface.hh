#ifndef MHO_MPIInterface_HH__
#define MHO_MPIInterface_HH__

#include "mpi.h"

#include "MHO_Message.hh"
#include <string>
#include <vector>

#define LOCAL_RANK_MPI

namespace hops
{

//helper template for mapping basic types to MPI type codes
template <typename T>
MPI_Datatype mpi_type_for();

template <>
inline MPI_Datatype mpi_type_for<int>() { return MPI_INT; }

template <>
inline MPI_Datatype mpi_type_for<double>() { return MPI_DOUBLE; }

template <>
inline MPI_Datatype mpi_type_for<float>() { return MPI_FLOAT; }


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

        //merge a collection of maps across all processes, so that it is available for the root (0) process
        template <typename T>
        std::map<std::string, T>
        MergeMap(const std::map<std::string, T>& local_map, MPI_Comm comm)
        {
            static_assert(std::is_trivially_copyable_v<T>, "MHO_MPIInterface::MergeMap only supports trivially copyable types.");

            int rank, size;
            MPI_Comm_rank(comm, &rank);
            MPI_Comm_size(comm, &size);

            //serialize the local map into primitive types
            std::vector<int> key_lengths;
            std::string concatenated_keys;
            std::vector<T> values;

            for (const auto& kv : local_map) 
            {
                key_lengths.push_back(static_cast<int>(kv.first.size()));
                concatenated_keys += kv.first;
                values.push_back(kv.second);
            }

            int local_entry_count = static_cast<int>(local_map.size());
            int local_char_count = static_cast<int>(concatenated_keys.size());

            // --- Gather entry counts ---
            std::vector<int> entry_counts(size);
            MPI_Gather(&local_entry_count, 1, MPI_INT, entry_counts.data(), 1, MPI_INT, 0, comm);

            // --- Gather character counts ---
            std::vector<int> char_counts(size);
            MPI_Gather(&local_char_count, 1, MPI_INT, char_counts.data(), 1, MPI_INT, 0, comm);

            std::map<std::string, T> merged;

            // --- Only rank 0 reconstructs ---
            if (rank == 0) 
            {
                // Compute displacements
                std::vector<int> entry_displs(size, 0);
                std::vector<int> char_displs(size, 0);

                for (int i = 1; i < size; ++i) 
                {
                    entry_displs[i] = entry_displs[i-1] + entry_counts[i-1];
                    char_displs[i]  = char_displs[i-1] + char_counts[i-1];
                }

                int total_entries = entry_displs[size-1] + entry_counts[size-1];
                int total_chars   = char_displs[size-1] + char_counts[size-1];

                // Allocate receive buffers
                std::vector<int> all_key_lengths(total_entries);
                std::vector<char> all_chars(total_chars);
                std::vector<T> all_values(total_entries);

                // Gather key lengths
                MPI_Gatherv(key_lengths.data(), local_entry_count, MPI_INT,
                            all_key_lengths.data(), entry_counts.data(), entry_displs.data(), MPI_INT,
                            0, comm);

                // Gather keys
                MPI_Gatherv(concatenated_keys.data(), local_char_count, MPI_CHAR,
                            all_chars.data(), char_counts.data(), char_displs.data(), MPI_CHAR,
                            0, comm);

                // Gather values
                MPI_Gatherv(values.data(), local_entry_count, mpi_type_for<T>(),
                            all_values.data(), entry_counts.data(), entry_displs.data(), mpi_type_for<T>(),
                            0, comm);

                // Reconstruct merged map
                size_t pos = 0;
                for (int i = 0; i < total_entries; ++i) 
                {
                    int len = all_key_lengths[i];
                    std::string key(all_chars.begin() + pos, all_chars.begin() + pos + len);
                    pos += len;
                    merged[key] = all_values[i];
                }
            } 
            else 
            {
                // Non-root ranks just need to participate in gathers
                MPI_Gatherv(key_lengths.data(), local_entry_count, MPI_INT,
                            nullptr, nullptr, nullptr, MPI_INT,
                            0, comm);

                MPI_Gatherv(concatenated_keys.data(), local_char_count, MPI_CHAR,
                            nullptr, nullptr, nullptr, MPI_CHAR,
                            0, comm);

                MPI_Gatherv(values.data(), local_entry_count, mpi_type_for<T>(),
                            nullptr, nullptr, nullptr, mpi_type_for<T>(),
                            0, comm);
            }

            return merged;
        }

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
