#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

//#include "difx2mark4.h"

#define MAX_FPPAIRS 10000           // dimensioned for b-lines x chans x pol_prods
#define MAX_DFRQ 800                // allowed max number of *DiFX* frequencies

struct CommandLineOptions
{
    int verbose;
    char fgroups[16];
    char bandwidth[8];
};

struct fblock_tag
{
    struct
    {
        char chan_id[8];            // channel id for identifying channels
        char sideband;              // U or L
        char pol;                   // R or L
        int ant;                    // antenna table index
        int find;                   // frequency table index
        int bs;                     // quantization bits/sample
        int first_time;             // true iff first entry in table of chan_id for ant 
        int zoom;                   // true iff this channel is zoom mode
        int n_spec_chan;            // # of spectral channels output from difx
        double pcal_int;            // pcal interval (MHz)
        double freq;                // LO frequency (MHz); negative for LSB
        double bw;                  // bandwidth (MHz)
    } stn[2];                   // reference | remote
};


int fill_fblock (DifxInput *D,                    // difx input structure pointer
                 struct CommandLineOptions *opts, // ptr to input options
                 struct fblock_tag *pfb);          // pointer to table to be filled in
