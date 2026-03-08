#ifndef MHO_IonosphericFringeFitterOpenMP_HH__
#define MHO_IonosphericFringeFitterOpenMP_HH__

#include <memory>
#include <vector>

#include "MHO_BasicFringeFitter.hh"
#include "MHO_IonosphericPhaseCorrection.hh"
#include "MHO_Tokenizer.hh"

#ifdef _OPENMP
    #include <omp.h>
#endif

namespace hops
{

struct IonLoopResult
{
    double famp     = 0.0;
    double ion_diff = 0.0;
};

/*!
 *@file MHO_IonosphericFringeFitterOpenMP.hh
 *@class MHO_IonosphericFringeFitterOpenMP
 *@author J. Barrettj - barrettj@mit.edu
 *@date Wed Jan 17 14:48:52 2024 -0500
 *@brief single-baseline fringe fitter with ionosphere search, experimentally augmented with OpenMP
 */

/**
 * @brief Class MHO_IonosphericFringeFitterOpenMP
 */
class MHO_IonosphericFringeFitterOpenMP: public MHO_BasicFringeFitter
{

    public:
        MHO_IonosphericFringeFitterOpenMP(MHO_FringeData* data);
        virtual ~MHO_IonosphericFringeFitterOpenMP();

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
        virtual void Accept(MHO_FringeFitterVisitor* visitor) override { visitor->Visit(this); };

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

        //per-thread infrastructure for parallel ion search
        int fNIonThreads;
        visibility_type fVisRef; //clean reference copy of vis_data (before any ion correction)

        std::vector< visibility_type > fPerThreadVis; //per-thread working vis copies
        std::vector< visibility_type > fPerThreadSBD; //per-thread SBD output buffers

        std::vector< std::unique_ptr< MHO_SingleSidebandNormFX > > fPerThreadSSBNormFX;
        std::vector< std::unique_ptr< MHO_MixedSidebandNormFX > >  fPerThreadMSBNormFX;
        std::vector< MHO_NormFX* >                                  fPerThreadNormFXPtr;

        std::vector< std::unique_ptr< MHO_MBDelaySearch > >                    fPerThreadMBDSearch;
        std::vector< std::unique_ptr< MHO_InterpolateFringePeakOptimized > >   fPerThreadPeakInterp;
        std::vector< std::unique_ptr< MHO_IonosphericPhaseCorrection > >       fPerThreadIono;

        std::vector< IonLoopResult > fIonLoopResults; //one slot per ionloop index

    private:
        void initialize_ion_threads();
};

} // namespace hops

#endif /*! end of include guard: MHO_IonosphericFringeFitterOpenMP_HH__ */
