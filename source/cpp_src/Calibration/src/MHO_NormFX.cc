#include "MHO_NormFX.hh"

namespace hops
{


MHO_NormFX::MHO_NormFX(){};

MHO_NormFX::~MHO_NormFX(){};

bool
MHO_NormFX::Initialize()
{
    //for the time being we are going to construct some fake parameters
    //to control the process so we can test this operation on the new
    //data containers, to do this we mainly need to build:
    //(1) 'pass' struct
    //(2) 'param' struct w/ control block
    //(3) 'status' struct


    // struct type_120 *t120;
    // struct freq_corel *fdata;
    // struct data_corel *datum;
    // int sb, st, i, rev_i, j, l, m;
    // static int nlags = 0;
    // int ip, ips;
    // static complex xp_spec[4*MAXLAG];
    // static complex xcor[4*MAXLAG], S[4*MAXLAG], xlag[4*MAXLAG];
    // complex z;
    // double factor, mean;
    // double diff_delay, deltaf, polcof, polcof_sum, phase_shift, dpar;
    // int freq_no,
    //     ibegin,
    //     sindex,
    //     pol,
    //     pols,                       // bit-mapped pols to be processed in this pass
    //     usb_present, lsb_present,
    //     usb_bypol[4],lsb_bypol[4],
    //     lastpol[2];                 // last pol index with data present, by sideband
    // int datum_uflag, datum_lflag;
    // int stnpol[2][4] = {0, 1, 0, 1, 0, 1, 1, 0}; // [stn][pol] = 0:L/X/H, 1:R/Y/V
    // static fftw_plan fftplan;
    //
    // extern struct type_param param;
    // extern struct type_status status;


}

bool
MHO_NormFX::ExecuteOperation()
{

}













} //end of namespace
