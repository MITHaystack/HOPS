#ifndef MHO_AdhocPhaseCorrection_HH__
#define MHO_AdhocPhaseCorrection_HH__

#include <complex>
#include <string>
#include <vector>

#include "MHO_Clock.hh"
#include "MHO_Constants.hh"
#include "MHO_Message.hh"

#include "MHO_ContainerDefinitions.hh"
#include "MHO_TableContainer.hh"
#include "MHO_UnaryOperator.hh"

namespace hops
{

/*!
 *@file MHO_AdhocPhaseCorrection.hh
 *@class MHO_AdhocPhaseCorrection
 *@author J. Barrett - barrettj@mit.edu
 *@date
 *@brief Port of the legacy fourfit 'adhoc_phase' feature. Applies a
 * time- and (for file mode) channel-dependent phase correction
 * exp(-i*zeta) to the visibility data. Three modes are supported,
 * mirroring the legacy SINEWAVE, POLYNOMIAL, and PHYLE constants.
 *
 * Reference time (fTRef) and polynomial/sinewave time arguments are
 * measured in seconds relative to most recent hour, which is read
 * from the "start" tag of the visibility object during Initialize.
 *
 * //DIRECTLY FROM <vhelp fourfit>
 *
 *    adhoc_phase        'sinewave', 'polynomial', or 'file'
 *    adhoc_period  For ad hoc sinewave model; the period in integer seconds.
 *    adhoc_amp      "   "  "     "       "    amplitude in degrees of phase.
 *    adhoc_tref    For both ad hoc phase models; the reference time in seconds
 *              past the most recent hour.
 *    adhoc_poly For the ad hoc phase polynomial model; From 1-6 coefficients
 *              describing a power-series model in time. (deg/sec^n)
 *    adhoc_file Name of the file containing phases in the ad hoc file mode.
 *    adhoc_file_chans String of channel labels for phases (columns) in the
 *              ad hoc file.
 *
 * File (PHYLE) mode: two optional ASCII files, one per station
 * (reference and remote). Each data line contains:
 *   <t_fpday>  <phase_ch0_deg>  <phase_ch1_deg>  ...
 * where t_fpday is fractional days since beginning of year and phases
 * are in degrees. The column order is defined by the corresponding
 * fAhFileChans string (one character per column matching the fourfit
 * channel-label / freq-code, e.g. "abcdef"). The applied correction
 * is the differential phase: phase_ref - phase_rem (same sign
 * convention as the legacy diff_file_phase function).
 */

/**
 * @brief Ad hoc phase correction mode, mirrors legacy SINEWAVE/POLYNOMIAL/PHYLE constants.
 */
enum class AdhocPhaseMode
{
    NONE = 0,
    SINEWAVE = 1,
    POLYNOMIAL = 2,
    PHYLE = 3
};

/**
 * @brief Class MHO_AdhocPhaseCorrection
 */
class MHO_AdhocPhaseCorrection: public MHO_UnaryOperator< visibility_type >
{
    public:
        MHO_AdhocPhaseCorrection();
        virtual ~MHO_AdhocPhaseCorrection();

        // ---------------------------------------------------------------
        // Mode
        // ---------------------------------------------------------------

        /**
         * @brief Set the correction mode (NONE, SINEWAVE, POLYNOMIAL, or PHYLE).
         */
        void SetMode(AdhocPhaseMode mode) { fMode = mode; }

        /**
         * @brief Get the currently configured correction mode.
         */
        AdhocPhaseMode GetMode() const { return fMode; }

        // ---------------------------------------------------------------
        // SINEWAVE and POLYNOMIAL shared parameter
        // ---------------------------------------------------------------

        /**
         * @brief Set the reference time used in SINEWAVE and POLYNOMIAL modes.
         *
         * @param tref_sec_from_scan_start Reference time in seconds measured
         *        from the scan start time. The time argument passed to the
         *        sinewave/polynomial model is (ap_center_time - tref).
         */
        void SetTRef(double tref_sec_from_scan_start) { fTRef = tref_sec_from_scan_start; }

        /**
         * @brief Get the reference time in seconds from scan start.
         */
        double GetTRef() const { return fTRef; }

        // ---------------------------------------------------------------
        // SINEWAVE parameters
        // ---------------------------------------------------------------

        /**
         * @brief Set the sinewave period in seconds.
         */
        void SetPeriod(double period_sec) { fPeriod = period_sec; }

        /**
         * @brief Get the sinewave period in seconds.
         */
        double GetPeriod() const { return fPeriod; }

        /**
         * @brief Set the sinewave amplitude in radians.
         */
        void SetAmplitude(double amp_rad) { fAmplitude = amp_rad; }

        /**
         * @brief Get the sinewave amplitude in radians.
         */
        double GetAmplitude() const { return fAmplitude; }

        // ---------------------------------------------------------------
        // POLYNOMIAL parameters
        // ---------------------------------------------------------------

