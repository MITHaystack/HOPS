#include <stdio.h>

#include "HNoisePowerFile.h"

int ReadNoisePowerFile(const char* filename, struct HNoisePowerFileStruct* power)
{
    size_t i;
    FILE* infile = NULL;
    infile = fopen(filename,"rb");

    if(infile)
    {
        fread( &(power->fHeader.fHeaderSize), sizeof(size_t), 1, infile);
        fread( &(power->fHeader.fVersionFlag), sizeof(char), HFLAG_WIDTH, infile);
        fread( &(power->fHeader.fSidebandFlag), sizeof(char), HFLAG_WIDTH, infile);
        fread( &(power->fHeader.fPolarizationFlag), sizeof(char), HFLAG_WIDTH, infile);

        fread( &(power->fHeader.fStartTime), sizeof(uint64_t), 1, infile);
        fread( &(power->fHeader.fSampleRate), sizeof(uint64_t), 1, infile);
        fread( &(power->fHeader.fLeadingSampleIndex), sizeof(uint64_t), 1, infile);
        fread( &(power->fHeader.fSampleLength), sizeof(size_t), 1, infile);
        fread( &(power->fHeader.fAccumulationLength), sizeof(size_t), 1, infile);
        fread( &(power->fHeader.fSwitchingFrequency), sizeof(double), 1, infile);
        fread( &(power->fHeader.fBlankingPeriod), sizeof(double), 1, infile);

        fread(  power->fHeader.fExperimentName, sizeof(char), HNAME_WIDTH, infile);
        fread(  power->fHeader.fSourceName, sizeof(char), HNAME_WIDTH, infile);
        fread(  power->fHeader.fScanName, sizeof(char), HNAME_WIDTH, infile);

        //malloc space for the accumulation data
        power->fAccumulations = malloc( (power->fHeader.fAccumulationLength)*sizeof(struct HDataAccumulationStruct) );
        if(power->fAccumulations == NULL)
        {
            //error could not allocate enough space
            fclose(infile);
            return HMALLOC_ERROR;
        }

        //write accumulation data
        for(i = 0; i < power->fHeader.fAccumulationLength; i++)
        {
            struct HDataAccumulationStruct* stat = &(power->fAccumulations[i]);
            fread( &(stat->sum_x), sizeof(double), 1, infile);
            fread( &(stat->sum_x2), sizeof(double), 1, infile);
            fread( &(stat->count), sizeof(double), 1, infile);
            fread( &(stat->state_flag), sizeof(uint64_t), 1, infile);
            fread( &(stat->start_index), sizeof(uint64_t), 1, infile);
            fread( &(stat->stop_index), sizeof(uint64_t), 1, infile);
        }

        fclose(infile);

        return HSUCCESS;
    }

    //error, couldn't open file
    return HFILE_OPEN_ERROR;
}
