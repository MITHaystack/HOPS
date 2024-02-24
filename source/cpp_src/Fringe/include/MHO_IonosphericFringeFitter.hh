#ifndef MHO_IonosphericFringeFitter_HH__
#define MHO_IonosphericFringeFitter_HH__

#include "MHO_BasicFringeFitter.hh"
#include "MHO_Tokenizer.hh"


namespace hops
{


/*!
*@file MHO_IonosphericFringeFitter.hh
*@class MHO_IonosphericFringeFitter
*@author J. Barrettj - barrettj@mit.edu
*@date Tue Sep 19 04:11:24 PM EDT 2023
*@brief single-baseline fringe fitter with ionosphere search
*/

class MHO_IonosphericFringeFitter: public MHO_BasicFringeFitter
{

    public:
        MHO_IonosphericFringeFitter(MHO_FringeData* data);
        virtual ~MHO_IonosphericFringeFitter();

        virtual void Run() override;
        void Finalize() override;

    protected:

        int rjc_ion_search();
        void sort_tecs(int nion, double dtec[][2]);
        int ion_search_smooth();
        void smoother (double *f, double *g, double *tec_step, int *npts);

        double fInitialSBWin[2]; //save the initial SBD window

};

}//end namespace

#endif /*! end of include guard: MHO_IonosphericFringeFitter_HH__ */
