#ifndef MHO_DCBlock_HH__
#define MHO_DCBlock_HH__

#include <cctype>
#include <cmath>
#include <complex>
#include <map>
#include <vector>

#include "MHO_Constants.hh"
#include "MHO_Message.hh"

#include "MHO_ContainerDefinitions.hh"
#include "MHO_TableContainer.hh"
#include "MHO_UnaryOperator.hh"

namespace hops
{

/*!
 *@file MHO_DCBlock.hh
 *@class MHO_DCBlock
 *@author J. Barrett - barrettj@mit.edu
 *@date
 *@brief Tue Apr  2 09:41:24 AM EDT 2024
 */

class MHO_DCBlock: public MHO_UnaryOperator< visibility_type >
{
    public:
        MHO_DCBlock();
        virtual ~MHO_DCBlock();

    protected:
        virtual bool InitializeInPlace(visibility_type* in) override;
        virtual bool InitializeOutOfPlace(const visibility_type* in, visibility_type* out) override;

        virtual bool ExecuteInPlace(visibility_type* in) override;
        virtual bool ExecuteOutOfPlace(const visibility_type* in, visibility_type* out) override;

    private:
        std::string fSidebandLabelKey;
        std::string fLowerSideband;
        std::string fUpperSideband;
};

} // namespace hops

#endif /*! end of include guard: MHO_DCBlock */
