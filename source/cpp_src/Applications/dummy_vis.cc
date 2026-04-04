#include <string>
#include <vector>

//option parsing and help text library
#include "CLI11.hpp"

#include "MHO_Message.hh"
#include "MHO_ParameterStore.hh"
#include "MHO_Tokenizer.hh"

#include "MHO_ContainerDefinitions.hh"
#include "MHO_GaussianWhiteNoiseSignal.hh"
#include "MHO_FringeRotation.hh"


#include <cmath>
#include <complex>
#include <vector>

using namespace hops;

// using visibility_element_type = std::complex<double>;
// using visibility_type = MHO_TableContainer< visibility_element_type, baseline_axis_pack >;

void GenerateSimulatedVisibilities(
    // Output container
    visibility_type& vis,
    // Signal parameters
    double fringe_amplitude,        // correlated flux (arbitrary units)
    double residual_delay_usec,     // microseconds
    double residual_delay_rate,     // microseconds/second
    double residual_phase_rad,      // radians
    // Frequency setup
    const std::vector<double>& channel_freqs_mhz,  // channel center frequencies in MHz
    double channel_width_mhz, // channel bandwidth in MHz
    int num_subchannels,  // FFT bins per channel
    // Time setup
    int num_aps,
    double ap_length_sec,  // seconds
    double start_time_sec, // seconds
    // Noise
    double system_noise_rms,
    int random_seed,
    double ref_freq_mhz //reference frequency
)
{
    int num_polprods = 1;  // single pol product for now
    int num_channels = channel_freqs_mhz.size();

    // =========================================================
    // 1. Allocate and resize the visibility container
    // =========================================================
    std::size_t dims[VIS_NDIM];
    dims[POLPROD_AXIS] = num_polprods;
    dims[CHANNEL_AXIS] = num_channels;
    dims[TIME_AXIS]    = num_aps;
    dims[FREQ_AXIS]    = num_subchannels;
    vis.Resize(dims);

    // =========================================================
    // 2. Fill the axis values
    // =========================================================

    // Polprod axis (just index 0 for now)
    auto polprod_axis = &(std::get< POLPROD_AXIS >(vis));
    polprod_axis->at(0) = "XX";

    // Channel axis - store channel center frequencies (MHz)
    auto chan_axis = &(std::get< CHANNEL_AXIS >(vis));
    for(int ch = 0; ch < num_channels; ++ch)
    {
        chan_axis->at(ch) = channel_freqs_mhz[ch];
    }

    // Time axis, APs
    auto time_axis = &(std::get< TIME_AXIS >(vis));
    for(int ap = 0; ap < num_aps; ++ap)
    {
        time_axis->at(ap) = ap * ap_length_sec;
    }

    // Frequency (sub-channel) axis - offsets from channel center (MHz)
    // Sub-channels span [-channel_width/2, +channel_width/2)
    auto freq_axis = &(std::get< FREQ_AXIS >(vis));
    double subchan_width_mhz = channel_width_mhz / static_cast<double>(num_subchannels);
    for(int sc = 0; sc < num_subchannels; ++sc)
    {
        freq_axis->at(sc) = -channel_width_mhz / 2.0 + (sc + 0.5) * subchan_width_mhz;
    }

    // =========================================================
    // 3. Set up the noise generators
    // =========================================================

    // Correlated source signal (real and imaginary parts independently)
    MHO_GaussianWhiteNoiseSignal source_noise_re;
    source_noise_re.SetMean(0.0);
    source_noise_re.SetStandardDeviation(fringe_amplitude / std::sqrt(2.0));
    source_noise_re.SetRandomSeed(random_seed);
    source_noise_re.Initialize();

    MHO_GaussianWhiteNoiseSignal source_noise_im;
    source_noise_im.SetMean(0.0);
    source_noise_im.SetStandardDeviation(fringe_amplitude / std::sqrt(2.0));
    source_noise_im.SetRandomSeed(random_seed + 1);  // different seed!
    source_noise_im.Initialize();

    // Uncorrelated system noise (real and imaginary parts independently)
    MHO_GaussianWhiteNoiseSignal system_noise_re;
    system_noise_re.SetMean(0.0);
    system_noise_re.SetStandardDeviation(system_noise_rms / std::sqrt(2.0));
    system_noise_re.SetRandomSeed(random_seed + 2);
    system_noise_re.Initialize();

    MHO_GaussianWhiteNoiseSignal system_noise_im;
    system_noise_im.SetMean(0.0);
    system_noise_im.SetStandardDeviation(system_noise_rms / std::sqrt(2.0));
    system_noise_im.SetRandomSeed(random_seed + 3);
    system_noise_im.Initialize();


    // =========================================================
    // 4. Set up the fringe rotation object
    // =========================================================
    //
    // vrot() computes the CORRECTION phasor:
    //   vrot = exp(-2*pi*i * (freq*dr*time_delta + mbd*(freq - ref_freq)))
    //
    // For a simulation, we want to apply the delay/rate effects,
    // which is the INVERSE of the correction, i.e. conj(vrot).
    //
    // The residual phase offset (phi_0) is not handled by vrot,
    // so we apply it as a separate constant phasor.

    MHO_FringeRotation frot;

    // Constant phasor for residual phase offset
    std::complex<double> phase_offset_phasor(
        std::cos(residual_phase_rad),
        std::sin(residual_phase_rad)
    );

    // =========================================================
    // 5. Fill the visibility array
    // =========================================================
    // Physical model:
    //   V(nu, t) = S(nu,t) * exp(2*pi*i*(nu*tau + nu*tau_dot*t + phi_0)) + N(nu,t)
    //
    // where:
    //   S(nu,t)  = correlated broadband source signal (complex Gaussian)
    //   tau      = residual delay (microseconds)
    //   tau_dot  = residual delay-rate (microseconds/second)
    //   phi_0    = residual phase (radians)
    //   N(nu,t)  = uncorrelated system noise (complex Gaussian)
    //   nu       = frequency (MHz)
    //
    // Note on units:
    //   nu [MHz] * tau [usec] = [1e6 Hz] * [1e-6 s] = [cycles] (dimensionless)
    //   so the product nu*tau is already in units of cycles, and
    //   2*pi*nu*tau gives radians directly. No other conversion needed.

    double dummy_time = 0.0;  // GetSample ignores time for white noise
    double samp_re, samp_im;
    double sys_re, sys_im;

    for(int pp = 0; pp < num_polprods; ++pp)
    {
        for(int ch = 0; ch < num_channels; ++ch)
        {
            double chan_center_mhz = channel_freqs_mhz[ch];

            for(int ap = 0; ap < num_aps; ++ap)
            {
                double time_delta = time_axis->at(ap);  // AP midpoint time as time_delta

                for(int sc = 0; sc < num_subchannels; ++sc)
                {
                    // Absolute frequency of this sub-channel (MHz)
                    double freq_mhz = chan_center_mhz + freq_axis->at(sc);

                    // -------------------------------------------------
                    // Step A: Generate correlated source signal sample
                    // -------------------------------------------------
                    source_noise_re.GetSample(dummy_time, samp_re);
                    source_noise_im.GetSample(dummy_time, samp_im);
                    std::complex<double> source_signal(samp_re, samp_im);

                    // -------------------------------------------------
                    // Step B: Get fringe rotation phasor and conjugate
                    //         to APPLY (rather than remove) the effects
                    // -------------------------------------------------
                    std::complex<double> correction = frot.vrot(
                        time_delta,           // time delta (seconds)
                        freq_mhz,             // sub-channel frequency (MHz)
                        ref_freq_mhz,         // reference frequency (MHz)
                        residual_delay_rate,   // delay rate (us/s)
                        residual_delay_usec    // multi-band delay (us)
                    );

                    // conj(vrot) inverts the correction, applying the
                    // delay/rate signature to the source signal
                    std::complex<double> inverse_rotation = std::conj(correction);

                    // Combine: source * inverse_rotation * phase_offset
                    std::complex<double> correlated_vis =
                        source_signal * inverse_rotation * phase_offset_phasor;

                    // -------------------------------------------------
                    // Step C: Add uncorrelated system noise
                    // -------------------------------------------------
                    system_noise_re.GetSample(dummy_time, sys_re);
                    system_noise_im.GetSample(dummy_time, sys_im);
                    std::complex<double> noise(sys_re, sys_im);

                    // -------------------------------------------------
                    // Step D: Store in container
                    // -------------------------------------------------
                    vis(pp, ch, ap, sc) = correlated_vis + noise;
                }
            }
        }
    }

}


