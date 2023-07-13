#include "MHO_DiFXInterface.hh"

namespace hops
{

MHO_DiFXInterface::MHO_DiFXInterface():
    fInputDirectory(""),
    fOutputDirectory("")
{
    fNormalize = false;
    fPreserveDiFXScanNames = false;
};

MHO_DiFXInterface::~MHO_DiFXInterface(){};

void
MHO_DiFXInterface::SetInputDirectory(std::string dir)
{
    fInputDirectory = fDirInterface.GetDirectoryFullPath(dir);
}

void
MHO_DiFXInterface::SetOutputDirectory(std::string dir)
{
    fOutputDirectory = fDirInterface.GetDirectoryFullPath(dir);
}

void MHO_DiFXInterface::SetStationCodes(MHO_StationCodeMap* code_map)
{
    fScanProcessor.SetStationCodes(code_map);
};


void 
MHO_DiFXInterface::Initialize()
{
    //directory interface
    MHO_DirectoryInterface fDirInterface;
    bool in_ok = fDirInterface.DoesDirectoryExist(fInputDirectory);
    bool out_ok = fDirInterface.DoesDirectoryExist(fOutputDirectory);

    msg_info("difx_interface", "input directory: " << fInputDirectory << eom);
    msg_info("difx_interface", "output directory: " << fOutputDirectory << eom);

    //get list of all the files (and directories) in directory
    std::vector< std::string > allFiles;
    std::vector< std::string > allSubDirs;

    fDirInterface.SetCurrentDirectory(fInputDirectory);
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
    std::vector< std::string > tmpFiles;
    fDirInterface.GetFilesMatchingExtention(tmpFiles, "vex");
    if(tmpFiles.size() != 1)
    {
        msg_fatal("difx_interface", tmpFiles.size() << " .vex files found in " << fInputDirectory << eom );
        std::exit(1);
    }
    fVexFile = tmpFiles[0];
    tmpFiles.clear();

    //find the .v2d file (should be unique)
    fDirInterface.GetFilesMatchingExtention(tmpFiles, "v2d");
    if(tmpFiles.size() != 1)
    {
        msg_info("difx_interface", tmpFiles.size() << " .v2d files found in " << fInputDirectory << eom );
        fV2DFile = ""; //no v2d file
    }
    fV2DFile = tmpFiles[0];
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
        std::string scan_name =  fDirInterface.StripExtensionFromBasename(basename);
        if(scan_name.size() != 0){scanNames.push_back(scan_name);}
    }
    std::sort(scanNames.begin(), scanNames.end());

    if(scanNames.size() == 0)
    {
        msg_fatal("difx_interface", "No scan input found under: " << fInputDirectory << eom );
        std::exit(1);
    }

    //find the .im files
    std::vector< std::string > imFiles;
    std::map< std::string, bool > imPresent;
    fDirInterface.GetFilesMatchingExtention(imFiles, "im");
    for(auto it = imFiles.begin(); it != imFiles.end(); it++){imPresent[*it] = true;}

    //find the .calc files
    std::vector< std::string > calcFiles;
    std::map< std::string, bool > calcPresent;
    fDirInterface.GetFilesMatchingExtention(calcFiles, "calc");
    for(auto it = calcFiles.begin(); it != calcFiles.end(); it++){calcPresent[*it] = true;}
    
    //find the .flag files
    std::vector< std::string > flagFiles;
    std::map< std::string, bool> flagPresent;
    fDirInterface.GetFilesMatchingExtention(flagFiles, "flag");
    for(auto it = flagFiles.begin(); it != flagFiles.end(); it++){flagPresent[*it] = true;}
    
    //grab all of the .difx directories 
    std::vector< std::string > difxDirs;
    std::map< std::string, bool > difxPresent;
    fDirInterface.GetSubDirectoriesMatchingExtention(difxDirs, "difx");
    for(auto it = difxDirs.begin(); it != difxDirs.end(); it++){difxPresent[*it] = true;}

