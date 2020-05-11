#ifndef HSpectrumObject_HH__
#define HSpectrumObject_HH__

#include <stdint.h>
#include <cstdlib>
#include <iostream>
#include <string>
#include <fstream>
#include <istream>
#include <vector>

#include "HDataAccumulation.hh"

/*
*File: HSpectrumObject.hh
*Class: HSpectrumObject
*Author: J. Barrett
*Email: barrettj@mit.edu
*Date:
*Description: quick and dirty I/O wrapper
*/

namespace hops
{


template< typename XSpectrumType >
class HSpectrumObject
{
    public:

        HSpectrumObject():
            fSidebandFlag('?'),
            fPolarizationFlag('?'),
            fStartTime(0),
            fSampleRate(0),
            fLeadingSampleIndex(0),
            fSampleLength(0),
            fNAverages(0),
            fExperimentName("unknown"),
            fSourceName("unknown"),
            fScanName("unknown"),
            fDataOwned(true),
            fSpectrumLength(0),
            fSpectrumData(nullptr)
            {};

        //copy constuctor
        HSpectrumObject( const HSpectrumObject& copy):
            fSidebandFlag('?'),
            fPolarizationFlag('?'),
            fStartTime(0),
            fSampleRate(0),
            fLeadingSampleIndex(0),
            fSampleLength(0),
            fNAverages(0),
            fExperimentName("unknown"),
            fSourceName("unknown"),
            fScanName("unknown"),
            fDataOwned(true),
            fSpectrumLength(0),
            fSpectrumData(nullptr)
        {
            fSidebandFlag = copy.fSidebandFlag;
            fPolarizationFlag = copy.fPolarizationFlag;
            fStartTime = copy.fStartTime;
            fSampleRate = copy.fSampleRate;
            fLeadingSampleIndex = copy.fLeadingSampleIndex;
            fSampleLength = copy.fSampleLength;
            fNAverages = copy.fNAverages;
            fSpectrumLength = copy.fSpectrumLength;
            fExperimentName = copy.fExperimentName;
            fSourceName = copy.fSourceName;
            fScanName = copy.fScanName;
            if(copy.fDataOwned)
            {
                AllocateSpectrum();
                for(size_t i=0; i<copy.fSpectrumLength; i++)
                {
                    fSpectrumData[i] = copy.fSpectrumData[i];
                }
            }
            else
            {
                fDataOwned = false;
                fSpectrumData = copy.fSpectrumData;
            }
        }

        virtual ~HSpectrumObject()
        {
            if(fDataOwned){delete[] fSpectrumData; fSpectrumData = nullptr;};
        }

        char GetSidebandFlag() const {return fSidebandFlag;};
        char GetPolarizationFlag() const {return fPolarizationFlag;};
        uint64_t GetStartTime() const {return fStartTime;};
        uint64_t GetSampleRate() const {return fSampleRate;};
        uint64_t GetLeadingSampleIndex() const {return fLeadingSampleIndex;};
        size_t GetSampleLength() const {return fSampleLength;};
        size_t GetNAverages() const {return fNAverages;};
        size_t GetSpectrumLength() const {return fSpectrumLength;};

        std::string GetExperimentName() const {return fExperimentName;};
        std::string GetSourceName() const {return fSourceName;};
        std::string GetScanName() const {return fScanName;};

        XSpectrumType* GetSpectrumData() {return fSpectrumData;};
        const XSpectrumType* GetSpectrumData() const {return fSpectrumData;};

        //allocate spectrum space (assumes fSpectrumLength is set)
        void AllocateSpectrum()
        {
            if(fSpectrumData != nullptr && fDataOwned)
            {
                delete[] fSpectrumData;
                fSpectrumData = nullptr;
            }

            if(fSpectrumData == nullptr)
            {
                fSpectrumData = new XSpectrumType[fSpectrumLength];
                fDataOwned = true;
            }
            else 
            {
                std::cout<<"error, tried to allocate spectrum space on top of non-owned data"<<std::endl;
                std::exit(1);
            }
        }

