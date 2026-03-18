#ifndef MHO_OutputVisitorFactory_HH__
#define MHO_OutputVisitorFactory_HH__

#include <map>

#include "MHO_Message.hh"
#include "MHO_FringeFitter.hh"


namespace hops
{

/*!
 *@file MHO_OutputVisitorFactory.hh
 *@class MHO_OutputVisitorFactory
 *@author J. Barrettj - barrettj@mit.edu
 *@date Sun Oct 13 08:05:51 PM EDT 2024
 *@brief Fringe fitter factory, builds the appropriate underlying fitter type, and returns it as a MHO_FringeFitter*
 */

/**
 * @brief Class MHO_OutputVisitorFactory
 */
class MHO_OutputVisitorFactory
{
    public:
        MHO_OutputVisitorFactory();
        virtual ~MHO_OutputVisitorFactory();

        /**
         * @brief Constructs and configures an MHO_FringePlotVisitor instance based on the current configuration.
         * @return MHO_FringePlotVisitor* - The constructed fringe fitter
         */
        MHO_FringeFitterVisitor* GetOutputVisitor(std::string format = "");

    protected:

        std::map< std::string, MHO_FringeFitterVisitor* > fOutputVisitors;
};

} // namespace hops

#endif /*! end of include guard: MHO_OutputVisitorFactory_HH__ */
