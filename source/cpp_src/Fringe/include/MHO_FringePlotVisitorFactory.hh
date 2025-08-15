#ifndef MHO_FringePlotVisitorFactory_HH__
#define MHO_FringePlotVisitorFactory_HH__

//global messaging util
#include "MHO_FringePlotVisitor.hh"
#include "MHO_Message.hh"

namespace hops
{

/*!
 *@file MHO_FringePlotVisitorFactory.hh
 *@class MHO_FringePlotVisitorFactory
 *@author J. Barrettj - barrettj@mit.edu
 *@date Sun Oct 13 08:05:51 PM EDT 2024
 *@brief Fringe fitter factory, builds the appropriate underlying fitter type, and returns it as a MHO_FringeFitter*
 */

/**
 * @brief Class MHO_FringePlotVisitorFactory
 */
class MHO_FringePlotVisitorFactory
{
    public:
        MHO_FringePlotVisitorFactory();
        virtual ~MHO_FringePlotVisitorFactory();

        /**
         * @brief Constructs and configures an MHO_FringePlotVisitor instance based on the current configuration.
         * @return MHO_FringePlotVisitor* - The constructed fringe fitter
         */
        MHO_FringePlotVisitor* ConstructPlotter();

    protected:
        MHO_FringePlotVisitor* fFringePlotter;
};

} // namespace hops

#endif /*! end of include guard: MHO_FringePlotVisitorFactory_HH__ */
