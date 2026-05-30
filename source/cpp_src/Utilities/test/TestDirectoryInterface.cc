#include <cstdio>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "MHO_DirectoryInterface.hh"
#include "MHO_Message.hh"
#include "MHO_TestAssertions.hh"

using namespace hops;

static void write_file(const std::string& path, const std::string& content)
{
    std::ofstream ofs(path.c_str());
    ofs << content;
}

// true if any entry's basename equals the given name
static bool list_contains(const std::vector< std::string >& v, const std::string& name)
{
    for(std::size_t i = 0; i < v.size(); i++)
    {
        if(MHO_DirectoryInterface::GetBasename(v[i]) == name)
        {
            return true;
        }
    }
    return false;
}

static void teardown(const std::string& base, const std::vector< std::string >& fnames)
{
    for(std::size_t i = 0; i < fnames.size(); i++)
    {
        std::remove((base + "/" + fnames[i]).c_str());
    }
    rmdir((base + "/subdir.dir").c_str());
    rmdir((base + "/plain_sub").c_str());
    rmdir((base + "/created_subdir").c_str());
    rmdir(base.c_str());
}

int main(int /*argc*/, char** /*argv*/)
{
    MHO_Message::GetInstance().SetMessageLevel(eFatal);

    const std::string base = "TestDirectoryInterface_tmp";
    const std::string symlink_path = "TestDirectoryInterface_symlink";

    // fixture file basenames, created inside base
    std::vector< std::string > fnames;
    fnames.push_back("alpha.txt");
    fnames.push_back("beta.dat");
    fnames.push_back("gamma.txt");
    fnames.push_back("data.cor");
    fnames.push_back("vex1234.3C2D5E"); // single-dot root-style file holding the "VEX" magic
    fnames.push_back("GE..3C2D5E");     // corel:   2-char station pair + 6-char root code
    fnames.push_back("G..3C2D5E");      // station: 1-char station + 6-char root code
    fnames.push_back("GE.X.1.3C2D5E");  // fringe:  baseline.fgroup.seq.root, seq 1
    fnames.push_back("GE.X.5.3C2D5E");  // fringe:  seq 5

    //  Build the fixture directory tree (best-effort clean of any stale run first)
    teardown(base, fnames);
    unlink(symlink_path.c_str());

    mkdir(base.c_str(), 0755);
    for(std::size_t i = 0; i < fnames.size(); i++)
    {
        std::string content = (fnames[i] == "vex1234.3C2D5E") ? "VEX rev 1.5\nstuff" : "data";
        write_file(base + "/" + fnames[i], content);
    }
    mkdir((base + "/subdir.dir").c_str(), 0755);
    mkdir((base + "/plain_sub").c_str(), 0755);

    // Case 1: path/basename string utilities (static, pure)
    {
        REQUIRE_EQUAL(MHO_DirectoryInterface::GetBasename("/a/b/c.txt"), "c.txt");
        REQUIRE_EQUAL(MHO_DirectoryInterface::GetBasename("bare"), "bare");
        REQUIRE_EQUAL(MHO_DirectoryInterface::GetBasename("/a/b/"), ""); // trailing slash -> empty basename

        REQUIRE_EQUAL(MHO_DirectoryInterface::GetPrefix("/a/b/c.txt"), "/a/b");
        REQUIRE_EQUAL(MHO_DirectoryInterface::GetPrefix("bare"), ""); // no prefix -> empty (+ warn)

        REQUIRE_EQUAL(MHO_DirectoryInterface::GetTrailingDirectory("/foo/bar/baz"), "baz");

        REQUIRE_EQUAL(MHO_DirectoryInterface::StripExtensionFromBasename("file.txt"), "file");
        REQUIRE_EQUAL(MHO_DirectoryInterface::StripExtensionFromBasename("a.b.c"), "a.b"); // strips last ext only
        REQUIRE_EQUAL(MHO_DirectoryInterface::StripExtensionFromBasename("noext"), ""); // no dot -> empty (+ warn)

        REQUIRE_EQUAL(MHO_DirectoryInterface::GetFileExtension("alpha.txt"), "txt");
        REQUIRE_EQUAL(MHO_DirectoryInterface::GetFileExtension("a.b.c"), "c");
        REQUIRE_EQUAL(MHO_DirectoryInterface::GetFileExtension("noext"), "");
        REQUIRE_EQUAL(MHO_DirectoryInterface::GetFileExtension("trailingdot."), "");
    }

    // Case 2: existence / type predicates (static)
    {
        std::string filepath = base + "/alpha.txt";

        REQUIRE(MHO_DirectoryInterface::DoesDirectoryExist(base) == true);
        REQUIRE(MHO_DirectoryInterface::DoesDirectoryExist(filepath) == false);
        REQUIRE(MHO_DirectoryInterface::DoesDirectoryExist("no_such_dir_xyz") == false);

        REQUIRE(MHO_DirectoryInterface::IsDirectory(base) == true);
        REQUIRE(MHO_DirectoryInterface::IsDirectory(filepath) == false);
        REQUIRE(MHO_DirectoryInterface::IsDirectory("no_such_path_xyz") == false);

        REQUIRE(MHO_DirectoryInterface::IsFile(filepath) == true);
        REQUIRE(MHO_DirectoryInterface::IsFile(base) == false);
        REQUIRE(MHO_DirectoryInterface::IsFile("no_such_file_xyz") == false);
    }

    // Case 3: GetDirectoryFullPath (static, realpath)
    {
        std::string full = MHO_DirectoryInterface::GetDirectoryFullPath(base);
        REQUIRE(full.size() > 0);
        REQUIRE(full[0] == '/'); // absolute
        REQUIRE_EQUAL(MHO_DirectoryInterface::GetBasename(full), base);
        REQUIRE(MHO_DirectoryInterface::DoesDirectoryExist(full) == true);
        // resolving an already-absolute existing path is idempotent
        REQUIRE_EQUAL(MHO_DirectoryInterface::GetDirectoryFullPath(full), full);
    }

    // Case 4: GetDirectoryFullPathPreserveSymlinks (static, readlink)
    {
        // a non-symlink input returns the path unchanged (readlink fails -> fallback)
        std::string filepath = base + "/alpha.txt";
        REQUIRE_EQUAL(MHO_DirectoryInterface::GetDirectoryFullPathPreserveSymlinks(filepath), filepath);

        // a real symlink resolves to its (literal) target rather than the dereferenced path
        unlink(symlink_path.c_str());
        REQUIRE(symlink("some_target", symlink_path.c_str()) == 0);
        REQUIRE_EQUAL(MHO_DirectoryInterface::GetDirectoryFullPathPreserveSymlinks(symlink_path), "some_target");
        unlink(symlink_path.c_str());
    }

    // Case 5: GetFileModifcationTime (static)
    {
        std::string ts = MHO_DirectoryInterface::GetFileModifcationTime(base + "/alpha.txt");
        REQUIRE(ts.size() > 0);
        REQUIRE(ts.find('T') != std::string::npos);
        REQUIRE(ts.find('Z') != std::string::npos);
        // a missing file leaves epoch_sec == 0 -> the unix epoch timestamp
        REQUIRE_EQUAL(MHO_DirectoryInterface::GetFileModifcationTime("no_such_file_xyz"), "1970-01-01T00:00:00Z");
    }

    // Case 6: directory navigation + listing (instance)
    {
        MHO_DirectoryInterface di;

        // reading before a directory is set fails
        REQUIRE(di.ReadCurrentDirectory() == false);

        di.SetCurrentDirectory(base);
        REQUIRE_EQUAL(di.GetCurrentDirectory(), MHO_DirectoryInterface::GetDirectoryFullPath(base));
        REQUIRE_EQUAL(MHO_DirectoryInterface::GetBasename(di.GetCurrentDirectory()), base);
        REQUIRE(di.GetCurrentParentDirectory().size() > 0);

        REQUIRE(di.ReadCurrentDirectory() == true);
        // a second read is a no-op and reports false (already read)
        REQUIRE(di.ReadCurrentDirectory() == false);

        std::vector< std::string > files;
        di.GetFileList(files);
        REQUIRE(files.size() == fnames.size());
        for(std::size_t i = 0; i < fnames.size(); i++)
        {
            REQUIRE(list_contains(files, fnames[i]));
        }

        std::vector< std::string > subdirs;
        di.GetSubDirectoryList(subdirs);
        REQUIRE(subdirs.size() == 2); // subdir.dir + plain_sub ('.' and '..' excluded)
        REQUIRE(list_contains(subdirs, "subdir.dir"));
        REQUIRE(list_contains(subdirs, "plain_sub"));
    }

    // Case 7: extension / prefix filters (instance, both std::string and const char* overloads)
    {
        MHO_DirectoryInterface di;
        di.SetCurrentDirectory(base);
        REQUIRE(di.ReadCurrentDirectory() == true);

        std::vector< std::string > txt;
        di.GetFilesMatchingExtention(txt, std::string(".txt"));
        REQUIRE(txt.size() == 2);
        REQUIRE(list_contains(txt, "alpha.txt"));
        REQUIRE(list_contains(txt, "gamma.txt"));

        std::vector< std::string > txt2;
        di.GetFilesMatchingExtention(txt2, ".txt"); // const char* overload
        REQUIRE(txt2.size() == 2);

        std::vector< std::string > pref;
        di.GetFilesMatchingPrefix(pref, std::string("beta"));
        REQUIRE(pref.size() == 1);
        REQUIRE(list_contains(pref, "beta.dat"));

        std::vector< std::string > pref2;
        di.GetFilesMatchingPrefix(pref2, "beta"); // const char* overload
        REQUIRE(pref2.size() == 1);

        std::vector< std::string > dirext;
        di.GetSubDirectoriesMatchingExtention(dirext, std::string(".dir"));
        REQUIRE(dirext.size() == 1);
        REQUIRE(list_contains(dirext, "subdir.dir"));

        std::vector< std::string > dirext2;
        di.GetSubDirectoriesMatchingExtention(dirext2, ".dir"); // const char* overload
        REQUIRE(dirext2.size() == 1);
    }

    // Case 8: legacy mk4 filename classification + splitters (instance)
    {
        MHO_DirectoryInterface di;

        std::vector< std::string > mk4;
        mk4.push_back("/data/exp/GE..3C2D5E");     // corel
        mk4.push_back("/data/exp/G..3C2D5E");      // station
        mk4.push_back("/data/exp/GE.X.1.3C2D5E");  // fringe seq 1
        mk4.push_back("/data/exp/GE.X.5.3C2D5E");  // fringe seq 5
        mk4.push_back("/data/exp/vex1234.3C2D5E"); // single-dot (root-style)
        mk4.push_back("/data/exp/notes.log");      // noise

        std::vector< std::string > corel;
        di.GetCorelFiles(mk4, corel);
        REQUIRE(corel.size() == 1);
        REQUIRE_EQUAL(MHO_DirectoryInterface::GetBasename(corel[0]), "GE..3C2D5E");

        std::vector< std::string > station;
        di.GetStationFiles(mk4, station);
        REQUIRE(station.size() == 1);
        REQUIRE_EQUAL(MHO_DirectoryInterface::GetBasename(station[0]), "G..3C2D5E");

        std::vector< std::string > fringe;
        int max_seq = -1;
        di.GetFringeFiles(mk4, fringe, max_seq);
        REQUIRE(fringe.size() == 2);
        REQUIRE(max_seq == 5);

        std::string st_pair, root_code;
        di.SplitCorelFileBasename("GE..3C2D5E", st_pair, root_code);
        REQUIRE_EQUAL(st_pair, "GE");
        REQUIRE_EQUAL(root_code, "3C2D5E");

        std::string st, root_code2;
        di.SplitStationFileBasename("G..3C2D5E", st, root_code2);
        REQUIRE_EQUAL(st, "G");
        REQUIRE_EQUAL(root_code2, "3C2D5E");
    }

    // Case 9: GetRootFile (opens candidate files, looks for the "VEX" magic)
    {
        MHO_DirectoryInterface di;

        std::vector< std::string > files;
        files.push_back(base + "/vex1234.3C2D5E"); // 1 dot, 6-char ext, contains "VEX"
        files.push_back(base + "/alpha.txt");      // not a root file

        std::string root;
        di.GetRootFile(files, root);
        REQUIRE_EQUAL(MHO_DirectoryInterface::GetBasename(root), "vex1234.3C2D5E");

        // a list with no qualifying file yields an empty result
        std::vector< std::string > nofiles;
        nofiles.push_back(base + "/alpha.txt");
        std::string none = "sentinel";
        di.GetRootFile(nofiles, none);
        REQUIRE_EQUAL(none, "");
    }

    // Case 10: CreateDirectory (static)
    {
        std::string newdir = base + "/created_subdir";
        REQUIRE(MHO_DirectoryInterface::CreateDirectory(newdir) == true);
        REQUIRE(MHO_DirectoryInterface::DoesDirectoryExist(newdir) == true);
        REQUIRE(MHO_DirectoryInterface::IsDirectory(newdir) == true);
        // create is idempotent: re-creating an existing directory still succeeds
        REQUIRE(MHO_DirectoryInterface::CreateDirectory(newdir) == true);
        // but a path that already exists as a regular file is not a directory -> fails
        REQUIRE(MHO_DirectoryInterface::CreateDirectory(base + "/alpha.txt") == false);
        rmdir(newdir.c_str());
    }

    //  Tear down the fixture
    teardown(base, fnames);

    return 0;
}
