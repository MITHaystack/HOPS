#ifndef MHO_DiFXStripVexFile_HH__
#define MHO_DiFXStripVexFile_HH__

/*
*@file: MHO_DiFXStripVexFile.hh
*@class: MHO_DiFXStripVexFile
*@author: J. Barrett
*@email: barrettj@mit.edu
*@date:
*@brief: extracts a scan-specific portion of session vex-file (originally from RJC's d2m4 createRoot.c)
*/

class MHO_DiFXStripVexFile
{
    public:
        MHO_DiFXStripVexFile();
        virtual ~MHO_DiFXStripVexFile();

        void SetSessionVexFile(std::string filename){fVexFile = filename;}
        void SetOutputFileName(std::string output_filename){fOutputFile = output_filename;}

    private:
};

#endif /* end of include guard: MHO_DiFXStripVexFile */