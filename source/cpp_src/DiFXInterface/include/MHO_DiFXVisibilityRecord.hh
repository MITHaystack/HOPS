#ifndef MHO_DiFXVisibilityRecord_HH__
#define MHO_DiFXVisibilityRecord_HH__

/*!
*@file MHO_DiFXVisibilityRecord.hh
*@class MHO_DiFXVisibilityRecord
*@author J. Barrett - barrettj@mit.edu 
*@date
*@brief
*/

#include <cstdint>
#include <vector>
#include <complex>
#include <cstring>

namespace hops
{

class MHO_DiFXVisibilityRecord
{
    public:
        MHO_DiFXVisibilityRecord(){Reset();}

        MHO_DiFXVisibilityRecord(const MHO_DiFXVisibilityRecord& copy)
        {
            nchan = copy.nchan;
            visnum = copy.visnum;
            sync = copy.sync;
            headerversion = copy.headerversion;
            baseline = copy.baseline;
            mjd = copy.mjd;
            seconds = copy.seconds;
            configindex = copy.configindex;
            sourceindex = copy.sourceindex;
            freqindex = copy.freqindex;
            memcpy(polpair, copy.polpair, 3);
            pulsarbin = copy.pulsarbin;
            dataweight = copy.dataweight;
            memcpy(uvw, copy.uvw, 3*sizeof(double));
            visdata = copy.visdata;

            bandwidth = copy.bandwidth;
            sky_freq = copy.sky_freq;
            freq_band = copy.freq_band;
            net_sideband = copy.net_sideband;

        };

        virtual ~MHO_DiFXVisibilityRecord(){};

        void Reset()
        {
            nchan = 0;
            visnum = 0;
            sync = 0;
            headerversion = 0;
            baseline = 0;
            mjd = 0;
            seconds = 0.0;
            configindex = 0;
            sourceindex = 0;
            freqindex = 0;
            polpair[0] = '\0';
            polpair[1] = '\0';
            polpair[2] = '\0';
            pulsarbin = 0;
            dataweight = 0.0;
            uvw[0] = 0.0;
            uvw[1] = 0.0;
            uvw[2] = 0.0;
            visdata.clear();

            bandwidth = 0.0;
            sky_freq = 0.0;;
            freq_band = "";
            net_sideband = "";
        }

        //we leave the members public  --- taken from directly from DifxVisRecord;

        int nchan;          /*! number of channels to expect */
        int visnum;         /*! counter of number of vis */
        int sync;           /*! space to store the sync value */
        int headerversion;  /*! 0=old style, 1=new binary style */
        int baseline;       /*! The baseline number (256*A1 + A2, 1 indexed) */
        int mjd;            /*! The MJD integer day */
        double seconds;     /*! The seconds offset from mjd */
        int configindex;    /*! The index to the configuration table */
        int sourceindex;    /*! The index to the source table */
        int freqindex;      /*! The index to the freq table */
        char polpair[3];    /*! The polarisation pair */
        int pulsarbin;      /*! The pulsar bin */
        double dataweight;  /*! The fractional data weight */
        double uvw[3];      /*! The u,v,w values in metres */
        std::vector< std::complex<float> > visdata; /*! nchan complex values (2x float) */


        //these are extra parameters that we will fill in when/if we have the info
        double bandwidth;
        double sky_freq;
        std::string freq_band;
        std::string net_sideband;
};

//helper union used for reading in visibility records
//to catch over-runs with sync word
typedef union
{
    float values[2];
    int32_t sync_test[2];
}
MHO_VisibilityChunk;


}//end of namespace

#endif /*! end of include guard: MHO_DiFXVisibilityRecord */