    //now construct the scan file sets for each input 
    fScanFileSetList.clear();
    std::size_t scan_count = 0;
    for(auto it=scanNames.begin(); it != scanNames.end(); it++)
    {
        //debug
        msg_debug("difx_interface", "constructing file-set for scan: " << *it << eom);

        std::string input_file = fInputDirectory + "/" + *it + ".input";
        std::string im_file = fInputDirectory + "/" + *it + ".im";
        std::string calc_file = fInputDirectory + "/" + *it + ".calc";
        std::string flag_file = fInputDirectory + "/" + *it + ".flag";
        std::string difx_dir = fInputDirectory + "/" + *it + ".difx";

        // std::cout<<"input = "<<input_file<<std::endl;
        // std::cout<<"im = "<<im_file<<std::endl;
        // std::cout<<"calc = "<<calc_file<<std::endl;
        // std::cout<<"flag = "<<flag_file<<std::endl;
        // std::cout<<"difx = "<<difx_dir<<std::endl;

        //verify each is present 
        bool have_full_set = true;
        auto im = imPresent.find(im_file); if(im == imPresent.end()){have_full_set = false;}
        auto calc = calcPresent.find(calc_file); if(calc == calcPresent.end()){have_full_set = false;}
        auto flag = flagPresent.find(flag_file); if(flag == flagPresent.end()){have_full_set = false;}
        auto difx = difxPresent.find(difx_dir); if(difx == difxPresent.end()){have_full_set = false;}

        if(have_full_set)
        {
            MHO_DiFXScanFileSet fileSet;
            
            fileSet.fIndex = scan_count;
            fileSet.fScanName = *it;
            fileSet.fInputBaseDirectory = fInputDirectory;
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
            for(auto it=visibFiles.begin(); it != visibFiles.end(); it++)
            {
                fileSet.fVisibilityFileList.push_back(*it);
            }

            //locate the pcal files 
            std::vector< std::string > pcalFiles;
            subDirInterface.GetFilesMatchingPrefix(pcalFiles, "PCAL_");
            for(auto it=pcalFiles.begin(); it != pcalFiles.end(); it++)
            {
                fileSet.fPCALFileList.push_back(*it);
            }

            if(fileSet.fVisibilityFileList.size() != 0)
            {
                fScanFileSetList.push_back(fileSet);
            }
            else 
            {
                msg_warn("difx_interface", "No visibility files found associated with scan: " << *it << " will not process." << eom);
            }
        }
        else 
        {
            msg_warn("difx_interface", "Could not find all difx files associated with scan: " << *it << " will not process." << eom);
        }
        scan_count++;
    }

    if(fScanFileSetList.size() == 0)
    {
        msg_fatal("difx_interface", "No complete scan input found under: " << fInputDirectory << eom );
        std::exit(1);
    }
}

void 
MHO_DiFXInterface::ProcessScans()
{
    //generate all of the root codes for the incoming scans  
    MHO_LegacyRootCodeGenerator rcode_gen;
    std::vector< std::string > scan_codes = rcode_gen.GetCodes( fScanFileSetList.size() );

    for(std::size_t i=0; i<fScanFileSetList.size(); i++)
    {
        msg_debug("difx_interface", "processing scan: "<< fScanFileSetList[i].fScanName << " and assigning root code: "<< scan_codes[i] << eom);
        fScanProcessor.SetExperimentNumber(fExperNum);
        fScanProcessor.SetRootCode(scan_codes[i]);
        fScanProcessor.SetNormalizeFalse();
        if(fNormalize){fScanProcessor.SetNormalizeTrue();}
        
        if(fPreserveDiFXScanNames){ fScanProcessor.SetPreserveDiFXScanNamesTrue();}
        else{ fScanProcessor.SetPreserveDiFXScanNamesFalse(); }

        fScanProcessor.ProcessScan(fScanFileSetList[i]);
    }
}


}//end of namespace
