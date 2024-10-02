#include "MHO_MPIInterfaceWrapper.hh"
#include "MHO_Message.hh"

using namespace hops;

int main(int argc, char** argv)
{

    MHO_Message::GetInstance().AcceptAllKeys();
    MHO_Message::GetInstance().SetMessageLevel(eDebug);

#ifdef HOPS_USE_MPI
    MHO_MPIInterface::GetInstance()->Initialize(&argc, &argv, true); //run with no local even/odd split
    int process_id = MHO_MPIInterface::GetInstance()->GetGlobalProcessID();
    int local_id = MHO_MPIInterface::GetInstance()->GetLocalProcessID();
    int n_processes = MHO_MPIInterface::GetInstance()->GetNProcesses();

    //first use the PrintMessage interface
    std::stringstream ss;
    std::string hostname = MHO_MPIInterface::GetInstance()->GetHostName();
    ss << "hello from global process ID# " << process_id << " local process ID# " << local_id << " on host: " << hostname
       << "\n";
    MHO_MPIInterface::GetInstance()->PrintMessage(ss.str());

    MHO_MPIInterface::GetInstance()->GlobalBarrier();

    std::string a_msg;
    if(MHO_MPIInterface::GetInstance()->GetGlobalProcessID() == 0)
    {
        a_msg = "hello there";
    }
    //now have each process print it's ID and local message value, sequentially
    MHO_MPIInterface::GetInstance()->BeginSequentialProcess();
    msg_info("main", "hello from global process ID# " << process_id << " local process ID# " << local_id
                                                      << " out of: " << n_processes << eom);
    msg_info("main", "message contents are <" << a_msg << "> " << eom);
    MHO_MPIInterface::GetInstance()->EndSequentialProcess();

    //now broadcast the root's  message to the other processes
    MHO_MPIInterface::GetInstance()->GlobalBarrier();
    MHO_MPIInterface::GetInstance()->BroadcastString(a_msg);

    //and have each process print it's ID ad message again, sequentially
    MHO_MPIInterface::GetInstance()->BeginSequentialProcess();
    msg_info("main", "hello from global process ID# " << process_id << " local process ID# " << local_id
                                                      << " out of: " << n_processes << eom);
    msg_info("main", "message contents are now <" << a_msg << "> " << eom);
    MHO_MPIInterface::GetInstance()->EndSequentialProcess();

    MHO_MPIInterface::GetInstance()->GlobalBarrier();
    MHO_MPIInterface::GetInstance()->Finalize();

#else
    msg_info("main", "no MPI interface present, recompile with HOPS_USE_MPI=ON" << eom);
#endif

    return 0;
}
