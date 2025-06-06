// deconstruct difx table links that define selection and recording of channels,
// and how they were correlated. It then puts the information into an easy-to-use
// frequency structure

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "difx2mark4.h"

int fill_fblock (DifxInput *D,                    // difx input structure pointer
                 struct CommandLineOptions *opts, // ptr to input options
                 struct fblock_tag *pfb)          // pointer to table to be filled in
    {
    int i,
        j,
        k,
        n,
        nprod = 0,                  // index for a frequency & polarization pair
        irbAfid,
        irbBfid,
        ibandA,
        ibandB,
        irfAfid,
        irfBfid,
        idfABfid,
        ants[64],
        swapped,
        present,
        first[2][2],                   // indexed by 0|1 for [USB/LSB][RCP/LCP]
        sbind,
        polind,
        nant,
        nfreq,
        zoom,
        nbw,
        nfg;

    char pol,
         buff[6];

    double temp,
           freqs[MAX_DFRQ];

    DifxBaseline *pbl;
    DifxDatastream *pdsA,
                   *pdsB;
    DifxFreq *pfr,
             *pdfr;


                                    // first fill in the frequency block structure
    for (n=0; n<D->nBaseline; n++)
        {
        pbl = D->baseline + n;
        pdsA = D->datastream + pbl->dsA;
        pdsB = D->datastream + pbl->dsB;
        for (i=0; i<pbl->nFreq; i++)
            {
            #if DIFXIO_VERSION_MAJOR > 3 && DIFXIO_VERSION_MINOR >= 7 
            idfABfid = pbl->destFq[i];
            pdfr = D->freq + idfABfid;
            #endif
            for (j=0; j<*pbl->nPolProd; j++)
                {
                ibandA = pbl->bandA[i][j];
                ibandB = pbl->bandB[i][j];
                                    // bandA  (reference station)
                if (ibandA < pdsA->nRecBand)
                    {               // not zoom mode
                    zoom = FALSE;
                    irbAfid = pdsA->recBandFreqId[ibandA];
                    pol = pdsA->recBandPolName[ibandA];
                    irfAfid = pdsA->recFreqId[irbAfid];
                    pfr = D->freq + irfAfid;
                    }
                else                // zoom mode
                    {
                    zoom = TRUE;
                    irbAfid = pdsA->zoomBandFreqId[ibandA-pdsA->nRecBand];
                    pol = pdsA->zoomBandPolName[ibandA-pdsA->nRecBand];
                    irfAfid = pdsA->zoomFreqId[irbAfid];
                    pfr = D->freq + irfAfid;
                    }
                #if DIFXIO_VERSION_MAJOR > 3 && DIFXIO_VERSION_MINOR >= 7 
                if (irfAfid != idfABfid)
                    {               // bandA is member of outputband; register the output instead!
                    //printf("info: .inp baseline %d pol %d: A fq %d != destFq %d, using destFq instead and mark as Zoom\n", i, j, irfAfid, idfABfid);
                    pfr = pdfr;
                    zoom = TRUE;
                    irfAfid = idfABfid;
                    }
                #endif
                                    // stuff ref station fblock structure
                pfb[nprod].stn[0].pol      = pol;
                pfb[nprod].stn[0].ant      = pdsA->antennaId;
                pfb[nprod].stn[0].find     = irfAfid;
                pfb[nprod].stn[0].freq     = pfr->freq;
                pfb[nprod].stn[0].sideband = pfr->sideband;
                pfb[nprod].stn[0].bw       = pfr->bw;
                pfb[nprod].stn[0].bs       = pdsA->quantBits;
                pfb[nprod].stn[0].zoom     = zoom;
                pfb[nprod].stn[0].pcal_int = pdsA->phaseCalIntervalMHz;
                pfb[nprod].stn[0].n_spec_chan = pfr->nChan / pfr->specAvg;

                                    // bandB  (remote station)
                if (ibandB < pdsB->nRecBand)
                    {               // not zoom mode
                    zoom = FALSE;
                    irbBfid = pdsB->recBandFreqId[ibandB];
                    pol = pdsB->recBandPolName[ibandB];
                    irfBfid = pdsB->recFreqId[irbBfid];
                    pfr = D->freq + irfBfid;
                    }
                else                // zoom mode
                    {
                    zoom = TRUE;
                    irbBfid = pdsB->zoomBandFreqId[ibandB-pdsB->nRecBand];
                    pol = pdsB->zoomBandPolName[ibandB-pdsB->nRecBand];
                    irfBfid = pdsB->zoomFreqId[irbBfid];
                    pfr = D->freq + irfBfid;
                    }
                #if DIFXIO_VERSION_MAJOR > 3 && DIFXIO_VERSION_MINOR >= 7 
                if (irfBfid != idfABfid)
                    {               // bandB is member of outputband; register the output instead!
                    //printf("info: .inp baseline %d pol %d: B fq %d != destFq %d, using destFq instead and mark as Zoom\n", i, j, irfBfid, idfABfid);
                    pfr = pdfr;
                    zoom = TRUE;
                    irfBfid = idfABfid;
                    }
                #endif
                                    // stuff rem station fblock structure
                pfb[nprod].stn[1].pol      = pol;
                pfb[nprod].stn[1].ant      = pdsB->antennaId;
                pfb[nprod].stn[1].find     = irfBfid;
                pfb[nprod].stn[1].freq     = pfr->freq;
                pfb[nprod].stn[1].sideband = pfr->sideband;
                pfb[nprod].stn[1].bw       = pfr->bw;
                pfb[nprod].stn[1].bs       = pdsB->quantBits;
                pfb[nprod].stn[1].zoom     = zoom;
                pfb[nprod].stn[1].pcal_int = pdsB->phaseCalIntervalMHz;
                pfb[nprod].stn[1].n_spec_chan = pfr->nChan / pfr->specAvg;

                                    // if sidebands mixed, make both USB
                if (pfb[nprod].stn[0].sideband != pfb[nprod].stn[1].sideband)
                    for (k=0; k<2; k++)
                        if (pfb[nprod].stn[k].sideband == 'L')
                            {
                            pfb[nprod].stn[k].freq -= pfb[nprod].stn[k].bw;
                            pfb[nprod].stn[k].find = pfb[nprod].stn[1-k].find;
                            pfb[nprod].stn[k].sideband = 'U';
                            }

                                    // bump and check product index
                if (++nprod > MAX_FPPAIRS)
                    {
                    printf ("too many freq-polpair combos; redimension\n");
                    return -1;
                    }
                }
            }
        }
                                    // now form & fill in the channel id's
                                    // make a list of all antennas present in fblock
    nant = 0;
    for (n=0; n<nprod; n++)
        for (k=0; k<2; k++)         // k = 0|1 for ref|rem antenna
            {
            present = FALSE;        // is antenna new?
            for (i=0; i<nant; i++)
                if (pfb[n].stn[k].ant == ants[i])
                    present = TRUE;
            if (!present)
                ants[nant++] = pfb[n].stn[k].ant;
            if (nant > 64)          // sanity check
                {
                printf ("too many antennas; redimension\n");
                return -1;
                }
            }

                                    // loop over those antennas
    for (i=0; i<nant; i++)
        {                           // for each antenna make a list of all frequencies
        nfreq = 0;
        for (n=0; n<nprod; n++)
            for (k=0; k<2; k++)     // k = 0|1 for ref|rem antenna
                {
                if (pfb[n].stn[k].ant == i)
                    {               // antenna matches; look for unique freq
                    present = FALSE;// is frequency new?
                    for (j=0; j<nfreq; j++)
                        if (pfb[n].stn[k].freq == freqs[j])
                            present = TRUE;
                    if (!present)
                        freqs[nfreq++] = pfb[n].stn[k].freq;
                                    // sanity check
                    if (nfreq > MAX_DFRQ)
                        {
                        printf ("too many frequencies, exceeding MAX_DFRQ; redimension\n");
                        return -1;
                        }
                    }
                }
                                    // bubble sort the frequency list
        do
            {
            swapped = FALSE;
            for (j=0; j<nfreq-1; j++)
                if (freqs[j] > freqs[j+1])
                    {
                    temp = freqs[j];
                    freqs[j] = freqs[j+1];
                    freqs[j+1] = temp;
                    swapped = TRUE;
                    }
            }
        while (swapped);
                                    // remove (collapse out) redundant frequencies
        for (j=0; j<nfreq-1; j++)
            if (freqs[j] == freqs[j+1])
                {
                for (k=j;k<nfreq-1;k++)
                    freqs[k] = freqs[k+1];
                nfreq--;
                }
                                    // generate channel id's for each freq
        for (j=0; j<nfreq; j++)
            {
            sprintf (buff, "%c%02d", getband (freqs[j]), j);
            buff[3] = 'U';          // change upon insertion if LSB
            buff[4] = 0;
            buff[5] = 0;
            first[0][0] = TRUE;     // first time for [sb][pol]
            first[0][1] = TRUE;
            first[1][0] = TRUE;
            first[1][1] = TRUE;
                                    // insert channel id's back into fblock
                                    // everywhere that ant & freq match
            for (n=0; n<nprod; n++)
                for (k=0; k<2; k++)     // k = 0|1 for ref|rem antenna
                    if (pfb[n].stn[k].ant == i && pfb[n].stn[k].freq == freqs[j])
                        {
                        buff[3] = pfb[n].stn[k].sideband;
                        buff[4] = pfb[n].stn[k].pol;
                        strcpy (pfb[n].stn[k].chan_id, buff);
                        sbind  = (pfb[n].stn[k].sideband == 'U') ? 0 : 1;
                        polind = (pfb[n].stn[k].pol == 'R' 
                               || pfb[n].stn[k].pol == 'X') ? 0 : 1;
                                    // see if first mention for ant, freq, sb, & pol
                        if (first[sbind][polind])
                            {
                            pfb[n].stn[k].first_time = TRUE;
                            first[sbind][polind] = FALSE;
                            }
                        else
                            pfb[n].stn[k].first_time = FALSE;
                        }
            }
        }


                                    // if freq groups specified, remove any non-matching lines
    if (strlen (opts->fgroups) != 0)
        {
        nfg = 0;
        for (n=0; n<nprod; n++)
            if (strchr (opts->fgroups, pfb[n].stn[0].chan_id[0]) == NULL)
                {
                for (i=n; i<nprod-1; i++) // slide remaining entries down one location
                    pfb[i] = pfb[i+1];
                pfb[i].stn[0].ant = 0;    // mark new end
                nprod--;                  // one less pfb entry
                n--;                      //need to reexamine this slot now
                nfg++;
                }
        fprintf (stderr, "%d correlation product channels deleted due to fgroup not %s\n",
                 nfg, opts->fgroups);
        }
                                    // if desired bandwidth specified, 
                                    // remove any non-matching lines
    if (strlen (opts->bandwidth) != 0)
        {
        nbw = 0;
        for (n=0; n<nprod; n++)
            if (atof (opts->bandwidth) != pfb[n].stn[0].bw)
                {
                for (i=n; i<nprod-1; i++) // slide remaining entries down one location
                    pfb[i] = pfb[i+1];
                pfb[i].stn[0].ant = 0;    // mark new end
                nprod--;                  // one less pfb entry
                n--;                      //need to reexamine this slot now
                nbw++;
                }
        fprintf (stderr, "%d correlation product channels deleted due to bw not %s\n",
                 nbw, opts->bandwidth);
        }

    if (opts->verbose > 1)
        {
        fprintf (stderr, "               ch_id s p 1st a id  z pc bs  freq    bw   #vis\n");
        for (n=0; n<nprod; n++)     // debug - print out fblock table
            fprintf (stderr,
                    "   fblock[%03d] %s %c %c %2d %2d %2d %2d %1d  %1d %.3f %.3f %4d\n"
                     "               %s %c %c %2d %2d %2d %2d %1d  %1d %.3f %.3f %4d\n",
                  n, pfb[n].stn[0].chan_id, pfb[n].stn[0].sideband, pfb[n].stn[0].pol,
                  pfb[n].stn[0].first_time, pfb[n].stn[0].ant, pfb[n].stn[0].find,
                  pfb[n].stn[0].zoom, (int)(pfb[n].stn[0].pcal_int+0.5), pfb[n].stn[0].bs,
                  pfb[n].stn[0].freq, pfb[n].stn[0].bw, pfb[n].stn[0].n_spec_chan,

                  pfb[n].stn[1].chan_id, pfb[n].stn[1].sideband, pfb[n].stn[1].pol,
                  pfb[n].stn[1].first_time, pfb[n].stn[1].ant, pfb[n].stn[1].find, 
                  pfb[n].stn[1].zoom, (int)(pfb[n].stn[1].pcal_int+0.5), pfb[n].stn[1].bs,
                  pfb[n].stn[1].freq, pfb[n].stn[1].bw, pfb[n].stn[1].n_spec_chan);
        }

    pfb[nprod].stn[0].ant = -1;     // mark end of table
    return 0;                       // signify that all is well
    }
