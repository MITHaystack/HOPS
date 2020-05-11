#include <stdio.h>

#include "HSpectrumFile.h"

int ReadSpectrumFile(const char* filename, struct HSpectrumFileStruct* spectrum)
{
    FILE* infile = NULL;
    infile = fopen(filename,"rb");

    ClearSpectrumFileStruct(spectrum);

    if(infile)
    {
        fread( &(spectrum->fHeader.fHeaderSize), sizeof(size_t), 1, infile);

        //TODO fix this to deal with possible multiple versions
        if( spectrum->fHeader.fHeaderSize != sizeof(struct HSpectrumHeaderStruct) )
        {
            fclose(infile);
            return HFILE_VERSION_ERROR;
        }

        fread( &(spectrum->fHeader.fVersionFlag), sizeof(char), HFLAG_WIDTH, infile);
        fread( &(spectrum->fHeader.fSidebandFlag), sizeof(char), HFLAG_WIDTH, infile);
        fread( &(spectrum->fHeader.fPolarizationFlag), sizeof(char), HFLAG_WIDTH, infile);

        fread( &(spectrum->fHeader.fStartTime), sizeof(uint64_t), 1, infile);
        fread( &(spectrum->fHeader.fSampleRate), sizeof(uint64_t), 1, infile);
        fread( &(spectrum->fHeader.fLeadingSampleIndex), sizeof(uint64_t), 1, infile);
        fread( &(spectrum->fHeader.fSampleLength), sizeof(size_t), 1, infile);
        fread( &(spectrum->fHeader.fNAverages), sizeof(size_t), 1, infile);
        fread( &(spectrum->fHeader.fSpectrumLength), sizeof(size_t), 1, infile);
        fread( &(spectrum->fHeader.fSpectrumDataTypeSize), sizeof(size_t), 1, infile);

        fread(  spectrum->fHeader.fExperimentName, sizeof(char), HNAME_WIDTH, infile);
        fread(  spectrum->fHeader.fSourceName, sizeof(char), HNAME_WIDTH, infile);
        fread(  spectrum->fHeader.fScanName, sizeof(char), HNAME_WIDTH, infile);

        //malloc space for the raw spectrum data
        spectrum->fRawSpectrumData = malloc( (spectrum->fHeader.fSpectrumLength)*(spectrum->fHeader.fSpectrumDataTypeSize) );
        if(spectrum->fRawSpectrumData == NULL)
        {
            //error could not allocate enough space
            fclose(infile);
            return HMALLOC_ERROR;
        }

        //read in the raw spectrum data
        fread( spectrum->fRawSpectrumData, sizeof(char), (spectrum->fHeader.fSpectrumLength)*(spectrum->fHeader.fSpectrumDataTypeSize), infile);

        fclose(infile);

        return HSUCCESS;
    }

        //error, couldn't open file
        return HFILE_OPEN_ERROR;
}