        void DeallocateSpectrum()
        {
            if(fDataOwned)
            {
                delete[] fSpectrumData;
                fSpectrumData = nullptr;
            }
            else 
            {
                std::cout<<"error, tried to deallocate non-owned spectrum space"<<std::endl;
                std::exit(1);
            }
        }

        void SetSidebandFlag(const char& s) {fSidebandFlag = s;};
        void SetPolarizationFlag(const char& p) {fPolarizationFlag = p;};
        void SetStartTime(uint64_t st){fStartTime = st; };
        void SetSampleRate(uint64_t rate){fSampleRate = rate;};
        void SetLeadingSampleIndex(uint64_t n){fLeadingSampleIndex = n;};
        void SetSampleLength(size_t n){fSampleLength = n;};
        void SetNAverages(size_t n){fNAverages = n;};
        void SetSpectrumLength(size_t spec_length){fSpectrumLength = spec_length;};

        void SetExperimentName(const std::string& expname) {fExperimentName = expname;};
        void SetSourceName(const std::string& sourcename) {fSourceName = sourcename;};
        void SetScanName(const std::string& scanname) {fScanName = scanname;};

        //only use for output, to point to externally allocated array
        void SetSpectrumData(XSpectrumType* array){fSpectrumData = array; fDataOwned = false;};
        void ReleaseSpectrumData(){fSpectrumData = nullptr; fDataOwned = true;};

        //for now we are going to stuff the noise statistics data in here for estimating tsys w/ the noise diode
        //doing otherwise will require some re-architecting of the whole consumer/producer/buffer pool system
        void AppendOnAccumulation( HDataAccumulation accum){ fOnAccumulations.push_back(accum); };
        void ExtendOnAccumulation( const std::vector< HDataAccumulation >* accum_vec)
        {
            fOnAccumulations.reserve( fOnAccumulations.size() + accum_vec->size() );
            fOnAccumulations.insert( fOnAccumulations.end(), accum_vec->begin(), accum_vec->end() );
        };
        std::vector< HDataAccumulation >* GetOnAccumulations() {return &fOnAccumulations;};

        void AppendOffAccumulation( HDataAccumulation accum){ fOffAccumulations.push_back(accum); };
        void ExtendOffAccumulation( const std::vector< HDataAccumulation >* accum_vec)
        {
            fOffAccumulations.reserve( fOffAccumulations.size() + accum_vec->size() );
            fOffAccumulations.insert( fOffAccumulations.end(), accum_vec->begin(), accum_vec->end() );
        };
        std::vector< HDataAccumulation >* GetOffAccumulations() {return &fOffAccumulations;};


