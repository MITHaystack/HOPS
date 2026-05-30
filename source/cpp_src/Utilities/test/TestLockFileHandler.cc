#include <cstdio>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "MHO_DirectoryInterface.hh"
#include "MHO_LockFileHandler.hh"
#include "MHO_Message.hh"
#include "MHO_TestAssertions.hh"

using namespace hops;

static void write_empty(const std::string& path)
{
    std::ofstream ofs(path.c_str());
    ofs << "x";
}

// count the *.lock files currently present in dir
static int count_locks(const std::string& dir)
{
    MHO_DirectoryInterface di;
    di.SetCurrentDirectory(dir);
    di.ReadCurrentDirectory();
    std::vector< std::string > locks;
    di.GetFilesMatchingExtention(locks, ".lock");
    return static_cast< int >(locks.size());
}

// remove every regular file in dir (leaves the directory itself in place)
static void clear_dir(const std::string& dir)
{
    MHO_DirectoryInterface di;
    di.SetCurrentDirectory(dir);
    di.ReadCurrentDirectory();
    std::vector< std::string > files;
    di.GetFileList(files);
    for(std::size_t i = 0; i < files.size(); i++)
    {
        std::remove(files[i].c_str());
    }
}

int main(int /*argc*/, char** /*argv*/)
{
    MHO_Message::GetInstance().SetMessageLevel(eFatal);

    const std::string base = "TestLockFileHandler_tmp";
    mkdir(base.c_str(), 0755);
    clear_dir(base); // best-effort clean of any stale run

    MHO_LockFileHandler& h = MHO_LockFileHandler::GetInstance();
    h.EnableLegacyMode(); // default, but be explicit

    // Case 1: empty directory -> first sequence number is 1; lock is created then removed
    {
        clear_dir(base);
        int seq = -99;
        int rc = h.WaitForWriteLock(base, seq);
        REQUIRE(rc == LOCK_STATUS_OK);
        REQUIRE(seq == 1);
        REQUIRE(count_locks(base) == 1); // lock held

        h.RemoveWriteLock();
        REQUIRE(count_locks(base) == 0); // lock released

        // removing again is a harmless no-op
        h.RemoveWriteLock();
        REQUIRE(count_locks(base) == 0);
    }

    // Case 2: legacy fringe file present -> next sequence number is max+1
    {
        clear_dir(base);
        write_empty(base + "/GE.X.7.ABCDEF"); // legacy fringe with sequence number 7
        int seq = -99;
        int rc = h.WaitForWriteLock(base, seq);
        REQUIRE(rc == LOCK_STATUS_OK);
        REQUIRE(seq == 8);
        h.RemoveWriteLock();
    }

    // Case 3: sequence advances across release / re-acquire when a fringe file is written
    {
        clear_dir(base);
        int seq = -99;

        int rc = h.WaitForWriteLock(base, seq);
        REQUIRE(rc == LOCK_STATUS_OK);
        REQUIRE(seq == 1);
        write_empty(base + "/GE.X.1.ABCDEF"); // emulate writing the fringe for seq 1
        h.RemoveWriteLock();

        rc = h.WaitForWriteLock(base, seq);
        REQUIRE(rc == LOCK_STATUS_OK);
        REQUIRE(seq == 2);
        h.RemoveWriteLock();
    }

    // Case 4: non-legacy mode reads ".frng" naming and increments correctly
    {
        clear_dir(base);
        h.DisableLegacyMode();
        // new-style fringe file: tokens "GE.Gs-Wf.X.XY.0VSI1M.<seq>.frng", seq is 2nd-to-last
        write_empty(base + "/GE.Gs-Wf.X.XY.0VSI1M.3.frng");
        int seq = -99;
        int rc = h.WaitForWriteLock(base, seq);
        REQUIRE(rc == LOCK_STATUS_OK);
        REQUIRE(seq == 4);
        h.RemoveWriteLock();
        h.EnableLegacyMode(); // restore default for any later use
    }

    // Case 5: a stale lock from a dead/foreign holder is reclaimed, not fatal
    {
        clear_dir(base);
        // synthetic lock file: pid.seq.time_sec(hex).time_usec(hex).host.lock
        // a foreign hostname skips the same-host liveness probe, and the ancient
        // timestamp (sec=1) makes it stale by the age backstop, so it must be
        // reclaimed rather than poisoning the directory with an error.
        write_empty(base + "/424242.1.1.0.otherhost.lock");
        int seq = -99;
        int rc = h.WaitForWriteLock(base, seq);
        REQUIRE(rc == LOCK_STATUS_OK);
        REQUIRE(seq == 1);
        // the stale lock was removed; only our freshly-acquired lock remains
        REQUIRE(count_locks(base) == 1);
        h.RemoveWriteLock();
        REQUIRE(count_locks(base) == 0);
    }

    // teardown
    clear_dir(base);
    rmdir(base.c_str());

    return 0;
}
