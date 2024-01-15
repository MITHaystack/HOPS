#ifndef MHO_PolProductSummation_HH__
#define MHO_PolProductSummation_HH__

/*
*File: MHO_PolProductSummation.hh
*Class: MHO_PolProductSummation
*Author:
*Email:
*Date:
*Description:
*/

#include <cmath>
#include <complex>
#include <vector>
#include <map>
#include <cctype>

#include "MHO_Message.hh"
#include "MHO_Constants.hh"

#include "MHO_TableContainer.hh"
#include "MHO_ContainerDefinitions.hh"
#include "MHO_UnaryOperator.hh"

#include "MHO_Reducer.hh"


namespace hops
{


class MHO_PolProductSummation: public MHO_UnaryOperator< visibility_type >
{
    public:

        MHO_PolProductSummation();
        virtual ~MHO_PolProductSummation();

        void SetPolProductSumLabel(std::string ppl){fSummedPolProdLabel = ppl;}
        void SetPolProductSet(std::vector< std::string >& pp_vec){ fPolProductSet = pp_vec;};

    protected:

        virtual bool InitializeInPlace(visibility_type* in) override;
        virtual bool InitializeOutOfPlace(const visibility_type* in, visibility_type* out) override;

        virtual bool ExecuteInPlace(visibility_type* in) override;
        virtual bool ExecuteOutOfPlace(const visibility_type* in, visibility_type* out) override;

    private:

        //class which does the summation
        MHO_Reducer< visibility_type, MHO_CompoundSum> fReducer;

        //multiplies each pol product by the appropriate pre-factor
        void PreMultiply(visibility_type* in);

        std::complex<double> GetPrefactor(std::string pp_label);

        void FixLabels(visibility_type* in);

        std::string fSummedPolProdLabel;
        std::vector< std::string > fPolProductSet;

};


}


#endif /* end of include guard: MHO_PolProductSummation */
