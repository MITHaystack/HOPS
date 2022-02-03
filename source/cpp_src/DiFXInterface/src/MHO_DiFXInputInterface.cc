#include "MHO_DiFXInputInterface.hh"

namespace hops
{

MHO_DiFXInputInterface::MHO_DiFXInputInterface():
    fInputDirectory(""),
    fOutputDirectory("")
{};

MHO_DiFXInputInterface::~MHO_DiFXInputInterface(){};

void
MHO_DiFXInputInterface::SetInputDirectory(std::string dir)
{
    fInputDirectory = fDirInterface.GetDirectoryFullPath(dir);
}

void
MHO_DiFXInputInterface::SetOutputDirectory(std::string dir)
{
    fOutputDirectory = fDirInterface.GetDirectoryFullPath(dir);
}

void 
MHO_DiFXInputInterface::Initialize()
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

    //debug
    for(auto it=allFiles.begin(); it != allFiles.end(); it++)
    {
        std::cout<<"file: "<<*it<<std::endl;
    }

    //debug
    for(auto it=allSubDirs.begin(); it != allSubDirs.end(); it++)
    {
        std::cout<<"dir: "<<*it<<std::endl;
    }

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
        msg_fatal("difx_interface", tmpFiles.size() << " .v2d files found in " << fInputDirectory << eom );
        std::exit(1);
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
    for(auto it=scanNames.begin(); it != scanNames.end(); it++)
    {
        //debug
        std::cout<<"scan: "<<*it<<std::endl;

        std::string input_file = fInputDirectory + "/" + *it + ".input";
        std::string im_file = fInputDirectory + "/" + *it + ".im";
        std::string calc_file = fInputDirectory + "/" + *it + ".calc";
        std::string flag_file = fInputDirectory + "/" + *it + ".flag";
        std::string difx_dir = fInputDirectory + "/" + *it + ".difx";

        std::cout<<"input = "<<input_file<<std::endl;
        std::cout<<"im = "<<im_file<<std::endl;
        std::cout<<"calc = "<<calc_file<<std::endl;
        std::cout<<"flag = "<<flag_file<<std::endl;
        std::cout<<"difx = "<<difx_dir<<std::endl;

        //verify each is present 
        bool have_full_set = true;
        auto im = imPresent.find(im_file); if(im == imPresent.end()){have_full_set = false;std::cout<<"1"<<std::endl;}
        auto calc = calcPresent.find(calc_file); if(calc == calcPresent.end()){have_full_set = false;std::cout<<"2"<<std::endl;}
        auto flag = flagPresent.find(flag_file); if(flag == flagPresent.end()){have_full_set = false;std::cout<<"3"<<std::endl;}
        auto difx = difxPresent.find(difx_dir); if(difx == difxPresent.end()){have_full_set = false;std::cout<<"4"<<std::endl;}

        if(have_full_set)
        {
            MHO_DiFXScanFileSet fileSet;
            
            fileSet.fScanName = *it;
            fileSet.fBaseDirectory = fInputDirectory;
            fileSet.fScanDirectory = difx_dir;
            fileSet.fInputFile = input_file;
            fileSet.fIMFile = im_file;
            fileSet.fCalcFile = calc_file;
            fileSet.fFlagFile = flag_file;
            fileSet.fV2DFile = fV2DFile;
            fileSet.fVexFile = fVexFile;

            fileSet.fVisibilityFileList.clear();
            fileSet.fPCALFileList.clear();

            //super primitive right now...just assume only a single DIFX_ file 
            MHO_DirectoryInterface subDirInterface;
            subDirInterface.SetCurrentDirectory(difx_dir);
            subDirInterface.ReadCurrentDirectory();
            
            //locate the visiblity file 
            std::vector< std::string > visibFiles;
            subDirInterface.GetFilesMatchingPrefix(visibFiles, "DIFX_");
            for(auto it=visibFiles.begin(); it != visibFiles.end(); it++)
            {
                std::cout<<"visib file: "<<*it<<std::endl;
                fileSet.fVisibilityFileList.push_back(*it);
            }

            //locate the pcal files 
            std::vector< std::string > pcalFiles;
            subDirInterface.GetFilesMatchingPrefix(pcalFiles, "PCAL_");
            for(auto it=pcalFiles.begin(); it != pcalFiles.end(); it++)
            {
                std::cout<<"pcal file: "<<*it<<std::endl;
                fileSet.fPCALFileList.push_back(*it);
            }

            if(fileSet.fVisibilityFileList.size() != 0)
            {
                fScanFileSetList.push_back(fileSet);
            }
            else 
            {
                msg_warn("difx_interface", "No visibility files found associated with scan: " << *it << eom);
            }
        }
        else 
        {
            msg_warn("difx_interface", "Could not find all difx aux files associated with scan: " << *it << eom);
        }
    }

