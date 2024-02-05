#ifndef MHO_IonosphericFringeFitter_HH__
#define MHO_IonosphericFringeFitter_HH__

#include "MHO_BasicFringeFitter.hh"
#include "MHO_Tokenizer.hh"

/*
*File: MHO_IonosphericFringeFitter.hh
*Class: MHO_IonosphericFringeFitter
*Author:
*Email:
*Date: Tue Sep 19 04:11:24 PM EDT 2023
*Description: single-baseline fringe fitter with ionosphere search
*/

namespace hops
{

class MHO_IonosphericFringeFitter: public MHO_BasicFringeFitter
{

    public:
        MHO_IonosphericFringeFitter(MHO_FringeData* data);
        virtual ~MHO_IonosphericFringeFitter();

        virtual void Run() override;

    protected:

        int rjc_ion_search();
        void sort_tecs(int nion, double dtec[][2]);

};

}//end namespace

#endif /* end of include guard: MHO_IonosphericFringeFitter_HH__ */
