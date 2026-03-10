#ifndef MHO_SpectralLineFringeFitter_HH__
#define MHO_SpectralLineFringeFitter_HH__

#include "MHO_ContainerDefinitions.hh"
#include "MHO_FringeFitter.hh"

//initialization
#include "MHO_OperatorBuilderManager.hh"
#include "MHO_ParameterConfigurator.hh"
#include "MHO_ParameterManager.hh"

//spectral line search operators
#include "MHO_InterpolateSpectralLinePeak.hh"
#include "MHO_SpectralLineFringeSearch.hh"

namespace hops
{

/*!
 *@file MHO_SpectralLineFringeFitter.hh
 *@class MHO_SpectralLineFringeFitter
 *@author J. Barrett - barrettj@mit.edu
 *@date
 *@brief Fringe fitter for narrow-band (spectral-line) VLBI data.
 *
 * Rather than the broadband (SBD, MBD, delay-rate) search used by
 * MHO_BasicFringeFitter, this fitter works in
 * (channel, intra-channel frequency bin, delay rate) space.
 *
 * The pipeline is:
 *   Configure  -> load data, build operators
 *   Initialize -> apply flagging / calibration operators
 *   PreRun     -> run prefit operators, init search operators
 *   Run        -> coarse (channel x DR x freq) search, fine interpolation
 *   PostRun    -> run postfit operators
 *   Finalize   -> store search windows, build plot data
 *
 * Assumes the polarisation-product axis has size 1 (single pol product or
 * pre-summed coherent combination).
 *
 * New parameter-store keys set by this fitter (in addition to the standard
 * keys shared with MHO_BasicFringeFitter):
 *   /fringe/is_spectral_line        (bool)   = true
 *   /fringe/peak_channel_idx        (int)    channel index of fringe peak
 *   /fringe/peak_freq_bin           (int)    intra-channel freq bin of fringe peak
 *   /fringe/peak_spectral_freq      (double) sky frequency of peak (MHz)
 *   /fringe/fringe_phase            (double) fringe phase at peak (radians)
 *
 * Standard keys that are set with spectral-line-appropriate values:
 *   /fringe/sbdelay   = 0                      (undefined for narrow-band)
 *   /fringe/mbdelay   = phase delay (us)       (NOT group delay)
 *   /fringe/drate     = delay rate (sec/sec)
 *   /fringe/frate     = fringe rate (Hz)
 *   /fringe/famp      = normalised fringe amplitude
 */

/**
 * @brief Class MHO_SpectralLineFringeFitter
 */
class MHO_SpectralLineFringeFitter: public MHO_FringeFitter
{
    public:
        MHO_SpectralLineFringeFitter(MHO_FringeData* data);
        virtual ~MHO_SpectralLineFringeFitter();

        virtual void Configure() override;
        virtual void Initialize() override;
        virtual void PreRun() override;
        virtual void Run() override;
        virtual void PostRun() override;
        virtual void Finalize() override;
        virtual bool IsFinished() override;

        virtual void Accept(MHO_FringeFitterVisitor* visitor) override { visitor->Visit(this); }

    protected:
        // Caching mechanism (identical to MHO_BasicFringeFitter).
        bool fEnableCaching;
        virtual void Cache() override;
        virtual void Refresh() override;

        // Raw visibility / weight pointers (into the container store).
        visibility_type* vis_data;
        weight_type* wt_data;

        // Ovex/root-file JSON.
        mho_json fVexInfo;

    private:
        void coarse_spectral_line_search();
        void interpolate_spectral_line_peak();

        // Spectral-line fringe search and peak interpolator.
        MHO_SpectralLineFringeSearch fSLFringeSearch;
        MHO_InterpolateSpectralLinePeak fSLPeakInterpolator;
};

} // namespace hops

#endif /*! end of include guard: MHO_SpectralLineFringeFitter_HH__ */
