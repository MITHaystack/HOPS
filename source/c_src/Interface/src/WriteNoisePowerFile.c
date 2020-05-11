#include <stdio.h>

#include "HNoisePowerFile.h"

int WriteNoisePowerFile(const char* filename, struct HNoisePowerFileStruct* power)
{
    size_t i;
    FILE* outfile = NULL;
    outfile = fopen(filename,"wb");

    if(outfile)
    {
        fwrite( &(power->fHeader.fHeaderSize), sizeof(size_t), 1, outfile);
        fwrite( &(power->fHeader.fVersionFlag), sizeof(char), HFLAG_WIDTH, outfile);
        fwrite( &(power->fHeader.fSidebandFlag), sizeof(char), HFLAG_WIDTH, outfile);
        fwrite( &(power->fHeader.fPolarizationFlag), sizeof(char), HFLAG_WIDTH, outfile);

        fwrite( &(power->fHeader.fStartTime), sizeof(uint64_t), 1, outfile);
        fwrite( &(power->fHeader.fSampleRate), sizeof(uint64_t), 1, outfile);
        fwrite( &(power->fHeader.fLeadingSampleIndex), sizeof(uint64_t), 1, outfile);
        fwrite( &(power->fHeader.fSampleLength), sizeof(size_t), 1, outfile);
        fwrite( &(power->fHeader.fAccumulationLength), sizeof(size_t), 1, outfile);
        fwrite( &(power->fHeader.fSwitchingFrequency), sizeof(double), 1, outfile);
        fwrite( &(power->fHeader.fBlankingPeriod), sizeof(double), 1, outfile);

        fwrite(  power->fHeader.fExperimentName, sizeof(char), HNAME_WIDTH, outfile);
        fwrite(  power->fHeader.fSourceName, sizeof(char), HNAME_WIDTH, outfile);
        fwrite(  power->fHeader.fScanName, sizeof(char), HNAME_WIDTH, outfile);

        //write accumulation data
        for(i = 0; i < power->fHeader.fAccumulationLength; i++)
        {
            struct HDataAccumulationStruct stat = power->fAccumulations[i];
            fwrite( &(stat.sum_x), sizeof(double), 1, outfile);
            fwrite( &(stat.sum_x2), sizeof(double), 1, outfile);
            fwrite( &(stat.count), sizeof(double), 1, outfile);
            fwrite( &(stat.state_flag), sizeof(uint64_t), 1, outfile);
            fwrite( &(stat.start_index), sizeof(uint64_t), 1, outfile);
            fwrite( &(stat.stop_index), sizeof(uint64_t), 1, outfile);
        }

        fclose(outfile);

        return HSUCCESS;
    }

    //error, couldn't open file
    return HFILE_OPEN_ERROR;
}
