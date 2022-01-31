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

    //grab all of the .difx directories 
    std::vector< std::string > difxDirs;
    fDirInterface.GetSubDirectoriesMatchingExtention(difxDirs, "difx");

    if(difxDirs.size() == 0)
    {
        msg_fatal("difx_interface", "No .difx directories found under: " << fInputDirectory << eom );
        std::exit(1);
    }

    for(auto it=difxDirs.begin(); it != difxDirs.end(); it++)
    {
        std::cout<<"difx sub-dir: "<<*it<<std::endl;
    }

    //super primitive right now...just assume only a single DIFX_ file 
    MHO_DirectoryInterface subDirInterface;
    subDirInterface.SetCurrentDirectory(*(difxDirs.begin()));
    subDirInterface.ReadCurrentDirectory();

    //locate the visiblity file 
    std::vector< std::string > visibFiles;
    subDirInterface.GetFilesMatchingPrefix(visibFiles, "DIFX_");
    for(auto it=visibFiles.begin(); it != visibFiles.end(); it++)
    {
        std::cout<<"visib file: "<<*it<<std::endl;
    }

    
}

void 
MHO_DiFXInputInterface::ReadDIFX_File(std::string filename)
{
    //read the visibilities and allocate memory to store them as we go

}

void 
MHO_DiFXInputInterface::ReadPCAL_File(std::string filename)
{
    //TODO 
}

void 
MHO_DiFXInputInterface::ReadIM_File(std::string filename)
{
    //TODO
}

void 
MHO_DiFXInputInterface::ReadInputFile(std::string filename)
{
    //TODO
}










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
//         vfile_status = fread (&pv->sync, sizeof (int), 1, vfile);
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
//         if (pv->sync == VISRECORD_SYNC_WORD_DIFX1) //old style ascii header
//             {
//             fprintf(stderr, "Error: difx2mark4 will not work with DiFX 1.x data\n");
//             return -3;
//             }
//         else if (pv->sync == VISRECORD_SYNC_WORD_DIFX2) //new style binary header
//             {
//             fread (&pv->version, sizeof (int), 1, vfile);
//             if(pv->version == 1) //new style binary header
//                 {
//                 fread (&pv->baseline,     sizeof (int),    1, vfile);
//                 fread (&pv->mjd,          sizeof (int),    1, vfile);
//                 fread (&pv->iat,          sizeof (double), 1, vfile);
//                 fread (&pv->config_index, sizeof (int),    1, vfile);
//                 fread (&pv->source_index, sizeof (int),    1, vfile);
//                 fread (&pv->freq_index,   sizeof (int),    1, vfile);
//                 fread (pv->pols,          sizeof (char),   2, vfile);
//                 fread (&pv->pulsar_bin,   sizeof (int),    1, vfile);
//                 fread (&pv->weight,       sizeof (double), 1, vfile);
//                 fread (pv->uvw,           sizeof (double), 3, vfile);
// 
//                                     // determine #vis from input tables
//                 pfr = D->freq + pv->freq_index;
//                 nvis[nvr] = pfr->nChan / pfr->specAvg;
//                                     // protect from array overrun
//                 if (nvis[nvr] > MAX_VIS) 
//                     {
//                     fprintf (stderr, 
//                     "fatal error: # visibilities (%d) exceeds array dimension (%d)\n",
//                     nvis, MAX_VIS);
//                     return (-7);
//                     }
//                 vrsize[nvr] = sizeof (vis_record) - sizeof (pv->comp)
//                                  + nvis[nvr] * 2 * sizeof (float);
//                 fread (pv->comp,          sizeof (float),  2*nvis[nvr], vfile);
// 
//                                     // if baseline not in fblock - skip over data record
//                 ipfb = get_pfb_index (pv->baseline, pv->freq_index, pfb);
//                 if (ipfb < 0)
//                     {
//                     if (opts->verbose > 2)
//                         fprintf (stderr, "Skipping data for index %d of baseline %d\n",
//                                   pv->freq_index, pv->baseline);
//                     nskip++;
//                     continue;
//                     }   
//                 if (opts->verbose > 2)
//                     fprintf (stderr, "valid read bl %x time %d %13.6f %p config %d source %d "
//                                      "freq %d, pol %c%c pb %d\n",
//                     pv->baseline, pv->mjd, pv->iat, &(pv->iat),pv->config_index,
//                     pv->source_index, pv->freq_index, pv->pols[0], pv->pols[1], pv->pulsar_bin);
//                 }
//             else
//                 {
//                 fprintf(stderr, "Error parsing Swinburne header: got a sync of %x and version"
//                         " of %d in record %d\n", pv->sync, pv->version, nvr);
//                 return -4;
//                 }
//             }
//         else
//             {
//             fprintf (stderr, "Error parsing Swinburne header: got an unrecognized sync"
//                     " of %x in record %d\n", pv->sync, nvr);
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