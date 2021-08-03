#ifndef MHO_NormFX_HH__
#define MHO_NormFX_HH__

/*
*File: MHO_NormFX.hh
*Class: MHO_NormFX
*Author: J. Barrett
*Email: barrettj@mit.edu
*Date:
*Description:
*/

#include <cmath>
#include <complex>

//dependencies from old hops
extern "C"
{
    #include <stdio.h>
    #include <math.h>
    #include <fftw3.h>
    #include "mk4_data.h"
    #include "param_struct.h"
    #include "pass_struct.h"
}


#include "MHO_FFTWTypes.hh"
#include "MHO_TableContainer.hh"
#include "MHO_ChannelizedVisibilities.hh"
#include "MHO_BinaryNDArrayOperator.hh"

namespace hops
{


class MHO_NormFX: public MHO_BinaryNDArrayOperator<
    ch_baseline_data_type,
    ch_baseline_weight_type,
    ch_baseline_sbd_type >
{
    public:
        MHO_NormFX();
        virtual ~MHO_NormFX();

        virtual bool Initialize() override;
        virtual bool ExecuteOperation() override;

    private:


        // //this version of the function will gradually get modified
        // //until we can move functionality out of it entirely and
        // //make it more modular
        void new_norm_fx(struct type_pass *pass,
                         struct type_param* param,
                         struct type_status* status,
                         int fr, int ap){};

        //private data structures to store what were 'extern'/globals
        //in the old code
        fftw_plan fftplan;
        hops_complex xp_spec[4*MAXLAG];
        hops_complex xcor[4*MAXLAG], S[4*MAXLAG], xlag[4*MAXLAG];

};


}


#endif /* end of include guard: MHO_NormFX */
