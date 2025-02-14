#include "MHO_DiFXInterface.hh"

namespace hops
{

MHO_DiFXInterface::MHO_DiFXInterface(): fInputDirectory(""), fOutputDirectory("")
{
    fNormalize = false;
    fPreserveDiFXScanNames = false;
    fFreqBands.clear();
    fFreqGroups.clear();
    fOnlyBandwidth = 0;
    fSelectByBandwidth = false;
};

MHO_DiFXInterface::~MHO_DiFXInterface(){};

void MHO_DiFXInterface::SetInputDirectory(std::string dir)
{
    fInputDirectory = fDirInterface.GetDirectoryFullPath(dir);
}

void MHO_DiFXInterface::SetOutputDirectory(std::string dir)
{
    fOutputDirectory = fDirInterface.GetDirectoryFullPath(dir);
}

void MHO_DiFXInterface::SetStationCodes(MHO_StationCodeMap* code_map)
{
    fScanProcessor.SetStationCodes(code_map);
};

void MHO_DiFXInterface::InitializeFromExperimentDir(const std::string& input_dir)
{
    //directory interface
    MHO_DirectoryInterface fDirInterface;
    bool in_ok = fDirInterface.DoesDirectoryExist(input_dir);
    bool out_ok = fDirInterface.DoesDirectoryExist(fOutputDirectory);

    msg_info("difx_interface", "input directory: " << input_dir << eom);
    msg_info("difx_interface", "output directory: " << fOutputDirectory << eom);

    //get list of all the files (and directories) in directory
    std::vector< std::string > allFiles;
    std::vector< std::string > allSubDirs;

    fDirInterface.SetCurrentDirectory(input_dir);
    fDirInterface.ReadCurrentDirectory();
    fDirInterface.GetFileList(allFiles);
    fDirInterface.GetSubDirectoryList(allSubDirs);

    // //debug
    // for(auto it=allFiles.begin(); it != allFiles.end(); it++)
    // {
    //     std::cout<<"file: "<<*it<<std::endl;
    // }
    //
    // //debug
    // for(auto it=allSubDirs.begin(); it != allSubDirs.end(); it++)
    // {
    //     std::cout<<"dir: "<<*it<<std::endl;
    // }

    //find the (master) .vex file (should be unique)
    fVexFile = "";
    std::vector< std::string > tmpFiles;
    fDirInterface.GetFilesMatchingExtention(tmpFiles, "vex");
    if(tmpFiles.size() != 1)
    {
        fVexFile = ""; //no vex file (or non-unique vex file)
    }
    else
    {
        fVexFile = tmpFiles[0];
    }
    tmpFiles.clear();
    
    //if we didn't find a 'vex' file, look for 'vex.obs', since this convention is sometimes used 
    //by the correlator
    if(fVexFile == "")
    {
        fDirInterface.GetFilesMatchingExtention(tmpFiles, "vex.obs");
        if(tmpFiles.size() != 1)
        {
            fVexFile = ""; //no vex file (or non-unique vex file)
        }
        else
        {
            fVexFile = tmpFiles[0];
        }
        tmpFiles.clear();
    }
    
    if(fVexFile == "")
    {
        msg_error("difx_interface", "unable to determine/locate the .vex or .vex.obs file in " << input_dir << eom);
    }

    //find the .v2d file (should be unique)
    fDirInterface.GetFilesMatchingExtention(tmpFiles, "v2d");
    if(tmpFiles.size() != 1)
    {
        msg_info("difx_interface", tmpFiles.size() << " .v2d files found in " << input_dir << eom);
        fV2DFile = ""; //no v2d file
    }
    else
    {
        fV2DFile = tmpFiles[0];
    }
    tmpFiles.clear();

    //find the .input files
    std::vector< std::string > inputFiles;
    fDirInterface.GetFilesMatchingExtention(inputFiles, "input");
    std::sort(inputFiles.begin(), inputFiles.end());

    //find the DiFX name for each scan (should be unique), derive this from the .input file list
    std::vector< std::string > scanNames;
    for(auto it = inputFiles.begin(); it != inputFiles.end(); it++)
    {
        //strip off extension
        std::string basename = fDirInterface.GetBasename(*it);
        std::string scan_name = fDirInterface.StripExtensionFromBasename(basename);
        if(scan_name.size() != 0)
        {
            scanNames.push_back(scan_name);
        }
    }
    std::sort(scanNames.begin(), scanNames.end());

    if(scanNames.size() == 0)
    {
        msg_fatal("difx_interface", "No scan input found under: " << input_dir << eom);
        std::exit(1);
    }

    //find the .im files
    std::vector< std::string > imFiles;
    std::map< std::string, bool > imPresent;
    fDirInterface.GetFilesMatchingExtention(imFiles, "im");
    for(auto it = imFiles.begin(); it != imFiles.end(); it++)
    {
        imPresent[*it] = true;
    }

    //find the .calc files
    std::vector< std::string > calcFiles;
    std::map< std::string, bool > calcPresent;
    fDirInterface.GetFilesMatchingExtention(calcFiles, "calc");
    for(auto it = calcFiles.begin(); it != calcFiles.end(); it++)
    {
        calcPresent[*it] = true;
    }

    //find the .flag files
    std::vector< std::string > flagFiles;
    std::map< std::string, bool > flagPresent;
    fDirInterface.GetFilesMatchingExtention(flagFiles, "flag");
    for(auto it = flagFiles.begin(); it != flagFiles.end(); it++)
    {
        flagPresent[*it] = true;
    }

    //grab all of the .difx directories
    std::vector< std::string > difxDirs;
    std::map< std::string, bool > difxPresent;
    fDirInterface.GetSubDirectoriesMatchingExtention(difxDirs, "difx");
    for(auto it = difxDirs.begin(); it != difxDirs.end(); it++)
    {
        difxPresent[*it] = true;
    }

    //now construct the scan file sets for each input
    fScanFileSetList.clear();
    std::size_t scan_count = 0;
    for(auto it = scanNames.begin(); it != scanNames.end(); it++)
    {
        //debug
        msg_debug("difx_interface", "constructing file-set for scan: " << *it << eom);

        std::string input_file = input_dir + "/" + *it + ".input";
        std::string im_file = input_dir + "/" + *it + ".im";
        std::string calc_file = input_dir + "/" + *it + ".calc";
        std::string flag_file = input_dir + "/" + *it + ".flag";
        std::string difx_dir = input_dir + "/" + *it + ".difx";

        // std::cout<<"input = "<<input_file<<std::endl;
        // std::cout<<"im = "<<im_file<<std::endl;
        // std::cout<<"calc = "<<calc_file<<std::endl;
        // std::cout<<"flag = "<<flag_file<<std::endl;
        // std::cout<<"difx = "<<difx_dir<<std::endl;

        //verify each is present
        bool have_full_set = true;
        auto im = imPresent.find(im_file);
        if(im == imPresent.end())
        {
            have_full_set = false;
        }
        auto calc = calcPresent.find(calc_file);
        if(calc == calcPresent.end())
        {
            have_full_set = false;
        }
        auto flag = flagPresent.find(flag_file);
        if(flag == flagPresent.end())
        {
            have_full_set = false;
        }
        auto difx = difxPresent.find(difx_dir);
        if(difx == difxPresent.end())
        {
            have_full_set = false;
        }

        if(have_full_set)
        {
            MHO_DiFXScanFileSet fileSet;

            fileSet.fIndex = scan_count;
            fileSet.fScanName = *it;
            fileSet.fInputBaseDirectory = input_dir;
            fileSet.fOutputBaseDirectory = fOutputDirectory;
            fileSet.fScanDirectory = difx_dir;
            fileSet.fInputFile = input_file;
            fileSet.fIMFile = im_file;
            fileSet.fCalcFile = calc_file;
            fileSet.fFlagFile = flag_file;
            fileSet.fV2DFile = fV2DFile;
            fileSet.fVexFile = fVexFile;

            fileSet.fVisibilityFileList.clear();
            fileSet.fPCALFileList.clear();

            MHO_DirectoryInterface subDirInterface;
            subDirInterface.SetCurrentDirectory(difx_dir);
            subDirInterface.ReadCurrentDirectory();

            //locate Swinburne file
            std::vector< std::string > visibFiles;
            subDirInterface.GetFilesMatchingPrefix(visibFiles, "DIFX_");
            for(auto it = visibFiles.begin(); it != visibFiles.end(); it++)
            {
                fileSet.fVisibilityFileList.push_back(*it);
            }

            //locate the pcal files
            std::vector< std::string > pcalFiles;
            subDirInterface.GetFilesMatchingPrefix(pcalFiles, "PCAL_");
            for(auto it = pcalFiles.begin(); it != pcalFiles.end(); it++)
            {
                fileSet.fPCALFileList.push_back(*it);
            }

            if(fileSet.fVisibilityFileList.size() != 0)
            {
                fScanFileSetList.push_back(fileSet);
            }
            else
            {
                msg_warn("difx_interface",
                         "No visibility files found associated with scan: " << *it << " will not process." << eom);
            }
        }
        else
        {
            msg_warn("difx_interface",
                     "Could not find all difx files associated with scan: " << *it << " will not process." << eom);
        }
        scan_count++;
    }

    if(fScanFileSetList.size() == 0)
    {
        msg_fatal("difx_interface", "no complete scan input found under: " << input_dir << eom);
        std::exit(1);
    }
}

void MHO_DiFXInterface::InitializeFromScanDir(const std::string& input_dir)
{
    //directory interface
    MHO_DirectoryInterface dirInterface;
    dirInterface.SetCurrentDirectory(input_dir);
    std::string parent_directory = dirInterface.GetCurrentParentDirectory();
    
    //initialize from the parent directory
    InitializeFromExperimentDir(parent_directory);
    std::vector< MHO_DiFXScanFileSet > tmpScanList;
    //now strip out all scans but the one matching the original input directory
    for(auto it = fScanFileSetList.begin(); it != fScanFileSetList.end(); it++)
    {
        if(it->fScanDirectory == input_dir)
        {
            it->fIndex = 0; //reset the scan index to zero
            tmpScanList.push_back(*it);
            break;
        }
    }
    fScanFileSetList.clear();
    fScanFileSetList = tmpScanList;
}


void MHO_DiFXInterface::Initialize()
{
    //check if the input directory ends with ".difx", if that is the case, assume 
    //we are converting a single scan
    if( IsSingleScan(fInputDirectory) )
    {
        msg_info("difx_interface", "processing the single scan found in: "<< fInputDirectory << eom);
        InitializeFromScanDir(fInputDirectory);
    }
    else 
    {
        msg_info("difx_interface", "processing the whole experiment found in: "<< fInputDirectory << eom);
        InitializeFromExperimentDir(fInputDirectory);
    }
}

void MHO_DiFXInterface::ProcessScans()
{
    //generate all of the root codes for the incoming scans
    MHO_LegacyRootCodeGenerator rcode_gen;
    std::vector< std::string > scan_codes = rcode_gen.GetCodes(fScanFileSetList.size());

    for(std::size_t i = 0; i < fScanFileSetList.size(); i++)
    {
        msg_debug("difx_interface",
                  "processing scan: " << fScanFileSetList[i].fScanName << " and assigning root code: " << scan_codes[i] << eom);
        fScanProcessor.SetExperimentNumber(fExperNum);
        fScanProcessor.SetRootCode(scan_codes[i]);
        fScanProcessor.SetNormalizeFalse();
        if(fNormalize)
        {
            fScanProcessor.SetNormalizeTrue();
        }

        if(fPreserveDiFXScanNames)
        {
            fScanProcessor.SetPreserveDiFXScanNamesTrue();
        }
        else
        {
            fScanProcessor.SetPreserveDiFXScanNamesFalse();
        }

        if(fFreqBands.size() != 0)
        {
            fScanProcessor.SetFrequencyBands(fFreqBands);
        }
        if(fFreqGroups.size() != 0)
        {
            fScanProcessor.SetFreqGroups(fFreqGroups);
        }
        if(fSelectByBandwidth)
        {
            fScanProcessor.SetOnlyBandwidth(fOnlyBandwidth);
        }

        fScanProcessor.ProcessScan(fScanFileSetList[i]);
    }
}


bool 
MHO_DiFXInterface::IsSingleScan(const std::string& input_dir) const
{
    std::string scanExt = ".difx";
    std::size_t index = input_dir.find(scanExt);
    if(index != std::string::npos)
    {
        //make sure the extension is the very end of the string
        std::string sub = input_dir.substr(index);
        if(sub == scanExt || sub == scanExt + "/" )
        {
            return true;
        }
    }
    return false;
}


} // namespace hops
