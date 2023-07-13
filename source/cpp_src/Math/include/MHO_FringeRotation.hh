#ifndef MHO_FringeRotation_HH__
#define MHO_FringeRotation_HH__


#include <complex>
#include <cmath>


/*
*File: MHO_FringeRotation.hh
*Class: MHO_FringeRotation
*Author: J. Barrett
*Email: barrettj@mit.edu
*Date:
*Description: collection of static math functions
*/

namespace hops 
{

class MHO_FringeRotation 
{
    public:
        MHO_FringeRotation();
        virtual ~MHO_FringeRotation(){};

        //virtual function (can be pointed to different implementations)
        virtual std::complex<double> vrot(double time_delta, double freq, double ref_freq, double dr, double mbd) const;

        void SetSBDSeparation(double sbd_sep){fSBDSep = sbd_sep;}; //value of the delta between points in SBD space
        void SetSBDMax(double sbd_max){fSBDMax = sbd_max;} //sbd delay at maximum
        void SetSBDMaxBin(int sbd_max_bin){fSBDMaxBin = sbd_max_bin;}
        void SetNSBDBins(int n_sbd_bins){fNSBDBins = n_sbd_bins;}
        //pass the sideband information if optimize_closure of single-sideband correction is needed
        void SetSideband(int sb){fSideband = sb;} //1 is USB, 0 if DSB, -1 if LSB

        void SetOptimizeClosureTrue(){fOptimizeClosure = true;}
        void SetOptimizeClosureFalse(){fOptimizeClosure = false;}
        
    private:

        static const std::complex<double> fImagUnit;

        std::complex<double> vrot_v1(double time_delta, double freq, double ref_freq, double dr, double mbd) const;
        
        double calc_sideband_correction(double mbd) const;
        
        int fSideband;
        int fNSBDBins;
        int fSBDMaxBin;
        double fSBDMax;
        double fSBDSep;
        bool fOptimizeClosure;

};

}

#endif /* end of include guard: MHO_FringeRotation_HH__ */
