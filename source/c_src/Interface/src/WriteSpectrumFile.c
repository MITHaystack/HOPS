#include <stdio.h>

#include "HSpectrumFile.h"

int WriteSpectrumFile(const char* filename, struct HSpectrumFileStruct* spectrum)
{
    FILE* outfile = NULL;
    outfile = fopen(filename,"wb");

    if(outfile)
    {
        fwrite( &(spectrum->fHeader.fHeaderSize), sizeof(size_t), 1, outfile);
        fwrite( &(spectrum->fHeader.fVersionFlag), sizeof(char), HFLAG_WIDTH, outfile);
        fwrite( &(spectrum->fHeader.fSidebandFlag), sizeof(char), HFLAG_WIDTH, outfile);
        fwrite( &(spectrum->fHeader.fPolarizationFlag), sizeof(char), HFLAG_WIDTH, outfile);

        fwrite( &(spectrum->fHeader.fStartTime), sizeof(uint64_t), 1, outfile);
        fwrite( &(spectrum->fHeader.fSampleRate), sizeof(uint64_t), 1, outfile);
        fwrite( &(spectrum->fHeader.fLeadingSampleIndex), sizeof(uint64_t), 1, outfile);
        fwrite( &(spectrum->fHeader.fSampleLength), sizeof(size_t), 1, outfile);
        fwrite( &(spectrum->fHeader.fNAverages), sizeof(size_t), 1, outfile);
        fwrite( &(spectrum->fHeader.fSpectrumLength), sizeof(size_t), 1, outfile);
        fwrite( &(spectrum->fHeader.fSpectrumDataTypeSize), sizeof(size_t), 1, outfile);

        fwrite(  spectrum->fHeader.fExperimentName, sizeof(char), HNAME_WIDTH, outfile);
        fwrite(  spectrum->fHeader.fSourceName, sizeof(char), HNAME_WIDTH, outfile);
        fwrite(  spectrum->fHeader.fScanName, sizeof(char), HNAME_WIDTH, outfile);

        //write the raw spectrum data
        fwrite( spectrum->fRawSpectrumData, sizeof(char), (spectrum->fHeader.fSpectrumLength)*(spectrum->fHeader.fSpectrumDataTypeSize), outfile);

        fclose(outfile);

        return HSUCCESS;
    }

    //error, couldn't open file
    return HFILE_OPEN_ERROR;
}
