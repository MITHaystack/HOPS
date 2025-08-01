#ifndef MHO_Operator_HH__
#define MHO_Operator_HH__

#include <string>

namespace hops
{

/*!
 *@file  MHO_Operator.hh
 *@class  MHO_Operator
 *@author  J. Barrett - barrettj@mit.edu
 *@date Thu Sep 23 16:03:48 2021 -0400
 *@brief abstract base class for operators
 */

/**
 * @brief Class MHO_Operator
 */
class MHO_Operator
{
    public:
        MHO_Operator();
        virtual ~MHO_Operator();

        /**
         * @brief Function Initialize
         *
         * @return Return value (bool)
         * @note This is a virtual function.
         */
        virtual bool Initialize() = 0;
        /**
         * @brief Function Execute
         *
         * @return Return value (bool)
         * @note This is a virtual function.
         */
        virtual bool Execute() = 0;

        /**
         * @brief Setter for operator name
         *
         * @param name New name to set
         */
        void SetName(std::string name) { fName = name; }

        /**
         * @brief Getter for operator name
         *
         * @return The stored name as a std::string
         */
        std::string GetName() const { return fName; }

        //allow priority to vary
        /**
         * @brief Setter for operator priority (determines order of execution within a operator category)
         *
         * @param priority New priority value to be assigned to fPriority field (double)
         * @note This is a virtual function.
         * @details a higher value for the fPriority field implies this operator should happend after other operators with lower priority values
         * order of execution goes from low to high
         */
        virtual void SetPriority(const double& priority) { fPriority = priority; }

        /**
         * @brief Get the the priority field value
         *
         * @return Return value (double)
         * @note This is a virtual function.
         */
        virtual double Priority() const { return fPriority; }

    private:
        std::string fName;
        double fPriority;
};

} // namespace hops

#endif /*! end of include guard: MHO_Operator */
