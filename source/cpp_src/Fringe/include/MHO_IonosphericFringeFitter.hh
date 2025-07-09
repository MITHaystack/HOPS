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

/**
 * @brief Class MHO_IonosphericFringeFitter
 */
class MHO_IonosphericFringeFitter: public MHO_BasicFringeFitter
{

    public:
        MHO_IonosphericFringeFitter(MHO_FringeData* data);
        virtual ~MHO_IonosphericFringeFitter();

        /**
         * @brief Runs fringe fitting process with additial search over dTEC (with optional smoothing)
         * @note This is a virtual function.
         */
        virtual void Run() override;
        
        /**
         * @brief Finalizes fringe fitting process by storing search windows and generating plot data.
         */
        void Finalize() override;

        /**
         * @brief Accepts and invokes a visitor to visit this object.
         * 
         * @param visitor A pointer to an MHO_FringeFitterVisitor that will be invoked.
         * @note This is a virtual function.
         */
        virtual void Accept(MHO_FringeFitterVisitor* visitor) override 
        {
            visitor->Visit(this);
        };

    protected:
        /**
         * @brief Searches for fringe peak in dtec/delay/delay-rate space.
         * 
         * @return 0 on success, 1 on failure.
         */
        int rjc_ion_search();
        
        /**
         * @brief Sorts TEC arrays and stores them in parameter store.
         * 
         * @param nion Number of ionospheric search points
         * @param dtec Reference to vector of vectors containing TEC values
         */
        void sort_tecs(int nion, std::vector< std::vector< double > >& dtec);
        
        /**
         * @brief Searches for fringe peak while applying smoothing function (convolution w/ half-cosine).
         * 
         * @return 0 if successful, 1 otherwise.
         */
        int ion_search_smooth();
        
        /**
         * @brief Applies a smoothing curve (half-cosine) to input data array for fringe finding in ionospheric processing.
         * 
         * @param f Input data array with arbitrary positive length
         * @param g Output data array with fourfold interpolation
         * @param tec_step Grid spacing of f in TEC units
         * @param npts Pointer to length of input array - modified!
         */
        void smoother(double* f, double* g, double* tec_step, int* npts);
        
        /**
         * @brief Calculates approximate signal-to-noise ratio (SNR) for fringe fitting.
         * 
         * @return Approximate SNR as a double
         */
        double calculate_approx_snr();

        double fInitialSBWin[2]; //save the initial SBD window

        int ion_npts;
};

} // namespace hops

#endif /*! end of include guard: MHO_IonosphericFringeFitter_HH__ */
