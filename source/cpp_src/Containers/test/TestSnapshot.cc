#include <complex>
#include <cstdio>
#include <iostream>
#include <string>
#include <sys/stat.h>
#include <unistd.h>

#include "MHO_Snapshot.hh"
#include "MHO_TestAssertions.hh"

using namespace hops;

// MHO_Snapshot writes through the take_snapshot* macros, which compile to
// nothing unless HOPS_ENABLE_SNAPSHOTS is set.  To exercise MHO_Snapshot.cc
// (AddKey / RemoveKey / RemoveAllKeys / PassSnapshot) regardless of that flag,
// this test calls the MHO_Snapshot API directly instead of via the macros.

static bool file_exists(const std::string& p)
{
    struct stat st;
    return (stat(p.c_str(), &st) == 0);
}

static long file_size(const std::string& p)
{
    struct stat st;
    if(stat(p.c_str(), &st) == 0)
    {
        return static_cast< long >(st.st_size);
    }
    return -1;
}

int main()
{
    MHO_Message::GetInstance().SetMessageLevel(eFatal);

    MHO_Snapshot& snap = MHO_Snapshot::GetInstance();

    // Unique, predictable executable name so we can locate the dump file.
    const std::string exe = "TestSnapshot_probe";
    snap.SetExecutableName(exe);

    // The dump filename is <HOPS_SNAPSHOT_DIR>/<exe>.pid<PID>.snap (see MHO_Snapshot ctor).
    const std::string outfile =
        std::string(HOPS_SNAPSHOT_DIR_STR) + "/" + exe + ".pid" + std::to_string(static_cast< int >(getpid())) + ".snap";
    std::remove(outfile.c_str()); // clean slate

    // Build a representative object to dump.
    size_t dim[VIS_NDIM];
    dim[POLPROD_AXIS] = 4;
    dim[CHANNEL_AXIS] = 8;
    dim[TIME_AXIS] = 3;
    dim[FREQ_AXIS] = 3;

    visibility_type vis;
    vis.Resize(dim);

    std::complex< double > unit(1.0, 0.0);
    std::complex< double > i_unit(0.0, 1.0);
    vis.SetArray(unit);
    for(std::size_t i = 0; i < dim[0]; i++)
    {
        auto vis_slice = vis.SliceView(i, ":", ":", ":");
        std::complex< double > tmp = (static_cast< double >(i + 1) * 2.0) * i_unit;
        vis_slice *= tmp;
    }

    // C1: key-gated dumping (LimitToKeySet) exercises PassSnapshot plus
    //     AddKey / RemoveKey / RemoveAllKeys from MHO_Snapshot.cc

    snap.LimitToKeySet();
    snap.RemoveAllKeys();
    snap.AddKey("accepted"); // std::string overload

    // a key that is not registered must NOT produce any output
    snap.DumpObject(&vis, "rejected", "vis_rejected");
    REQUIRE(file_exists(outfile) == false);

    // a registered key dumps the object
    snap.DumpObject(&vis, "accepted", "vis_accepted");
    REQUIRE(file_exists(outfile) == true);
    long size_after_first = file_size(outfile);
    REQUIRE(size_after_first > 0);

    // a rejected key must not grow the file
    snap.DumpObject(&vis, "rejected", "vis_rejected2");
    REQUIRE(file_size(outfile) == size_after_first);

    // removing the accepted key disables further dumps for it
    snap.RemoveKey("accepted"); // std::string overload
    snap.DumpObject(&vis, "accepted", "vis_accepted2");
    REQUIRE(file_size(outfile) == size_after_first);

    // C2: AcceptAllKeys dumps regardless of key.  The explicit 5-arg
    //     DumpObject(file/line) replaces the former take_snapshot_here macro.

    snap.AcceptAllKeys();
    snap.DumpObject(&vis, "test", "visib", __FILE__, __LINE__);
    long size_after_acceptall = file_size(outfile);
    REQUIRE(size_after_acceptall > size_after_first);

    // C3: const char* overloads of AddKey / RemoveKey also resolve & run

    snap.LimitToKeySet();
    snap.RemoveAllKeys();
    snap.AddKey("ckey");    // const char* -> std::string implicit conversion
    snap.RemoveKey("ckey"); // const char* -> std::string implicit conversion
    long size_before = file_size(outfile);
    snap.DumpObject(&vis, "ckey", "vis_ckey"); // key removed -> no growth
    REQUIRE(file_size(outfile) == size_before);

    std::remove(outfile.c_str());
    return 0;
}