    std::cout<<"number of scan file sets = "<<fScanFileSetList.size()<<std::endl;

    if(fScanFileSetList.size() == 0)
    {
        msg_fatal("difx_interface", "No complete scan input found under: " << fInputDirectory << eom );
        std::exit(1);
    }

    ProcessScan(fScanFileSetList[0]);

}


void 
MHO_DiFXInputInterface::ProcessScan(MHO_DiFXScanFileSet& fileSet)
{
        //testing
        ReadDIFX_File(fileSet.fVisibilityFileList[0]);
}

void 
MHO_DiFXInputInterface::ReadDIFX_File(std::string filename)
{
    //read the visibilities and allocate memory to store them as we go

    // typedef struct
    // {
    // FILE *infile;			/* file pointer */
    // DifxParameters *params;		/* structure containing text params */
    // int nchan;			/* number of channels to expect */
    // int visnum;			/* counter of number of vis */
    // int sync;			/* space to store the sync value */
    // int headerversion;		/* 0=old style, 1=new binary style */
    // int baseline;			/* The baseline number (256*A1 + A2, 1 indexed) */
    // int mjd;			/* The MJD integer day */
    // double seconds;			/* The seconds offset from mjd */
    // int configindex;		/* The index to the configuration table */
    // int sourceindex;		/* The index to the source table */
    // int freqindex;			/* The index to the freq table */
    // char polpair[3];		/* The polarisation pair */
    // int pulsarbin;			/* The pulsar bin */
    // double dataweight;		/* The fractional data weight */
    // double uvw[3];			/* The u,v,w values in metres */
    // cplx32f *visdata;		/* pointer to nchan complex values (2x float) */
    // } DifxVisRecord;

    MHO_DiFXVisibilityRecord visRecord;

    // #define VISRECORD_SYNC_WORD_DIFX1 //old ascii, unsupported
    // #define VISRECORD_SYNC_WORD_DIFX2

    std::fstream vFile;
    //open file for binary reading
    vFile.open(filename.c_str(), std::fstream::in | std::ios::binary);
    if( !vFile.is_open() || !vFile.good() )
    {
        msg_error("file", "Failed to open visibility file: "  << filename << " for reading." << eom);
    }

    std::size_t n_records = 0;
    bool keep_reading = true;
    while(keep_reading && vFile.good())
    {
        visRecord.Reset();

        vFile.read( reinterpret_cast<char*>( &(visRecord.sync) ), sizeof(int) );

        if( !(vFile.good() ) )
        {
            msg_error("difx_interface", "Could not read input file: " << filename << eom);
            break;
        }

        if (visRecord.sync == VISRECORD_SYNC_WORD_DIFX1) //old style ascii header, bad
        {
            msg_error("difx_interface", "Cannot read DiFX 1.x data. " << eom );
            break;
        }

        if(visRecord.sync == VISRECORD_SYNC_WORD_DIFX2) //new style binary header, ok
        {
            //msg_info("difx_interface", "Reading a DiFX binary file. " << eom );
            
            vFile.read( reinterpret_cast<char*>(&(visRecord.headerversion) ), sizeof(int) );
            //fread (&visRecord.version, sizeof (int), 1, vfile);
            if(visRecord.headerversion == 1) //new style binary header
            {
                vFile.read( reinterpret_cast<char*>(&visRecord.baseline), sizeof(int) );
                vFile.read( reinterpret_cast<char*>(&visRecord.mjd), sizeof(int) );
                vFile.read( reinterpret_cast<char*>(&visRecord.seconds), sizeof(double) );
                vFile.read( reinterpret_cast<char*>( &visRecord.configindex), sizeof(int) );
                vFile.read( reinterpret_cast<char*>(&visRecord.sourceindex), sizeof(int) );
                vFile.read( reinterpret_cast<char*>(&visRecord.freqindex), sizeof(int) ); 
                vFile.read( reinterpret_cast<char*>(visRecord.polpair), 2*sizeof(char) );
                vFile.read( reinterpret_cast<char*>(&visRecord.pulsarbin), sizeof(int) ); 
                vFile.read( reinterpret_cast<char*>(&visRecord.dataweight), sizeof(double) );
                vFile.read( reinterpret_cast<char*>(visRecord.uvw), 3*sizeof(double) );

                // std::cout<<visRecord.headerversion<<std::endl;
                // std::cout<<visRecord.baseline<<std::endl;
                // std::cout<<visRecord.mjd<<std::endl;
                // std::cout<<visRecord.seconds<<std::endl;
                // std::cout<<visRecord.configindex<<std::endl;
                // std::cout<<visRecord.freqindex<<std::endl;
                // std::cout<<visRecord.polpair[0]<<visRecord.polpair[1]<<std::endl;
                // std::cout<<visRecord.pulsarbin<<std::endl;
                // std::cout<<visRecord.dataweight<<std::endl;
                // std::cout<<visRecord.uvw[0]<<", "<<visRecord.uvw[1]<<", "<<visRecord.uvw[2]<<std::endl;
                // std::cout<<"visbilities: "<<std::endl;

                //parsel out the visibilities one at a time (for now)
                std::size_t nchans = 0;
                while(true)
                {
                    MHO_VisibilityChunk chunk;
                    vFile.read( reinterpret_cast<char*>(&chunk), sizeof(MHO_VisibilityChunk) );
                    //verify we haven't smacked into the sync word 
                    if(vFile.good())
                    {
                        if( chunk.sync_test[0] != VISRECORD_SYNC_WORD_DIFX2  && chunk.sync_test[1] != VISRECORD_SYNC_WORD_DIFX2  ) //got a good one
                        {
                            //std::cout << std::hex;
                            //std::cout << chunk.sync_test[0] <<std::endl;
                            nchans++;
                            visRecord.visdata.push_back( std::complex<float>(chunk.values[0], chunk.values[1] ) );
                        }
                        else
                        {
                            //"Lemme back up a minute..."
                            vFile.seekg( -1*sizeof(MHO_VisibilityChunk), std::ios_base::cur);
                            break;
                        }
                    }
                    else 
                    {
                        //hit the end of the file, break off here
                        keep_reading = false;
                        break;
                    }
                }
                visRecord.nchan = nchans;
                fBaselineVisibilities[visRecord.baseline].push_back(visRecord);
                n_records++;
                if(n_records%1000 == 0){std::cout<<"read "<<n_records<<" records with "<< nchans <<" chans"<<std::endl;}
            }
        }
    }

    std::cout<<"read data from "<<fBaselineVisibilities.size()<<" baselines "<<std::endl;
    for(auto it = fBaselineVisibilities.begin(); it != fBaselineVisibilities.end(); it++)
    {
        std::cout<<" baseline: "<< it->first <<" has: "<< it->second.size()<<" records"<<std::endl;
    }

    //close the Swinburne file
    vFile.close();


}

// void 
// MHO_DiFXInputInterface::ReadPCAL_File(std::string filename)
// {
//     //TODO 
// }
// 
// void 
// MHO_DiFXInputInterface::ReadIM_File(std::string filename)
// {
//     //TODO
// }
// 
// void 
// MHO_DiFXInputInterface::ReadInputFile(std::string filename)
// {
//     //TODO
// }



}//end of namespace