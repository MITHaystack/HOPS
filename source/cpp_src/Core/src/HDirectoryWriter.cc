#include "HDirectoryWriter.hh"

#include <cstring>
#include <sstream>
#include <iostream>

//needed for mkdir on *NIX
#include <sys/types.h>
#include <sys/stat.h>

namespace hops
{

HDirectoryWriter::HDirectoryWriter():
    fExperimentName("unknown"),
    fSourceName("unknown"),
    fScanName("unknown")
{
    //if unassigned use default data dir
    fBaseOutputDirectory = std::string(STR2(DATA_INSTALL_DIR));
};

void HDirectoryWriter::SetExperimentName(std::string exp_name)
{
    //truncate experiment name if needed
    if(exp_name.size() < HNAME_WIDTH){fExperimentName = exp_name;}
    else{fExperimentName = exp_name.substr(0,HNAME_WIDTH-1);}
};

std::string 
HDirectoryWriter::GetExperimentName() const
{
    return fExperimentName;
}

void HDirectoryWriter::SetSourceName(std::string source_name)
{
    if(source_name.size() < HNAME_WIDTH){fSourceName = source_name;}
    else{fSourceName = source_name.substr(0,HNAME_WIDTH-1);}
};

std::string 
HDirectoryWriter::GetSourceName() const
{
    return fSourceName;
}

void HDirectoryWriter::SetScanName(std::string scan_name)
{
    if(scan_name.size() < HNAME_WIDTH){fScanName = scan_name;}
    else{fScanName = scan_name.substr(0,HNAME_WIDTH-1);}
};

std::string 
HDirectoryWriter::GetScanName() const
{
    return fScanName;
}

void HDirectoryWriter::SetBaseOutputDirectory(std::string output_dir)
{
    fBaseOutputDirectory = output_dir;
}

std::string 
HDirectoryWriter::GetBaseOutputDirectory() const
{
    return fBaseOutputDirectory;
}

void HDirectoryWriter::InitializeOutputDirectory()
{
    //assume fBaseOutputDirectory and fScanName have been set
    
    //check for the existence of fBaseOutputDirectory
    struct stat sb;
    if( !(stat(fBaseOutputDirectory.c_str(), &sb) == 0 && S_ISDIR(sb.st_mode) ) )
    {
        std::cout<<"Error base output directory: "<<fBaseOutputDirectory<<" does not exist! Using: "<< STR2(DATA_INSTALL_DIR) <<std::endl;
        std::stringstream dss;
        dss << STR2(DATA_INSTALL_DIR);
        fBaseOutputDirectory = dss.str();
    }

    //create data output directory
    std::stringstream ss;
    ss << fBaseOutputDirectory;
    ss << "/";
    ss << fExperimentName;

    //now check if the directory we want to make already exists, if not, create it
    struct stat sb2;
    if(!( stat(ss.str().c_str(), &sb2) == 0 && S_ISDIR(sb2.st_mode) ))
    {
        int dirstatus = mkdir(ss.str().c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

        if(dirstatus)
        {
            std::cout<<"Problem when attempting to create directory: "<< ss.str() << std::endl;
        }
    }

    std::stringstream ss2;
    ss2 << fBaseOutputDirectory;
    ss2 << "/";
    ss2 << fExperimentName;
    ss2 << "/";
    ss2 << fScanName;

    fCurrentOutputDirectory = ss2.str();

    //now check if the directory we want to make already exists, if not, create it
    struct stat sb3;
    if(!( stat(ss2.str().c_str(), &sb3) == 0 && S_ISDIR(sb3.st_mode) ))
    {
        int dirstatus = mkdir(ss2.str().c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

        if(dirstatus)
        {
            fCurrentOutputDirectory = "./"; //fall back
            std::cout<<"Problem when attempting to create directory: "<< ss2.str() << std::endl;
        }
    }
}

std::string 
HDirectoryWriter::GetCurrentOutputDirectory() const 
{
    return fCurrentOutputDirectory;
};


}//end of namespace