#ifndef HDataAccumulationContainer_HH__
#define HDataAccumulationContainer_HH__

#include <string>
#include <vector>
#include <stdint.h>

extern "C"
{
    #include "HDataAccumulationStruct.h"
}

/*
*File: HDataAccumulationContainer.hh
*Class: HDataAccumulationContainer
*Author: J. Barrett
*Email: barrettj@mit.edu
*Date:
*Description: wrapper class to hold data accumuations for noise cal + metadata
*/

class HDataAccumulationContainer
{
    public:

        HDataAccumulationContainer():
            fSidebandFlag('?'),
            fPolarizationFlag('?'),
            fAcquisitionStartSecond(0),
            fSampleIndex(0),
            fSampleRate(0),
            fSampleLength(0),
            fNoiseDiodeSwitchingFrequency(0.0),
            fNoiseDiodeBlankingPeriod(0.0)
        {};

        virtual ~HDataAccumulationContainer(){};

        char GetSidebandFlag() const {return fSidebandFlag;};
        void SetSidebandFlag(const char& s) {fSidebandFlag = s;};

        char GetPolarizationFlag() const {return fPolarizationFlag;};
        void SetPolarizationFlag(const char& p) {fPolarizationFlag = p;};

        //the integer second (since the epoch) at which the aqcuisition was started
        uint64_t GetAcquisitionStartSecond() const {return fAcquisitionStartSecond;};
        void SetAcquisitionStartSecond(const uint64_t& sec) {fAcquisitionStartSecond = sec;};

        //sample index relative to start of the aqcuisition
        uint64_t GetLeadingSampleIndex() const {return fSampleIndex;};
        void SetLeadingSampleIndex(const uint64_t& index){fSampleIndex = index;};

        //total lengthe of sample array
        uint64_t GetSampleLength() const {return fSampleLength;};
        void SetSampleLength(const uint64_t& len){fSampleLength = len;};

        //sampling rate of the digitizer
        uint64_t GetSampleRate() const {return fSampleRate;};
        void SetSampleRate(const uint64_t& rate) {fSampleRate = rate;};

        //for now we are going to stuff the noise statistics data in here for estimating tsys w/ the noise diode
        //doing otherwise will require some re-architecting of the whole consumer/producer/buffer pool system

        double GetNoiseDiodeSwitchingFrequency() const {return fNoiseDiodeSwitchingFrequency;};
        void SetNoiseDiodeSwitchingFrequency(const double& freq){fNoiseDiodeSwitchingFrequency = freq;};

        double GetNoiseDiodeBlankingPeriod() const {return fNoiseDiodeBlankingPeriod;};
        void SetNoiseDiodeBlankingPeriod(const double& blank){fNoiseDiodeBlankingPeriod = blank;};

        void ClearAccumulation(){fAccumulations.clear();};
        void AppendAccumulation( struct HDataAccumulationStruct accum){ fAccumulations.push_back(accum); };
        void ExtendAccumulation( const std::vector< struct HDataAccumulationStruct >* accum_vec)
        {
            fAccumulations.reserve( fAccumulations.size() + accum_vec->size() );
            fAccumulations.insert( fAccumulations.end(), accum_vec->begin(), accum_vec->end() );
        };
        std::vector< struct HDataAccumulationStruct >* GetAccumulations() {return &fAccumulations;};


        HDataAccumulationContainer& operator= (const HDataAccumulationContainer& rhs)
        {
            if( this != &rhs)
            {
                fSidebandFlag = rhs.fSidebandFlag;
                fPolarizationFlag = rhs.fPolarizationFlag;
                fAcquisitionStartSecond = rhs.fAcquisitionStartSecond;
                fSampleIndex = rhs.fSampleIndex;
                fSampleRate = rhs.fSampleRate;
                fSampleLength = rhs.fSampleLength;
                fNoiseDiodeSwitchingFrequency = rhs.fNoiseDiodeSwitchingFrequency;
                fNoiseDiodeBlankingPeriod = rhs.fNoiseDiodeBlankingPeriod;
                fAccumulations = rhs.fAccumulations;
            }
            return *this;
        }


    private:

        char fSidebandFlag; //U, L, D
        char fPolarizationFlag; //L, R, X, Y

        //seconds since epoch, of the start of the aquisition
        uint64_t fAcquisitionStartSecond;

        //index of the leading sample relative to the start of the aquisition
        uint64_t fSampleIndex;

        //sample rate in Hz (integer only, may need to relax this limitation)
        uint64_t fSampleRate;

        uint64_t fSampleLength;

        double fNoiseDiodeSwitchingFrequency;
        double fNoiseDiodeBlankingPeriod;

        //data statistics (for noise diode)
        std::vector< struct HDataAccumulationStruct > fAccumulations;
};

#endif /* end of include guard: HDataAccumulationContainer */
