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
        virtual ~MHO_InterpolateFringePeak();

        virtual bool Initialize() override;
        virtual bool Execute() override;

    private:

        int fMBDMaxBin;
        int fDRMaxBin;
        int fSBDMaxBin;
    
        double fRefFreq;
        visibility_type fSBDArray;
        weight fWeights;
    
        time_axis_type* fMBDAxis;
        delay_rate_axis_type* fDRAxis;

        void fine_peak_interpolation();

        //copy of max555.c impl
        void max555 (double drf[5][5][5], double xlim[3][2], double xi[3], double *drfmax);  

};


}


#endif /* end of include guard: MHO_InterpolateFringePeak_HH__ */
