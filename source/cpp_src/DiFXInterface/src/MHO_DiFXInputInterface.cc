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

    DifxVisRecord visRecord;

    // #define VISRECORD_SYNC_WORD_DIFX1 //old ascii, unsupported
    // #define VISRECORD_SYNC_WORD_DIFX2

    std::fstream vFile;
    //open file for binary reading
    vFile.open(filename.c_str(), std::fstream::in | std::ios::binary);
    if( !vFile.is_open() || !vFile.good() )
    {
        msg_error("file", "Failed to open visibility file: "  << filename << " for reading." << eom);
    }

    while(true)
    {
        vFile.read(&(visRecord.sync) ), sizeof(int) ) ;

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
            msg_info("difx_interface", "Reading a DiFX binary file. " << eom );
            
            vFile.read(&(visRecord.version) ), sizeof(int) ) ;
            //fread (&visRecord.version, sizeof (int), 1, vfile);
            if(visRecord.version == 1) //new style binary header
            {
                vFile.read(&visRecord.baseline,     sizeof(int) );
                vFile.read(&visRecord.mjd,          sizeof(int) );
                vFile.read(&visRecord.iat,          sizeof(double) );
                vFile.read(&visRecord.config_index, sizeof(int) );
                vFile.read(&visRecord.source_index, sizeof(int) );
                vFile.read(&visRecord.freq_index,   sizeof(int) ); 
                vFile.read(visRecord.pols,          3*sizeof(char) );
                vFile.read(&visRecord.pulsar_bin,   sizeof(int) ); 
                vFile.read(&visRecord.weight,       sizeof(double) );
                vFile.read(visRecord.uvw,           3*sizeof(double) );


                std::size_t nchans = 1;
                //figure out the number of values here


                vFile.read(&(visRecord.visdata), nchans*sizeof(cplx32f) );



                // // if baseline not in fblock - skip over data record
                // ipfb = get_pfb_index (visRecord.baseline, visRecord.freq_index, pfb);
                // if (ipfb < 0)
                // {
                //     if (opts->verbose > 2)
                //     fprintf (stderr, "Skipping data for index %d of baseline %d\n",
                //     visRecord.freq_index, visRecord.baseline);
                //     nskip++;
                //     continue;
                // }   

                // if (opts->verbose > 2)
                // fprintf (stderr, "valid read bl %x time %d %13.6f %p config %d source %d "
                // "freq %d, pol %c%c pb %d\n",
                // visRecord.baseline, visRecord.mjd, visRecord.iat, &(visRecord.iat),visRecord.config_index,
                // visRecord.source_index, visRecord.freq_index, visRecord.pols[0], visRecord.pols[1], visRecord.pulsar_bin);
            // }
            // else
            // {
            //     fprintf(stderr, "Error parsing Swinburne header: got a sync of %x and version"
            //     " of %d in record %d\n", visRecord.sync, visRecord.version, nvr);
            //     return -4;
            //         //     }


        }
        break;
    }

    vFile.close();

        //     fread (&visRecord.version, sizeof (int), 1, vfile);
        //     if(visRecord.version == 1) //new style binary header
        //     {
        //         fread (&visRecord.baseline,     sizeof (int),    1, vfile);
        //         fread (&visRecord.mjd,          sizeof (int),    1, vfile);
        //         fread (&visRecord.iat,          sizeof (double), 1, vfile);
        //         fread (&visRecord.config_index, sizeof (int),    1, vfile);
        //         fread (&visRecord.source_index, sizeof (int),    1, vfile);
        //         fread (&visRecord.freq_index,   sizeof (int),    1, vfile);
        //         fread (visRecord.pols,          sizeof (char),   2, vfile);
        //         fread (&visRecord.pulsar_bin,   sizeof (int),    1, vfile);
        //         fread (&visRecord.weight,       sizeof (double), 1, vfile);
        //         fread (visRecord.uvw,           sizeof (double), 3, vfile);
        // 
        //         // determine #vis from input tables
        //         pfr = D->freq + visRecord.freq_index;
        //         nvis[nvr] = pfr->nChan / pfr->specAvg;
        //         // protect from array overrun
        //         if (nvis[nvr] > MAX_VIS) 
        //         {
        //             fprintf (stderr, 
        //             "fatal error: # visibilities (%d) exceeds array dimension (%d)\n",
        //             nvis, MAX_VIS);
        //             return (-7);
        //         }
        //         vrsize[nvr] = sizeof (vis_record) - sizeof (visRecord.comp)
        //         + nvis[nvr] * 2 * sizeof (float);
        //         fread (visRecord.comp,          sizeof (float),  2*nvis[nvr], vfile);
        // 
        //         // if baseline not in fblock - skip over data record
        //         ipfb = get_pfb_index (visRecord.baseline, visRecord.freq_index, pfb);
        //         if (ipfb < 0)
        //         {
        //             if (opts->verbose > 2)
        //             fprintf (stderr, "Skipping data for index %d of baseline %d\n",
        //             visRecord.freq_index, visRecord.baseline);
        //             nskip++;
        //             continue;
        //         }   
        //         if (opts->verbose > 2)
        //         fprintf (stderr, "valid read bl %x time %d %13.6f %p config %d source %d "
        //         "freq %d, pol %c%c pb %d\n",
        //         visRecord.baseline, visRecord.mjd, visRecord.iat, &(visRecord.iat),visRecord.config_index,
        //         visRecord.source_index, visRecord.freq_index, visRecord.pols[0], visRecord.pols[1], visRecord.pulsar_bin);
        //     }
        //     else
        //     {
        //         fprintf(stderr, "Error parsing Swinburne header: got a sync of %x and version"
        //         " of %d in record %d\n", visRecord.sync, visRecord.version, nvr);
        //         return -4;
        //     }
        // }
        // else
        // {
        //     fprintf (stderr, "Error parsing Swinburne header: got an unrecognized sync"
        //     " of %x in record %d\n", visRecord.sync, nvr);
        //     return -5;
        // }
        // 
        // vrsize_tot += vrsize[nvr];
        // nvr += 1;                   // bump the record counter
        // // protect from visibility array overruns
        // if (nvr > NVRMAX) 
        // {
        //     fprintf (stderr, 
        //     "fatal error: # visibility records (%d) exceeds array dimension (%d)\n",
        //     nvr, NVRMAX);
        //     return (-8);
        // }
        // // point to next record
        // pch = (char *) *vrec + vrsize_tot;
        // pv = (vis_record *) pch;
        // // if necessary, get another chunk's worth of ram
        // // trigger realloc when less than half of the
        // // current chunk remains
        // if (allocated_tot - vrsize_tot < 0.5 * CHUNK)
        // {
        //     if (opts->verbose > 1)
        //     printf ("realloc another mem chunk for visibilities, nvr %d size %d bytes\n",
        //     nvr, (int) CHUNK);
        // 
        //     allocated_tot += CHUNK;
        //     *vrec = realloc (*vrec, (size_t) allocated_tot);
        //     if (*vrec == NULL)
        //     {
        //         printf ("error reallocating memory for %d records, requested %zu bytes\n",
        //         nvr, allocated_tot);
        //         return -2;
        //     }
        //     // recalculate pointers based on reallocated memory
        //     pch = (char *) *vrec + vrsize_tot;
        //     pv = (vis_record *) pch;
        // }
    // }
    // if (opts->verbose > 1 && nskip > 0)
    // fprintf (stderr, "total Swinburne records skipped %d\n", nskip);


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