        void WriteToFile(std::string filename)
        {
            std::ofstream outfile;
            outfile.open(filename.c_str(), std::ios::out | std::ios::binary);
//            outfile << *this;

            outfile.write( (const char*) &fSidebandFlag, sizeof(char) );
            outfile.write( (const char*) &fPolarizationFlag, sizeof(char) );

            outfile.write( (const char*) &fStartTime, sizeof(uint64_t) );
            outfile.write( (const char*) &fSampleRate, sizeof(uint64_t) );
            outfile.write( (const char*) &fLeadingSampleIndex, sizeof(uint64_t) );
            outfile.write( (const char*) &fSampleLength, sizeof(size_t) );
            outfile.write( (const char*) &fNAverages, sizeof(size_t) );

            size_t length_value = fExperimentName.size();
            outfile.write( (const char*) &length_value, sizeof(size_t) );
            outfile.write( (const char*) &(fExperimentName[0]), length_value*sizeof(char) );
            length_value = fSourceName.size();
            outfile.write( (const char*) &length_value, sizeof(size_t) );
            outfile.write( (const char*) &(fSourceName[0]), length_value*sizeof(char) );
            length_value = fScanName.size();
            outfile.write( (const char*) &length_value, sizeof(size_t) );
            outfile.write( (const char*) &(fScanName[0]), length_value*sizeof(char) );

            //write the spectrum
            outfile.write( (const char*) &fSpectrumLength, sizeof(size_t) );
            outfile.write( (const char*) fSpectrumData, sizeof(XSpectrumType)*fSpectrumLength );
            unsigned int on_accum_size = fOnAccumulations.size();
            outfile.write( (const char*) &on_accum_size, sizeof(unsigned int) );
            for(unsigned int i=0; i<on_accum_size; i++)
            {
                HDataAccumulation stat = fOnAccumulations[i];
                outfile.write( (const char*) &(stat.sum_x), sizeof(double) );
                outfile.write( (const char*) &(stat.sum_x2), sizeof(double) );
                outfile.write( (const char*) &(stat.count), sizeof(double) );
                outfile.write( (const char*) &(stat.start_index), sizeof(uint64_t) );
                outfile.write( (const char*) &(stat.stop_index), sizeof(uint64_t) );
            }

            unsigned int off_accum_size = fOffAccumulations.size();
            outfile.write( (const char*) &off_accum_size, sizeof(unsigned int) );
            for(unsigned int i=0; i<off_accum_size; i++)
            {
                HDataAccumulation stat = fOffAccumulations[i];
                outfile.write( (const char*) &(stat.sum_x), sizeof(double) );
                outfile.write( (const char*) &(stat.sum_x2), sizeof(double) );
                outfile.write( (const char*) &(stat.count), sizeof(double) );
                outfile.write( (const char*) &(stat.start_index), sizeof(uint64_t) );
                outfile.write( (const char*) &(stat.stop_index), sizeof(uint64_t) );
            }

            outfile.close();
        }

        void ReadFromFile(std::string filename)
        {
            std::ifstream infile;
            infile.open(filename.c_str(), std::ifstream::binary);

            //infile >> *this;

            infile.read( (char*) &fSidebandFlag, sizeof(char) );
            infile.read( (char*) &fPolarizationFlag, sizeof(char) );

            infile.read( (char*) &fStartTime, sizeof(uint64_t) );
            infile.read( (char*) &fSampleRate, sizeof(uint64_t) );
            infile.read( (char*) &fLeadingSampleIndex, sizeof(uint64_t) );
            infile.read( (char*) &fSampleLength, sizeof(size_t) );
            infile.read( (char*) &fNAverages, sizeof(size_t) );

            char tmp;
            size_t length_value;
            fExperimentName.clear();
            infile.read( (char*) &length_value, sizeof(size_t) );
            for(size_t i=0; i<length_value; i++)
            {
                infile.read( (char*) &tmp, sizeof(char) );
                fExperimentName.push_back(tmp);
            }

            fSourceName.clear();
            infile.read( (char*) &length_value, sizeof(size_t) );
            for(size_t i=0; i<length_value; i++)
            {
                infile.read( (char*) &tmp, sizeof(char) );
                fSourceName.push_back(tmp);
            }

            fScanName.clear();
            infile.read( (char*) &length_value, sizeof(size_t) );
            for(size_t i=0; i<length_value; i++)
            {
                infile.read( (char*) &tmp, sizeof(char) );
                fScanName.push_back(tmp);
            }

            infile.read( (char*) &fSpectrumLength, sizeof(size_t) );
            //allocate some spectrum space
            ReleaseSpectrumData();
            AllocateSpectrum();
            infile.read( (char*) fSpectrumData, sizeof(XSpectrumType)*fSpectrumLength );

            unsigned int on_accum_size;
            infile.read( (char*) &on_accum_size, sizeof(unsigned int) );
            fOnAccumulations.reserve(on_accum_size);
            for(unsigned int i=0; i<on_accum_size; i++)
            {
                HDataAccumulation stat;
                infile.read( (char*) &(stat.sum_x), sizeof(double) );
                infile.read( (char*) &(stat.sum_x2), sizeof(double) );
                infile.read( (char*) &(stat.count), sizeof(double) );
                infile.read( (char*) &(stat.start_index), sizeof(uint64_t) );
                infile.read( (char*) &(stat.stop_index), sizeof(uint64_t) );
                fOnAccumulations.push_back(stat);
            }

            unsigned int off_accum_size;
            infile.read( (char*) &off_accum_size, sizeof(unsigned int) );
            fOffAccumulations.reserve(off_accum_size);
            for(unsigned int i=0; i<off_accum_size; i++)
            {
                HDataAccumulation stat;
                infile.read( (char*) &(stat.sum_x), sizeof(double) );
                infile.read( (char*) &(stat.sum_x2), sizeof(double) );
                infile.read( (char*) &(stat.count), sizeof(double) );
                infile.read( (char*) &(stat.start_index), sizeof(uint64_t) );
                infile.read( (char*) &(stat.stop_index), sizeof(uint64_t) );
                fOffAccumulations.push_back(stat);
           }

            infile.close();
        }

