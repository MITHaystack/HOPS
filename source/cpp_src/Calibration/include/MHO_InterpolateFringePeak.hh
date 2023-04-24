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
        MHO_InterpolateFringePeak();
        virtual ~MHO_InterpolateFringePeak(){};

        void SetReferenceFrequency(double ref_freq){fRefFreq = ref_freq;}
        void SetMaxBins(int sbd_max, int mbd_max, int dr_max);

        void SetSBDArray(visibility_type* sbd_arr){fSBDArray = sbd_arr;}
        void SetWeights(weight_type* weights){fWeights = weights;}

        void SetMBDAxis(const time_axis_type* mbd_ax){fMBDAxis.Copy(*mbd_ax);}
        void SetDRAxis(const delay_rate_axis_type* dr_ax){fDRAxis.Copy(*dr_ax);}

        virtual bool Initialize() override;
        virtual bool Execute() override;

        double GetSBDelay() const {return fSBDelay;}
        double GetMBDelay() const {return fMBDelay;}
        double GetDelayRate() const {return fDelayRate;}

    private:

        int fMBDMaxBin;
        int fDRMaxBin;
        int fSBDMaxBin;

        double fRefFreq;
        double fTotalSummedWeights;
        visibility_type* fSBDArray;
        weight_type* fWeights;

        time_axis_type fMBDAxis;
        delay_rate_axis_type fDRAxis;

        void fine_peak_interpolation();

        MHO_NDArrayWrapper<double, 3> fDRF;

        double fSBDelay;
        double fMBDelay;
        double fDelayRate;

        //copy of max555.c impl
        void max555 (MHO_NDArrayWrapper<double, 3>&, double xlim[3][2], double xi[3], double *drfmax);
        void interp555 (MHO_NDArrayWrapper<double, 3>&, double xi[3], double *drfval);
        double dwin (double, double, double);
};


}


#endif /* end of include guard: MHO_InterpolateFringePeak_HH__ */
