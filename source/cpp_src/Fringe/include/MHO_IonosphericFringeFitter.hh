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
 *@date Wed Jan 17 14:48:52 2024 -0500
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
        void sort_tecs(int nion, std::vector< std::vector<double> >& dtec);
        int ion_search_smooth();
        void smoother(double* f, double* g, double* tec_step, int* npts);

        double fInitialSBWin[2]; //save the initial SBD window

        int ion_npts;
};

} // namespace hops

#endif /*! end of include guard: MHO_IonosphericFringeFitter_HH__ */
