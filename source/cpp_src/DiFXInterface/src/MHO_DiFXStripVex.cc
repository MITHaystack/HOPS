#include "MHO_DiFXStripVex.hh"

namespace hops 
{

MHO_DiFXStripVex::MHO_DiFXStripVex(){};
MHO_DiFXStripVex::~MHO_DiFXStripVex(){};

void MHO_DiFXStripVex::SetSessionVexFile(std::string filename){fVexFile = filename;}
void MHO_DiFXStripVex::SetOutputFileName(std::string output_filename){fOutputFile = output_filename;}

void MHO_DiFXStripVex::ExtractScan()
{
    // initialize memory as necessary
    current_def[0] = 0;
    current_scan[0] = 0;
    for (i=0; i<MAX_FS; i++)        // null out sequence names
    fseq_list[i][0] = 0;
    nfs = 0;

    // create scan identifier

    // source name
    sourceId = D->scan[scanId].pointingCentreSrc;
    if (sourceId < 0 || sourceId > D->nSource)
    {
    printf("sourceId %d out of range\n", sourceId);
    return (-1);
    }

    configId = D->scan[scanId].configId;
    if (configId < 0)
    {
    printf("No config for scan %d\n", scanId);
    return (-1);
    }

    strcpy (source, D->source[sourceId].name);

    if (opts->verbose > 0)
    printf ("      source %s\n", source);
    // modify source (change . to _) name
    // for filename only
    i = 0;
    while (source[i] != 0)
    {
    if (source[i] == '.')
    source[i] = '_';
    i++;
    }


    // FIXME inname for vexfile and antlist for job antennas

    // open input vex file
    printf("      Opening vex file <%s>\n", D->job[jobId].vexFile);
    fin = fopen (D->job[jobId].vexFile, "r");
    if (fin == NULL)
    {
    perror (D->job[jobId].vexFile);
    fprintf (stderr, "fatal error opening input vex file %s for job %d\n", 
    D->job[jobId].vexFile, jobId);
    return (-1);
    }

    // create the new root file name
    sprintf(rootname, "%s/%s.%s", node, source, rcode);
    printf ("      output rootfile: %s\n", rootname);
    // open output (root) file
    fout = fopen (rootname, "w");
    if (fout == NULL)
    {
    perror (rootname);
    fprintf (stderr, "fatal error opening output file %s\n", rootname);
    fclose (fin);
    return (-1);
    }
    current_block = NO_BLOCK;




    // loop over all statements in input file
    while (fgets (line, 256, fin) != NULL)
    {
        // get all fields of this line
        strcpy (s, line);           // make a copy for (destructive) parsing
        //fprintf(stderr, "%s\n", line);
        // space-delimit any = sign
        if ((pchar = strchr (s, '=')) != NULL)
        {
            n = pchar - s;
            strcpy (pchar, " = ");
            strcpy (pchar+3, line+n+1);
        }


        for (i=0; i<50; i++)
        {
            pst[i] = strtok ((i>0) ? (char *) NULL : s, " :;\t\r\n");
            if (pst[i] == NULL)
            break;
        }
        // ensure that pointers are valid
        for (j=i; j<50; j++)
        pst[j] = "";

        // see if this is a block stmt
        match = FALSE;
        i = -1;
        while (strcmp (blocks[++i], "END_LIST"))
        if (strncmp (pst[0], blocks[i], strlen (blocks[i])) == 0)
        {
            match = TRUE;       // found it
            if (opts->verbose > 0)
            printf ("      processing %s block\n", blocks[i]);
            break;
        }

        if (match)                  // yes, it was found, change current block
        current_block = i;

        if (strncmp ("def", pst[0], strlen (pst[0])) == 0)
        strncpy (current_def, pst[1], 32);

        if (strncmp ("scan", pst[0], strlen (pst[0])) == 0)
        {
            strncpy (current_scan, pst[1], 32);
            //fprintf(stderr, "scan=%s\n", current_scan);
        }
        // remember the site ID for later
        if (strncmp (pst[0], "site_ID", 7) == 0)
        {
            strncpy (current_site, pst[2], 3);
            //fprintf(stderr, "site=%s\n", current_site);
        }

        // switch to proper context for current block
        switch (current_block)
        {
            case NO_BLOCK:          // insert ovex revision number
                //initial state
            break;
            case ANTENNA:           // fix up the axis_offset stmt, iff necessary
                ExtractAntenna(); //process antenna
            break;
            case EXPER:             // modify target_correlator
                ExtractExper(); //process exper section 
            break;
            case GLOBAL:
                ExtractGlobal(); //process global sectoin
            break;
            case MODE:              // delete all modes other than current one
                ExtractMode();
            break;
            case SCHED:             // insert fourfit reference time into sched block
                ExtractSched();
            break;

            case SCHEDULING_PARAMS: // the scheduling_params block is deleted
                line[0] = 0;
            break;

            case SITE:              // need to insert the single char site ID
                ExtractSite();//process site info
            break;

            case STATION:           // need to add ref to clock section
                ExtractStation(); //process station info
            break;

            // edit out these sections that will be re-generated
            case BBC:
            case CLOCK:
            case EOP:
            case FREQ:
            case IF:
            case SOURCE:
            case TRACKS:
            line[0] = 0;
            break;
            // nothing special needs to be done for these blocks
            // but delete for clarity's sake
            case DAS:
            case HEAD_POS:
            case PASS_ORDER:
            case PHASE_CAL_DETECT:
            case PROCEDURES:
            case ROLL:
            case SEFD:
            line[0] = 0;
            break;
            // just copy any other sections to the output
            case TAPELOG_OBS:
            default:
            break;
        }
        fputs (line, fout); 
        // detect exit of def block and clear current def
        if (strncmp (pst[0], "enddef", 6) == 0)
        current_def[0] = 0;
    }

}


void MHO_DiFXStripVex::ExtractAntenna()
{
    if (strncmp (pst[0], "axis_offset", 11) == 0 && strstr (line, "el:") == 0)
    {
        strcpy (buff, line);
        pchar = strchr (line, '=') + 2;
        strcpy (pchar, "el:");
        strcpy (pchar + 3, strchr (buff, '=') + 1);
    }
    else if (strncmp (pst[0], "antenna_motion", 14) == 0) 
        line[0] = '*';  // comment out antenna motion command,
                        // as it causes problems with vex parser
}

void MHO_DiFXStripVex::ExtractExper()
{
    if (strcmp (pst[0], "target_correlator") == 0)
    {
        strcpy (line, "    target_correlator = difx;\n");
        tarco = TRUE;
    }
    if (strcmp (pst[0], "exper_num") == 0)
    {
        sprintf (line, "    exper_num = %s;\n", node_name);
        exper_num = TRUE;
    }
    else if (strncmp (pst[0], "enddef", 6) == 0)
    {
        strcpy (line, "");
        if (tarco == FALSE)
        {
            strcpy (buff, "    target_correlator = difx;\n");
            strcat (line, buff);
            tarco = TRUE;
        }
        if (exper_num == FALSE)
        {
            sprintf (buff, "    exper_num = %s;\n", node_name);
            strcat (line, buff);
            exper_num = TRUE;
        }
        strcat (line, "  enddef;\n");
    }
    break;
}

void MHO_DiFXStripVex::ExtractGlobal()
{
    // insert a dummy EOP ref (which is fine for fourfit)
    if (strncmp (pst[0], "$GLOBAL", 7) == 0)
    {
        strcat (line, "* correlated from input file ");
        strcat (line, D->job[jobId].inputFile);
        strcat (line, "\n    ref $EOP = EOP_DIFX_INPUT;\n"); 
    }
    else if (strncmp (pst[0], "ref", 3) == 0 && strncmp (pst[1], "$SCHEDULING_PARAMS", 18) == 0)
    {
        line[0] = '*';  // comment out ref to deleted section 
    }
    else if (strncmp (pst[0], "ref", 3) == 0 && strncmp (pst[1], "$EOP", 4) == 0)
    {
        line[0] = '*';  // comment out ref to real EOP section
    }
}

void MHO_DiFXStripVex::ExtractMode()
{
    if (strncmp (pst[0], "def", 3) == 0  && strncmp (pst[1], D->scan[scanId].obsModeName, 30) != 0)
    {
        delete_mode = TRUE;
    }

    if (delete_mode)
    {
        line[0] = 0;
        // exit delete mode at enddef
        if (strncmp (pst[0], "enddef", 6) == 0)
        {
            delete_mode = FALSE;
        }
    }
    else                // mode is currently active
    {
        if (strncmp (pst[0], "ref", 3) == 0)
        {
            if (strncmp (pst[1], "$BBC", 4) == 0
            || strncmp (pst[1], "$FREQ", 5) == 0
            || strncmp (pst[1], "$IF", 3) == 0
            || strncmp (pst[1], "$TRACKS", 7) == 0
            || strncmp (pst[1], "$PHASE_CAL_DETECT", 17) == 0
            || strncmp (pst[1], "$PROCEDURES", 11) == 0
            || strncmp (pst[1], "$ROLL", 5) == 0
            || strncmp (pst[1], "$HEAD_POS", 9) == 0
            || strncmp (pst[1], "$PASS_ORDER", 11) == 0)
            {
                line[0] = 0; // delete original mode lines
            }
        }
        // synthesize new ref's at end of MODE
        else if (strncmp (pst[0], "enddef", 6) == 0)
        {
            strcpy (trax1b, "    ref $TRACKS = trax_1bit");
            strcpy (trax2b, "    ref $TRACKS = trax_2bit");
            trax1b_used = FALSE;
            trax2b_used = FALSE;

            // create list of quantization bits per antenna
            for (dstr=0; dstr<D->job[jobId].activeDatastreams; dstr++)
            {
                if ((D->job[jobId]).datastreamIdRemap)
                {
                    redstr = *((D->job[jobId]).datastreamIdRemap + dstr);
                }
                else
                {
                    redstr = dstr;
                    if (redstr < 0) continue;       // not assigned this job
                    pdds = D->datastream + redstr;
                    if ((size_t)(pdds->antennaId) >= sizeof(antbits)/sizeof(antbits[0]))
                    printf("      Programmer error: antenna Id %d exceeds hardcoded array size %zu\n", pdds->antennaId, sizeof(antbits)/sizeof(antbits[0])); // let segfault
                    antbits[pdds->antennaId] = pdds->quantBits;
                }
            }

            // insert one freq line per used antenna
            for (n = 0; n < D->nAntenna; n++)
            {
                if (D->scan[scanId].im != NULL && D->scan[scanId].im[n] != 0)
                {
                    // FIXME - generate antenna name from station
                    sprintf (antnam, ":%c%c", D->antenna[n].name[0], tolower (D->antenna[n].name[1]));
                    fprintf (fout, "    ref $FREQ = ant%02d%s;\n", n, antnam);
                    // add antenna to appropriate trax statement
                    if (antbits[n] == 1)
                    {
                        strcat (trax1b, antnam);
                        trax1b_used = TRUE;
                    }
                    else if (antbits[n] == 2)
                    {
                        strcat (trax2b, antnam);
                        trax2b_used = TRUE;
                    }
                    else
                    {
                        printf ("        Warning, no quant bits for %s\n", antnam);
                    }
                }
            }
            fprintf (fout, "    ref $BBC = bbcs;\n");
            fprintf (fout, "    ref $IF = ifs;\n");
            // only print references that are actually used
            if (trax1b_used)
            {
                fprintf (fout, "%s;\n", trax1b);
            }

            if (trax2b_used)
            {
                fprintf (fout, "%s;\n", trax2b);
            }
        }
    }
}

void MHO_DiFXStripVex::ExtractSched()
{
    // don't change $SCHED line
    if (strncmp (pst[0], "$SCHED", 6) == 0){break;}
    else if (strcmp (current_scan, D->scan[scanId].identifier) != 0)      // skip line copy if we're within the wrong scan
    {
        line[0] = 0;
        continue;
    }
    // correct scan, insert one copy of ff ref time
    else if (strncmp (pst[0], "scan", 4) == 0)
    {
        scan_found = TRUE;
        // initialize earliest/latest to extrema
        latest_start = 0;
        earliest_stop = 9999;
    }
    // Check that station is in this scan
    else if (strncmp (pst[0], "station", 7) == 0)
    {
        // parse station line to find scan intersection
        pchar = strchr (line, ':');
        itime = atoi (pchar+1);
        if (itime > latest_start)
        {
            latest_start = itime;
            pchar = strchr (pchar+1, ':');
            itime = atoi (pchar+1);
            if (itime < earliest_stop){earliest_stop = itime;}
        }

        i = isValidAntenna(D, pst[2], scanId);
        if(i < 0)
        {
            line[0] = 0;
            // this station participates, use difx start
        }
        else 
        {
            nant++;
            stns[i].inscan = TRUE;
        }
    }
    // source name may have been changed in .v2d
    // set to difx input source name
    else if (strncmp (pst[0], "source", 6) == 0)
    {
        sprintf (buff, "  source = %s;",
        D->source[sourceId].name);

        if ((pchar = strchr (line, ';')) == NULL)
        {
            strcpy (line, buff);
            strcat (line, "\n");
        }
        else            // copy rest of stmts from the start line
        {
            strcat (buff, pchar+1);
            strcpy (line, buff);
        }
    }
    // process start
    else if (strncmp (pst[0], "start", 5) == 0)
    {
        sscanf (pst[2], "%dy%dd%dh%dm%ds", &yy, &dd, &hh, &mm, &ss);
        fract = ((ss / 60.0 + mm) / 60.0 + hh ) / 24.0;
        // generate fresh scan start, in case it was overridden
        conv2date (D->scan[scanId].mjdStart, &caltime);
        sprintf (buff, "  start = %04hdy%03hdd%02hdh%02hdm%02ds;",
        caltime.year, caltime.day, 
        caltime.hour, caltime.minute, (int) caltime.second);
        if ((pchar = strchr (line, ';')) == NULL)
        {
            strcpy (line, buff);
            strcat (line, "\n");
        }
        else            // copy rest of stmts from the start line
        {
            strcat (buff, pchar+1);
            strcpy (line, buff);
        }
    }

    else if (strncmp (pst[0], "endscan", 7) == 0)
    {
        // end of the scan
        // generate and insert a fourfit ref time stmt
        // at the middle of the intersected scans
        itime = (latest_start + earliest_stop) / 2;
        conv2date (frt (D->scan[scanId].mjdStart, fract, itime), &caltime);

        sprintf (buff, 
        "    fourfit_reftime = %04hdy%03hdd%02hdh%02hdm%02ds;\n",
        caltime.year, caltime.day, 
        caltime.hour, caltime.minute, (int) caltime.second);
        // insert frt prior to line containing endscan
        fputs (buff, fout); 
    }
    // FIXME difxio scan start time is time when first
    // antenna comes on source. It may also be different
    // if mjdStart is set to the middle of a scan in
    // the .v2d file.
}

void MHO_DiFXStripVex::ExtractSite()
{
        // and filter out unused sites
    if (strncmp (pst[0], "def", 6) == 0)
    {               // start fresh block for this def
        strcpy (def_block, line);
        line[0] = 0;
    }
        // if this site is in the correlation, it is
        // now time to insert the site ID
    else if (strncmp (pst[0], "enddef", 6) == 0)
    {
            // if not in difx list, discard whole block
        i = isValidAntenna(D, current_site, scanId);
        if (i < 0)
        {
            // TODO: when nsite != nant the "discard" below leads to segfault further down the line?
            if (opts->verbose > 0)
            fprintf (stderr,
                "        intl_name %c%c difx_name -- mk4_id - "
                         "difx site index --\n",
                 current_site[0], current_site[1]);
            line[0] = 0;
            break;
        }

        c = single_code (current_site, opts->scodeFile);
        if (c == 0)
        {
            printf ("      All 52 codes used up, no code for %c%c\n",
             current_site[0], current_site[1]);
            return (-1);
        }
        sprintf (buff, "  mk4_site_ID = %c;\n", c);
        strcat (buff, line);
        strcpy (line, def_block);
        strcat (line, buff);
            // save 2-letter code for later use
        memcpy ((stns+i)->intl_name, current_site, 2);
            // file single letter code for future use
        (stns+i)->mk4_id = c;

        if (opts->verbose > 0)
        printf ("        intl_name %c%c difx_name %c%c mk4_id %c "
                 "difx site index %d\n",
        *((stns+i)->intl_name), *((stns+i)->intl_name+1),
        *((stns+i)->difx_name), *((stns+i)->difx_name+1),
        (stns+i)->mk4_id, i);
        nsite++;
    }

    else if (strncmp (pst[0], "$SITE", 5))
    {               // append lines onto this def's block
            // so long as it isn't the $SITE stmt
        strcat (def_block, line);
        line[0] = 0;
    }
}

void MHO_DiFXStripVex::ExtractStation()
{
        // delete vex's clock and das
    if (strncmp (pst[1], "$CLOCK", 6) == 0 || strncmp (pst[1], "$DAS", 4) == 0)
    {
        line[0] = 0;
    }
    else if (strncmp (pst[0], "enddef", 6) == 0)
    {
        strcpy (buff, "    ref $CLOCK = ");
        strcat (buff, current_def);
        strcat (buff, ";\n");
        strcat (buff, line);
        strcpy (line, buff);
    }
}


}//end namespace