// 
// 
// int get_vis (DifxInput *D,                    // ptr to difx input file data
//              char *vf_name,                   // name of input file
//              struct CommandLineOptions *opts, // ptr to input options
//              int *nvrtot,                     // total number of vis. records read
//              int *nvis,                       // array of #vis per record
//              int *vrsize,                     // array of size of vis records in bytes
//              vis_record **vrec,               // ptr to malloced array of vis. recs as read
//              char *corrdate,                  // modification date of input file
//              struct fblock_tag *pfb)          // ptr to filled-in fblock table
//     {
//     int err,
//         vfile_status,
//         nskip=0,
//         nvr,
//         ipfb;
//     size_t vrsize_tot,
//         allocated_tot;
// 
//     FILE *vfile;
//     struct tm *mod_time;
//     struct stat attrib;
//     vis_record *pv;                 // convenience pointers
//     char *pch;
//     DifxFreq *pfr;
//                                     // local function prototypes
//     int get_pfb_index (int, int, struct fblock_tag *);
// 
//     vfile = fopen (vf_name, "r");
//     if (vfile == NULL)
//         {
//         perror (vf_name);
//         fprintf (stderr, "fatal error opening input data file %s\n", vf_name);
//         return (-1);
//         }
// 
//     printf ("      opened input file %s\n", vf_name);
//     err = stat (vf_name, &attrib);
//     if (err)
//         {
//         fprintf (stderr, "Warning: error stating file %s\n", vf_name);
//         fprintf (stderr, "         t000.date will be set to 2000001-000000\n");
//         sprintf (corrdate, "2000001-000000");
//         }
//     else
//         {
//         mod_time = gmtime (&(attrib.st_mtime));
//         snprintf (corrdate, 16, "%4d%03d-%02d%02d%02d", 
//                  mod_time->tm_year+1900,
//                  mod_time->tm_yday+1, mod_time->tm_hour,
//                  mod_time->tm_min,  mod_time->tm_sec);
//         }
// 
//     *vrec = malloc (CHUNK);
//     allocated_tot = CHUNK;          // total amount of memory allocated
//     pv = *vrec;
//     pch = (char *) pv;
//     nvr = 0;
//     vrsize_tot = 0;
//                                     // loop over all records in file
//     while (TRUE)
//         {
//                                     // read a header from the input file
//                                     // first read sync word to identify header version
//         vfile_status = fread (&visRecord.sync, sizeof (int), 1, vfile);
//         if (vfile_status != 1)
//             {
//             if (feof (vfile))
//                 {
//                                     //EOF in .difx file
//                 if (opts->verbose > 0)
//                     printf ("        EOF in input file\n");
//                 fclose (vfile);
//                 *nvrtot = nvr;
//                 return -1;
//                 }
//             else 
//                 {
//                 fprintf (stderr, "unreadable input file %s status %d\n",
//                          vf_name, vfile_status);
//                                     //unreadable .difx file
//                 fclose (vfile);
//                 return -2;
//                 }
//             }
// 
//         if (visRecord.sync == VISRECORD_SYNC_WORD_DIFX1) //old style ascii header
//             {
//             fprintf(stderr, "Error: difx2mark4 will not work with DiFX 1.x data\n");
//             return -3;
//             }
//         else if (visRecord.sync == VISRECORD_SYNC_WORD_DIFX2) //new style binary header
//             {
//             fread (&visRecord.version, sizeof (int), 1, vfile);
//             if(visRecord.version == 1) //new style binary header
//                 {
//                 fread (&visRecord.baseline,     sizeof (int),    1, vfile);
//                 fread (&visRecord.mjd,          sizeof (int),    1, vfile);
//                 fread (&visRecord.iat,          sizeof (double), 1, vfile);
//                 fread (&visRecord.config_index, sizeof (int),    1, vfile);
//                 fread (&visRecord.source_index, sizeof (int),    1, vfile);
//                 fread (&visRecord.freq_index,   sizeof (int),    1, vfile);
//                 fread (visRecord.pols,          sizeof (char),   2, vfile);
//                 fread (&visRecord.pulsar_bin,   sizeof (int),    1, vfile);
//                 fread (&visRecord.weight,       sizeof (double), 1, vfile);
//                 fread (visRecord.uvw,           sizeof (double), 3, vfile);
// 
//                                     // determine #vis from input tables
//                 pfr = D->freq + visRecord.freq_index;
//                 nvis[nvr] = pfr->nChan / pfr->specAvg;
//                                     // protect from array overrun
//                 if (nvis[nvr] > MAX_VIS) 
//                     {
//                     fprintf (stderr, 
//                     "fatal error: # visibilities (%d) exceeds array dimension (%d)\n",
//                     nvis, MAX_VIS);
//                     return (-7);
//                     }
//                 vrsize[nvr] = sizeof (vis_record) - sizeof (visRecord.comp)
//                                  + nvis[nvr] * 2 * sizeof (float);
//                 fread (visRecord.comp,          sizeof (float),  2*nvis[nvr], vfile);
// 
//                                     // if baseline not in fblock - skip over data record
//                 ipfb = get_pfb_index (visRecord.baseline, visRecord.freq_index, pfb);
//                 if (ipfb < 0)
//                     {
//                     if (opts->verbose > 2)
//                         fprintf (stderr, "Skipping data for index %d of baseline %d\n",
//                                   visRecord.freq_index, visRecord.baseline);
//                     nskip++;
//                     continue;
//                     }   
//                 if (opts->verbose > 2)
//                     fprintf (stderr, "valid read bl %x time %d %13.6f %p config %d source %d "
//                                      "freq %d, pol %c%c pb %d\n",
//                     visRecord.baseline, visRecord.mjd, visRecord.iat, &(visRecord.iat),visRecord.config_index,
//                     visRecord.source_index, visRecord.freq_index, visRecord.pols[0], visRecord.pols[1], visRecord.pulsar_bin);
//                 }
//             else
//                 {
//                 fprintf(stderr, "Error parsing Swinburne header: got a sync of %x and version"
//                         " of %d in record %d\n", visRecord.sync, visRecord.version, nvr);
//                 return -4;
//                 }
//             }
//         else
//             {
//             fprintf (stderr, "Error parsing Swinburne header: got an unrecognized sync"
//                     " of %x in record %d\n", visRecord.sync, nvr);
//             return -5;
//             }
// 
//         vrsize_tot += vrsize[nvr];
//         nvr += 1;                   // bump the record counter
//                                     // protect from visibility array overruns
//         if (nvr > NVRMAX) 
//             {
//             fprintf (stderr, 
//             "fatal error: # visibility records (%d) exceeds array dimension (%d)\n",
//             nvr, NVRMAX);
//             return (-8);
//             }
//                                     // point to next record
//         pch = (char *) *vrec + vrsize_tot;
//         pv = (vis_record *) pch;
//                                     // if necessary, get another chunk's worth of ram
//                                     // trigger realloc when less than half of the
//                                     // current chunk remains
//         if (allocated_tot - vrsize_tot < 0.5 * CHUNK)
//             {
//             if (opts->verbose > 1)
//                 printf ("realloc another mem chunk for visibilities, nvr %d size %d bytes\n",
//                         nvr, (int) CHUNK);
// 
//             allocated_tot += CHUNK;
//             *vrec = realloc (*vrec, (size_t) allocated_tot);
//             if (*vrec == NULL)
//                 {
//                 printf ("error reallocating memory for %d records, requested %zu bytes\n",
//                         nvr, allocated_tot);
//                 return -2;
//                 }
//                                     // recalculate pointers based on reallocated memory
//             pch = (char *) *vrec + vrsize_tot;
//             pv = (vis_record *) pch;
//             }
//         }
//         if (opts->verbose > 1 && nskip > 0)
//             fprintf (stderr, "total Swinburne records skipped %d\n", nskip);
//     }                               // return path is always through 
// 
// 



}//end of namespace