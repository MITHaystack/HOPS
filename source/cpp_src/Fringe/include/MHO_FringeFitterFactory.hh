#ifndef MHO_FringeFitterFactory_HH__
#define MHO_FringeFitterFactory_HH__

//global messaging util
#include "MHO_Message.hh"
#include "MHO_FringeData.hh"
#include "MHO_FringeFitter.hh"

namespace hops
{

/*!
 *@file MHO_FringeFitterFactory.hh
 *@class MHO_FringeFitterFactory
 *@author J. Barrettj - barrettj@mit.edu
 *@date Sun Oct 13 08:05:51 PM EDT 2024
 *@brief Fringe fitter factory, builds the appropriate underlying fitter type, and returns it as a MHO_FringeFitter*
 */

/**
 * @brief Class MHO_FringeFitterFactory
 */
class MHO_FringeFitterFactory
{
    public:
        MHO_FringeFitterFactory(MHO_FringeData* data);

        virtual ~MHO_FringeFitterFactory();

        /**
         * @brief Constructs and configures an MHO_FringeFitter instance based on configuration.
         * the underlying object returned can be a MHO_BasicFringeFitter or MHO_IonosphericFringeFitter (other types will be introduced in the future)
         * 
         * @return MHO_FringeFitter* - The constructed fringe fitter
         */
        MHO_FringeFitter* ConstructFringeFitter();

    protected:

        MHO_FringeData* fFringeData;
        MHO_FringeFitter* fFringeFitter;

};

} // namespace hops

#endif /*! end of include guard: MHO_FringeFitterFactory_HH__ */