        /**
         * @brief Set polynomial coefficients for POLYNOMIAL mode.
         *
         * @param coeffs Coefficients c_0..c_N where
         *        zeta = c_0 + c_1*t + c_2*t^2 + ... (t in seconds from tref).
         *        Up to 6 coefficients are used; excess entries are ignored,
         *        missing entries default to zero.
         */
        void SetPolynomialCoeffs(const std::vector< double >& coeffs);

        /**
         * @brief Get the polynomial coefficients (always 6 entries).
         */
        const double* GetPolynomialCoeffs() const { return fPolyCoeffs; }

        // ---------------------------------------------------------------
        // PHYLE parameters
        // ---------------------------------------------------------------

        /**
         * @brief Set the adhoc phase file for the reference station.
         *
         * @param filename Path to the adhoc phase file.
         * @param chans    String of fourfit channel-label characters whose
         *                 columns appear in the file (e.g. "abcde").
         */
        void SetRefAdhocFile(const std::string& filename, const std::string& chans);

        /**
         * @brief Get the adhoc phase file info for the reference station.
         */
        void GetRefAdhocFile(std::string& filename, std::string& chans) const;

        /**
         * @brief Set the adhoc phase file for the remote station.
         *
         * @param filename Path to the adhoc phase file.
         * @param chans    String of fourfit channel-label characters whose
         *                 columns appear in the file (e.g. "abcde").
         */
        void SetRemAdhocFile(const std::string& filename, const std::string& chans);

        /**
         * @brief Get the adhoc phase file info for the remote station.
         */
        void GetRemAdhocFile(std::string& filename, std::string& chans) const;

    protected:
        virtual bool InitializeInPlace(visibility_type* in) override;
        virtual bool InitializeOutOfPlace(const visibility_type* in, visibility_type* out) override;
        virtual bool ExecuteInPlace(visibility_type* in) override;
        virtual bool ExecuteOutOfPlace(const visibility_type* in, visibility_type* out) override;

    private:
        /**
         * @brief Open, parse, and store the adhoc phase file for station stn_idx (0=ref, 1=rem).
         *
         * @return true if the file was read successfully (or the filename is empty).
         */
        bool LoadAdhocFile(std::size_t stn_idx);

        /**
         * @brief Compute the phase correction zeta (radians) for the given
         *        channel label and AP centre time.
         *
         * @param chan_label  Fourfit channel label / freq-code character(s).
         * @param ap_center_sec  AP centre time in seconds from scan start.
         * @return zeta in radians. The caller applies exp(-i*zeta).
         */
        double ComputeZeta(const std::string& chan_label, double ap_center_sec) const;

        /**
         * @brief Linearly interpolate the file-based phase for one station.
         *
         * @param stn_idx  0 = reference, 1 = remote.
         * @param fcode    Fourfit channel-label character to look up.
         * @param t_fpday  AP centre time as fractional days since BOY.
         * @return Interpolated phase in radians (0.0 if file absent or channel not found).
         */
        double InterpolateFilePhase(std::size_t stn_idx, char fcode, double t_fpday) const;

        // ---------------------------------------------------------------
        // Configuration
        // ---------------------------------------------------------------
        AdhocPhaseMode fMode;

        // SINEWAVE / POLYNOMIAL shared reference time (seconds from scan start)
        double fTRef;

        // SINEWAVE
        double fPeriod;    // seconds
        double fAmplitude; // radians

        // POLYNOMIAL (6 coefficients, units: rad / s^n)
        double fPolyCoeffs[6];

        // PHYLE: filenames and channel-code strings for [ref=0, rem=1]
        std::string fAhFile[2];
        std::string fAhFileChans[2];

        // Parsed PHYLE data. fFileData[stn] is a row-major flat vector:
        //   row k occupies indices [k*(nchan+1) .. (k+1)*(nchan+1))
        //   column 0 is the time (fpday); columns 1..nchan are phases (degrees).
        std::vector< double > fFileData[2];
        std::size_t fNFileRows[2]; // number of data rows read per station
        std::size_t fNFileCols[2]; // number of columns per row (= nchan + 1)

        // ---------------------------------------------------------------
        // Scan timing (filled during Initialize)
        // ---------------------------------------------------------------
        double fScanStartFpDay;       // scan start as fractional days since BOY
        double fScanStartSecPastHour; //scan start in seconds past the most recent hour
        int fScanYear;                // year of scan start
        double fAccPeriod;            // accumulation period duration in seconds

        // ---------------------------------------------------------------
        // Constants / key strings
        // ---------------------------------------------------------------
        std::complex< double > fImagUnit;
        std::string fChannelLabelKey; // "channel_label"
        std::string fStartKey;        // "start"
};

} // namespace hops

#endif /*! end of include guard: MHO_AdhocPhaseCorrection_HH__ */