int main(int argc, char** argv)
{
    CLI::App app{"dummy_vis"};

    double fringe_amplitude = 1.0;
    std::string output_file = "simulated_vis.dat";
    double residual_delay = 0.0;  // microseconds
    double residual_delay_rate = 0.0;  // microseconds/second
    double residual_phase = 0.0;  // degrees
    int num_channels = 16;
    double channel_width = 32.0;  // MHz
    std::vector<double> channel_frequencies;  // MHz
    double ref_freq_mhz = 10000.0;
    double start_freq = 8000.0;  // MHz
    int num_subchannels = 32;
    int num_aps = 120;
    double ap_length = 2.0;  // seconds
    double start_time = 0.0;  // seconds
    double snr = 10.0;
    double system_noise_rms = -1.0;  // if negative, will be computed from SNR
    int random_seed = -1;  // -1 means use time-based seed
    std::string polprod = "XX";
    bool binary_output = false;
    int message_level = 0;
    std::vector< std::string > message_categories;

    // Remove help flag because it shortcuts all processing
    app.set_help_flag();
    auto* help = app.add_flag("-h,--help", "print this help message and exit");


    app.add_option("-o,--output", output_file, "name of output file for simulated visibilities");
    app.add_option("-A,--amplitude", fringe_amplitude, "fringe amplitude (correlated flux density)")->default_val(1.0);
    app.add_option("-d,--delay", residual_delay, "residual delay in microseconds")->default_val(0.0);
    app.add_option("-r,--delay-rate", residual_delay_rate, "residual delay-rate in microseconds/second")->default_val(0.0);
    app.add_option("-p,--phase", residual_phase, "residual phase offset in degrees")->default_val(0.0);
    app.add_option("-c,--channels", num_channels, "total number of frequency channels")->default_val(16);
    app.add_option("-w,--channel-width", channel_width, "channel width in MHz")->default_val(32.0);
    app.add_option("-R,--reference-frequency", ref_freq_mhz, "reference frequency in MHz")->default_val(10000.0);
    app.add_option("-f,--frequencies", channel_frequencies,
        "channel center frequencies in MHz (comma-separated). If not specified, will be auto-generated")->delimiter(',');
    app.add_option("-F,--start-freq", start_freq, "starting frequency in MHz (if frequencies not explicitly given)")
        ->default_val(8000.0);
    app.add_option("-s,--subchannels", num_subchannels, "number of sub-channels (lags) per channel")->default_val(32);
    app.add_option("-n,--num-aps", num_aps, "total number of accumulation periods")->default_val(120);
    app.add_option("-t,--ap-length", ap_length, "accumulation period length in seconds")->default_val(2.0);
    app.add_option("-T,--start-time", start_time, "start time in seconds")->default_val(0.0);
    app.add_option("--snr", snr, "signal-to-noise ratio")->default_val(10.0);
    app.add_option("--noise-rms", system_noise_rms,"system noise RMS (if specified, overrides SNR parameter)");
    app.add_option("--seed", random_seed, "random number generator seed (-1 for time-based)")->default_val(-1);
    app.add_option("-P,--polprod", polprod, "polarization product (e.g., XX, YY, XY, RR, LL)")->default_val("XX");

    app.add_option("-M,--message-categories", message_categories,
        "message categories to enable (comma-separated)")
        ->delimiter(',');
    app.add_option("-m,--message-level", message_level,
        "message level to be used, range: -2 (debug) to 5 (silent)")
        ->default_val(0);

    try
    {
        app.parse(argc, argv);
        if(*help)
        {
            throw CLI::CallForHelp();
        }
    }
    catch(const CLI::Error& e)
    {
        std::cout << app.help() << std::endl;
        std::exit(1);
    }

    //clamp message level
    if(message_level > 5)
    {
        message_level = 5;
    }
    if(message_level < -2)
    {
        message_level = -2;
    }
    MHO_Message::GetInstance().AcceptAllKeys();
    MHO_Message::GetInstance().SetLegacyMessageLevel(message_level);

    //check if any message categories were passed, if so, we limit the messages
    //to only those categories
    if(message_categories.size() != 0)
    {
        for(std::size_t m = 0; m < message_categories.size(); m++)
        {
            MHO_Message::GetInstance().AddKey(message_categories[m]);
        }
        MHO_Message::GetInstance().LimitToKeySet();
    }
    //set the message level
    MHO_Message::GetInstance().SetLegacyMessageLevel(message_level);

    // Auto-generate channel frequencies if not provided
    if(channel_frequencies.empty())
    {
        for(int i = 0; i < num_channels; ++i)
        {
            channel_frequencies.push_back(start_freq + i * channel_width);
        }
    }
    else
    {
        // Validate that we have the right number
        if(channel_frequencies.size() != static_cast<size_t>(num_channels))
        {
            std::cerr << "Warning: " << channel_frequencies.size()
                      << " frequencies specified but " << num_channels
                      << " channels requested. Using " << channel_frequencies.size()
                      << " channels." << std::endl;
            num_channels = channel_frequencies.size();
        }
    }

    // Calculate system noise RMS if not specified
    if(system_noise_rms < 0.0)
    {
        system_noise_rms = fringe_amplitude / snr;
        std::cout << "System noise RMS calculated from SNR: "
                  << system_noise_rms << std::endl;

    }

    // Convert phase from degrees to radians for internal use
    double residual_phase_rad = residual_phase * M_PI / 180.0;

    std::cout << "\n=== Simulation Parameters ===" << std::endl;
    std::cout << "Fringe amplitude: " << fringe_amplitude << std::endl;
    std::cout << "Residual delay: " << residual_delay << " μs" << std::endl;
    std::cout << "Residual delay-rate: " << residual_delay_rate << " μs/s" << std::endl;
    std::cout << "Residual phase: " << residual_phase << " deg" << std::endl;
    std::cout << "Channels: " << num_channels << std::endl;
    std::cout << "Sub-channels: " << num_subchannels << std::endl;
    std::cout << "Channel width: " << channel_width << " MHz" << std::endl;
    std::cout << "APs: " << num_aps << " × " << ap_length << " s" << std::endl;
    std::cout << "SNR: " << snr << std::endl;
    std::cout << "System noise RMS: " << system_noise_rms << std::endl;
    std::cout << "Polarization: " << polprod << std::endl;
    std::cout << "Output file: " << output_file << std::endl;
    std::cout << "============================\n" << std::endl;

    // generate_visibilities(fringe_amplitude, residual_delay, residual_delay_rate,
    //                       residual_phase_rad, channel_frequencies, channel_width,
    //                       num_subchannels, num_aps, ap_length, start_time,
    //                       system_noise_rms, random_seed, polprod, output_file);























    return 0;
}
