#include "MHO_PythonSubprocessRunner.hh"

#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

#include "MHO_DirectoryInterface.hh"
#include "MHO_Message.hh"

namespace hops
{

std::string MHO_PythonSubprocessRunner::ResolveInterpreter()
{
    //$HOPS_PYTHON wins (lets the user point at a specific interpreter for the
    //subprocess path); otherwise rely on the shell's PATH lookup of "python3".
    const char* hops_python = std::getenv("HOPS_PYTHON");
    if(hops_python != nullptr && hops_python[0] != '\0')
    {
        return std::string(hops_python);
    }
    return std::string("python3");
}

std::string MHO_PythonSubprocessRunner::ResolvePackageDir()
{
    //follows source/bash_src/hops_pypath.sh.in: <prefix>/<site-packages subdir>.
    //Prefer the $HOPS_INSTALL the user's sourced environment advertises (same
    //authority the shell helper uses), otherwise derive the prefix from the running
    //binary so an un-sourced/relocated install still resolves.
    std::string prefix;
    const char* hops_install = std::getenv("HOPS_INSTALL");
    if(hops_install != nullptr && hops_install[0] != '\0')
    {
        prefix = std::string(hops_install);
    }
    else
    {
        prefix = MHO_DirectoryInterface::GetHopsInstallPrefix();
    }

#ifdef HOPS_PYTHON_SITE_SUBDIR
    std::string subdir = HOPS_PYTHON_SITE_SUBDIR;
#else
    std::string subdir;
#endif

    if(prefix.empty() || subdir.empty())
    {
        return std::string();
    }
    if(prefix.back() == '/')
    {
        prefix.pop_back();
    }
    return prefix + "/" + subdir;
}

namespace
{

//Compute the PYTHONPATH for the child: our shipped site dir prepended to any
//inherited PYTHONPATH (so a shell that already sourced hops_pypath.sh keeps
//working, and our self-contained packages take precedence).
std::string compose_child_pythonpath()
{
    std::string pkg_dir = MHO_PythonSubprocessRunner::ResolvePackageDir();
    const char* inherited = std::getenv("PYTHONPATH");
    std::string existing = (inherited != nullptr) ? std::string(inherited) : std::string();

    if(pkg_dir.empty())
    {
        return existing;
    }
    if(existing.empty())
    {
        return pkg_dir;
    }
    return pkg_dir + ":" + existing;
}

//Quote a string for safe inclusion in a /bin/sh command line (popen runs the
//command via the shell). Wrap in single quotes, escaping embedded single quotes
//as the usual '\'' sequence.
std::string shell_single_quote(const std::string& s)
{
    std::string out = "'";
    for(char c : s)
    {
        if(c == '\'')
        {
            out += "'\\''";
        }
        else
        {
            out.push_back(c);
        }
    }
    out.push_back('\'');
    return out;
}

//Create a temp file in TMPDIR (or /tmp), write `contents`, return its path.
//Returns empty string on failure.
std::string write_temp_file(const std::string& tag, const std::string& contents)
{
    const char* tmpdir = std::getenv("TMPDIR");
    std::string dir = (tmpdir != nullptr && tmpdir[0] != '\0') ? std::string(tmpdir) : std::string("/tmp");
    if(dir.back() == '/')
    {
        dir.pop_back();
    }
    std::string templ = dir + "/hops_" + tag + "_XXXXXX";
    std::vector< char > buf(templ.begin(), templ.end());
    buf.push_back('\0');

    int fd = ::mkstemp(buf.data());
    if(fd < 0)
    {
        msg_error("python_subprocess", "failed to create temp file (" << templ << "): " << std::strerror(errno) << eom);
        return std::string();
    }
    std::string path(buf.data());

    //write the contents through the fd, then close it
    const char* p = contents.data();
    std::size_t remaining = contents.size();
    bool ok = true;
    while(remaining > 0)
    {
        ssize_t w = ::write(fd, p, remaining);
        if(w < 0)
        {
            if(errno == EINTR)
            {
                continue;
            }
            ok = false;
            break;
        }
        p += w;
        remaining -= static_cast< std::size_t >(w);
    }
    ::close(fd);

    if(!ok)
    {
        msg_error("python_subprocess", "failed to write temp file " << path << ": " << std::strerror(errno) << eom);
        ::remove(path.c_str());
        return std::string();
    }
    return path;
}

std::string slurp_file(const std::string& path)
{
    std::ifstream ifs(path.c_str(), std::ios::binary);
    if(!ifs.is_open())
    {
        return std::string();
    }
    return std::string((std::istreambuf_iterator< char >(ifs)), std::istreambuf_iterator< char >());
}

} // namespace

MHO_PythonSubprocessRunner::Result MHO_PythonSubprocessRunner::RunModule(const std::string& module,
                                                                         const std::string& request_json)
{
    Result result;

    //write the JSON request, pre-create the response file the child writes its
    //JSON answer into, and a file to capture the child's stderr. The request and
    //response paths are handed to the child on its command line. Keeping the
    //response on its own file (not the shared stdout) means stray output from
    //user control/plot code cannot corrupt the JSON contract.
    std::string req_path = write_temp_file("pyreq", request_json);
    if(req_path.empty())
    {
        return result;
    }
    std::string resp_path = write_temp_file("pyresp", std::string());
    if(resp_path.empty())
    {
        ::remove(req_path.c_str());
        return result;
    }
    std::string err_path = write_temp_file("pyerr", std::string());
    //err capture is best-effort; proceed even if it could not be created

    std::string interpreter = ResolveInterpreter();
    std::string child_pythonpath = compose_child_pythonpath();

    //assemble: [PYTHONPATH='...'] <interp> -m <module> '<reqfile>' '<respfile>' 2>'<errfile>'
    std::string cmd;
    if(!child_pythonpath.empty())
    {
        cmd += "PYTHONPATH=" + shell_single_quote(child_pythonpath) + " ";
    }
    cmd += shell_single_quote(interpreter);
    cmd += " -m " + shell_single_quote(module);
    cmd += " " + shell_single_quote(req_path);
    cmd += " " + shell_single_quote(resp_path);
    if(!err_path.empty())
    {
        cmd += " 2>" + shell_single_quote(err_path);
    }

    msg_debug("python_subprocess", "running python subprocess: " << cmd << eom);

    FILE* pipe = ::popen(cmd.c_str(), "r");
    if(pipe == nullptr)
    {
        msg_error("python_subprocess", "popen() failed for python subprocess: " << std::strerror(errno) << eom);
        ::remove(req_path.c_str());
        ::remove(resp_path.c_str());
        if(!err_path.empty())
        {
            ::remove(err_path.c_str());
        }
        return result;
    }
    result.spawned = true;

    //drain the child's stdout so a chatty child cannot block on a full pipe;
    //stdout is no longer the contract channel, just any user/library noise that
    //we forward to the debug log.
    std::string child_stdout;
    char buf[4096];
    std::size_t n = 0;
    while((n = ::fread(buf, 1, sizeof(buf), pipe)) > 0)
    {
        child_stdout.append(buf, n);
    }

    int status = ::pclose(pipe);

    //the JSON response is read from the response file the child wrote, not stdout
    result.out = slurp_file(resp_path);

    if(!err_path.empty())
    {
        result.err = slurp_file(err_path);
        ::remove(err_path.c_str());
    }
    ::remove(resp_path.c_str());
    ::remove(req_path.c_str());

    if(!child_stdout.empty())
    {
        msg_debug("python_subprocess", "python subprocess stdout (diagnostic): " << child_stdout << eom);
    }

    if(status != -1)
    {
        if(WIFEXITED(status))
        {
            result.exited = true;
            result.exit_code = WEXITSTATUS(status);
        }
        else if(WIFSIGNALED(status))
        {
            msg_error("python_subprocess", "python subprocess terminated by signal " << WTERMSIG(status) << eom);
        }
    }

    return result;
}

} // namespace hops
