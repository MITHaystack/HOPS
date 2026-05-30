// Helper program for the multi-process lock-contention test orchestrated by
// run_lock_contention.sh. Each invocation repeatedly acquires the write lock on
// a shared directory, claims the next fringe sequence number, writes a fringe
// file with that number (while still holding the lock), then releases. The
// orchestrator launches many copies in parallel and checks that the union of
// all claimed sequence numbers is unique and contiguous.
//
// usage: LockContentionWorker <lock_dir> <num_iterations> <legacy|nolegacy>

#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>

#include <sys/time.h>
#include <unistd.h>

#include "MHO_LockFileHandler.hh"
#include "MHO_Message.hh"

using namespace hops;

int main(int argc, char** argv)
{
    if(argc < 4)
    {
        std::cerr << "usage: LockContentionWorker <lock_dir> <num_iterations> <legacy|nolegacy>" << std::endl;
        return 2;
    }

    std::string dir = argv[1];
    int iters = std::atoi(argv[2]);
    std::string mode = argv[3];
    bool legacy = (mode == "legacy");

    // suppress library logging so it cannot pollute the parsed "SEQ" stdout lines
    MHO_Message::GetInstance().SetMessageLevel(eFatal);

    MHO_LockFileHandler& h = MHO_LockFileHandler::GetInstance();
    if(legacy)
    {
        h.EnableLegacyMode();
    }
    else
    {
        h.DisableLegacyMode();
    }

    for(int i = 0; i < iters; i++)
    {
        int seq = -1;
        int rc = h.WaitForWriteLock(dir, seq);
        if(rc != LOCK_STATUS_OK)
        {
            std::cerr << "ERR " << rc << std::endl;
            return 1;
        }

        // report the claimed sequence number to the orchestrator, tagged with our
        // pid and the acquire timestamp so a collision can be traced back to two
        // processes that arbitrated at (nearly) the same microsecond
        struct timeval tv;
        gettimeofday(&tv, NULL);
        char tbuf[64];
        snprintf(tbuf, sizeof(tbuf), "%ld.%06ld", static_cast< long >(tv.tv_sec), static_cast< long >(tv.tv_usec));
        std::cout << "SEQ " << seq << " pid=" << getpid() << " t=" << tbuf << std::endl;

        // write a fringe file with this sequence number WHILE still holding the
        // lock, so the next acquirer observes the incremented maximum
        std::string fname;
        if(legacy)
        {
            fname = dir + "/GE.X." + std::to_string(seq) + ".ABCDEF";
        }
        else
        {
            fname = dir + "/GE.Gs-Wf.X.XY.0VSI1M." + std::to_string(seq) + ".frng";
        }
        {
            std::ofstream ofs(fname.c_str());
            ofs << "x";
        }

        // widen the critical section to provoke real contention between processes
        usleep(10000);

        h.RemoveWriteLock();
    }

    return 0;
}
