#ifndef MHO_InterpolateFringePeak_HH__
#define MHO_InterpolateFringePeak_HH__


/*
*File: MHO_DelayRate.hh
*Class: MHO_DelayRate
*Author: J. Barrett
*Email: barrettj@mit.edu
*Date:
*Description:
*/

#include <cmath>
#include <complex>

#include "MHO_Operator.hh"
#include "MHO_ContainerDefinitions.hh"

namespace hops
{

class MHO_InterpolateFringePeak: public MHO_Operator
{
    public:
        MHO_InterpolateFringePeak(){};
        virtual ~MHO_InterpolateFringePeak(){};

        void SetReferenceFrequency(double ref_freq){fRefFreq = ref_freq;}
        void SetMaxBins(int sbd_max, int mbd_max, int dr_max);

        void SetSBDArray(visibility_type* sbd_arr){fSBDArray = sbd_arr;}
        void SetWeights(weight_type* weights){fWeights = weights;}

        void SetMBDAxis(time_axis_type* mbd_ax){fMBDAxis = mbd_ax;}
        void SetDRAxis(delay_rate_axis_type* dr_ax){fDRAxis = dr_ax;}

        virtual bool Initialize() override;
        virtual bool Execute() override;

    private:

        int fMBDMaxBin;
        int fDRMaxBin;
        int fSBDMaxBin;
    
        double fRefFreq;
        visibility_type* fSBDArray;
        weight_type* fWeights;
    
        time_axis_type* fMBDAxis;
        delay_rate_axis_type* fDRAxis;

        void fine_peak_interpolation();

        //copy of max555.c impl
        void max555 (double drf[5][5][5], double xlim[3][2], double xi[3], double *drfmax);  
        void interp555 (double drf[5][5][5], double xi[3], double *drfval);
        double dwin (double, double, double);
};


}


#endif /* end of include guard: MHO_InterpolateFringePeak_HH__ */