    protected:

        //data
        char fSidebandFlag;
        char fPolarizationFlag;
        uint64_t fStartTime; //acquisition start time in seconds since epoch
        uint64_t fSampleRate; //may need to accomodate doubles?
        uint64_t fLeadingSampleIndex; //sample index since start of the acquisition
        size_t fSampleLength; //total number of samples used to compute the spectrum
        size_t fNAverages; //if spectral averaging was used, how many averaging periods were used

        std::string fExperimentName;
        std::string fSourceName;
        std::string fScanName;

        //indicates if we need to delete spectrum data on destruction
        bool fDataOwned;
        size_t fSpectrumLength; //number of spectral points
        //pointer to spectrum data array, not owned if passed through SetSpectrumData()
        XSpectrumType* fSpectrumData;

        //on/off statistics data
        std::vector< HDataAccumulation > fOnAccumulations;
        std::vector< HDataAccumulation > fOffAccumulations;

};

template< typename XSpectrumType, typename XStreamType >
XStreamType& operator>>(XStreamType& s, HSpectrumObject<XSpectrumType>& aData)
{
    uint64_t start_time; 
    uint64_t sample_rate;
    uint64_t leading_sample_index; 
    size_t sample_length; 
    size_t n_averages; 
    size_t spectrum_length;
    std::string expname;
    std::string sourcename;
    std::string scanname;

    s >> start_time;
    s >> sample_rate;
    s >> leading_sample_index;
    s >> sample_length;
    s >> n_averages;


    s >> expname;
    s >> sourcename;
    s >> scanname;

    s >> spectrum_length;

    aData.SetStartTime(start_time);
    aData.SetSampleRate(sample_rate);
    aData.SetLeadingSampleIndex(leading_sample_index);
    aData.SetSampleLength(sample_length);
    aData.SetNAverages(n_averages);

    aData.SetExperimentName(expname);
    aData.SetSourceName(sourcename);
    aData.SetScanName(scanname);

    aData.SetSpectrumLength(spectrum_length);

    aData.AllocateSpectrum();

    XSpectrumType* spec = aData.GetSpectrumData();
    for(size_t i=0; i<spectrum_length; i++)
    {
        s >> spec[i];
    }

    return s;
}

template< typename XSpectrumType, typename XStreamType >
XStreamType& operator<<(XStreamType& s, const HSpectrumObject<XSpectrumType>& aData)
{
    s << aData.GetStartTime() << ' ';
    s << aData.GetSampleRate() << ' ';
    s << aData.GetLeadingSampleIndex() << ' ';
    s << aData.GetSampleLength() << ' ';
    s << aData.GetNAverages() << ' ';

    s << aData.GetExperimentName() << ' ';
    s << aData.GetSourceName() << ' ';
    s << aData.GetScanName() << ' ';

    s << aData.GetSpectrumLength() << ' ';

    const XSpectrumType* spec = aData.GetSpectrumData();
    size_t n = aData.GetSpectrumLength();
    for(size_t i=0; i<n; i++)
    {
        s << spec[i] << ' ';
    }

    return s;
}

}

#endif /* end of include guard: HSpectrumObject */
