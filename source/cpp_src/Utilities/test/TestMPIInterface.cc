#include "MHO_Message.hh"
#include "MHO_MPIInterfaceWrapper.hh"

using namespace hops;

int main(int argc, char** argv)
{

    MHO_Message::GetInstance().AcceptAllKeys();
    MHO_Message::GetInstance().SetMessageLevel(eDebug);

    #ifdef HOPS_USE_MPI
        MHO_MPIInterface::GetInstance()->Initialize(&argc, &argv, true); //run with no local even/odd split

        //have each process print it's ID, sequentially
        MHO_MPIInterface::GetInstance()->BeginSequentialProcess();
        int process_id = MHO_MPIInterface::GetInstance()->GetGlobalProcessID();
        int local_id = MHO_MPIInterface::GetInstance()->GetLocalProcessID();
        int n_processes = MHO_MPIInterface::GetInstance()->GetNProcesses();
        msg_info("main", "hello from global process ID# " << process_id << " local process ID# "<< local_id << " out of: " << n_processes << eom);
        MHO_MPIInterface::GetInstance()->EndSequentialProcess();

        //now use the PrintMessage interface
        std::stringstream ss;
        std::string hostname = MHO_MPIInterface::GetInstance()->GetHostName();
        ss << "hello from global process ID# " << process_id << " local process ID# "<< local_id << " on host: " << hostname << "\n";
        MHO_MPIInterface::GetInstance()->PrintMessage( ss.str() );

        MHO_MPIInterface::GetInstance()->GlobalBarrier();
        MHO_MPIInterface::GetInstance()->Finalize();
    #else
        msg_info("main", "no MPI interface present, recompile with HOPS_USE_MPI=ON" << eom);
    #endif

    return 0;
}
