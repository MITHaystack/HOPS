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
 *@brief
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
         * @brief Setter for name
         * 
         * @param name New name to set
         */
        void SetName(std::string name) { fName = name; }

        /**
         * @brief Getter for name
         * 
         * @return The stored name as a std::string
         */
        std::string GetName() const { return fName; }

        //allow priority to vary
        /**
         * @brief Setter for priority
         * 
         * @param priority New priority value to be assigned to fPriority field.
         * @note This is a virtual function.
         */
        virtual void SetPriority(const double& priority) { fPriority = priority; }

        //a higher value for the fPriority field implies a lower priority
        //for this operator in the order of execution
        /**
         * @brief Sets the priority field with a higher value implying lower priority.
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
