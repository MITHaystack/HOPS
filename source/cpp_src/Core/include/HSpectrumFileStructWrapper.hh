#ifndef HSpectrumFileStructWrapper_HH__
#define HSpectrumFileStructWrapper_HH__

extern "C" 
{
    #include "HBasicDefines.h"
    #include "HSpectrumFile.h"
    #include "HNoisePowerFile.h"
}

#include <vector>
#include <string>
#include <cstring>

/*
*File: HSpectrumFileStructWrapper.hh
*Class: HSpectrumFileStructWrapper
*Author: J. Barrett
*Email: barrettj@mit.edu
*Date:
*Description:
*/

namespace hops
{

template< typename XSpectrumDataType >
class HSpectrumFileStructWrapper
{
    public:

        HSpectrumFileStructWrapper()
        {
            InitializeSpectrumFileStruct(&fFileStruct);
        }

        HSpectrumFileStructWrapper(std::string filename)
        {
            InitializeSpectrumFileStruct(&fFileStruct);
            ReadFromFile(filename);
        }

        HSpectrumFileStructWrapper(const HSpectrumFileStructWrapper& copy)
        {
            InitializeSpectrumFileStruct(&fFileStruct);
            CopySpectrumFileStruct( &(copy.fFileStruct), &fFileStruct);
        }

        virtual ~HSpectrumFileStructWrapper()
        {
            ClearSpectrumFileStruct(&fFileStruct);
        }

        void ReadFromFile(std::string filename)
        {
             int code = ReadSpectrumFile(filename.c_str(), &fFileStruct);
             if(code != HSUCCESS)
             {
                 std::cout<<"file error: "<<code<<", file: "<<filename<<std::endl;
             }
        }

        void WriteToFile(std::string filename)
        {
            fFileStruct.fHeader.fSpectrumDataTypeSize = sizeof( XSpectrumDataType );
            WriteSpectrumFile(filename.c_str(), &fFileStruct);
        }

        void SetVersionFlag(std::string vflag)
        {
            //TODO construct version flag automatically, and disable this function
            std::string trunc = vflag;
            if(vflag.size() >= HFLAG_WIDTH){ trunc = vflag.substr(0,HFLAG_WIDTH); }
            strcpy(fFileStruct.fHeader.fVersionFlag, trunc.c_str());
        }
        std::string GetVersionFlag() const {return std::string(fFileStruct.fHeader.fVersionFlag); };

        void SetSidebandFlag(std::string sbflag)
        {
            std::string trunc = sbflag;
            if(sbflag.size() >= HFLAG_WIDTH){ trunc = sbflag.substr(0,HFLAG_WIDTH); }
            strcpy(fFileStruct.fHeader.fSidebandFlag, trunc.c_str());
        }
        std::string GetSidebandFlagFlag() const {return std::string(fFileStruct.fHeader.fSidebandFlag); };

        void SetPolarizationFlag(std::string pflag)
        {
            std::string trunc = pflag;
            if(pflag.size() >= HFLAG_WIDTH){ trunc = pflag.substr(0,HFLAG_WIDTH); }
            strcpy(fFileStruct.fHeader.fPolarizationFlag, trunc.c_str());
        }
        std::string GetPolarizationFlag() const {return std::string(fFileStruct.fHeader.fPolarizationFlag); };

        void SetStartTime(uint64_t start){ fFileStruct.fHeader.fStartTime = start;}
        uint64_t GetStartTime() const {return fFileStruct.fHeader.fStartTime;}

        void SetSampleRate(uint64_t sample_rate){ fFileStruct.fHeader.fSampleRate = sample_rate;};
        uint64_t GetSampleRate() const {return fFileStruct.fHeader.fSampleRate;}

        void SetLeadingSampleIndex(uint64_t index){ fFileStruct.fHeader.fLeadingSampleIndex = index;};
        uint64_t GetLeadingSampleIndex() const {return fFileStruct.fHeader.fLeadingSampleIndex;}

        void SetSampleLength(uint64_t sample_length){ fFileStruct.fHeader.fSampleLength = sample_length; };
        uint64_t GetSampleLength() const {return fFileStruct.fHeader.fSampleLength;}

        void SetNAverages(uint64_t n_ave){ fFileStruct.fHeader.fNAverages = n_ave;};
        uint64_t GetNAverages() const {return fFileStruct.fHeader.fNAverages;}

        void SetSpectrumLength(uint64_t spec_length){ fFileStruct.fHeader.fSpectrumLength = spec_length;};
        uint64_t GetSpectrumLength() const {return fFileStruct.fHeader.fSpectrumLength;}

        XSpectrumDataType* GetSpectrumData()
        {
            if(sizeof(XSpectrumDataType) == fFileStruct.fHeader.fSpectrumDataTypeSize)
            {
                return reinterpret_cast<XSpectrumDataType*>(fFileStruct.fRawSpectrumData);
            }
            else
            {
                return nullptr;
            }
        }

        void GetSpectrumData(std::vector<XSpectrumDataType>& data)
        {
            data.clear();
            if(sizeof(XSpectrumDataType) == fFileStruct.fHeader.fSpectrumDataTypeSize)
            {
                data.resize(fFileStruct.fHeader.fSpectrumLength);
                XSpectrumDataType* arr = reinterpret_cast<XSpectrumDataType*>(fFileStruct.fRawSpectrumData);
                for(size_t i=0; i<fFileStruct.fHeader.fSpectrumLength; i++)
                {
                    data[i] = arr[i];
                }
            }
        }

    private:

        struct HSpectrumFileStruct fFileStruct;

};

}

#endif /* end of include guard: HSpectrumFileStructWrapper */
