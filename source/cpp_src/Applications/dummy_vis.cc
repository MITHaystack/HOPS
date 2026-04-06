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

#include "MHO_BinaryFileInterface.hh"
#include "MHO_ObjectTags.hh"

#include "MHO_Clock.hh"


#include <cmath>
#include <complex>
#include <vector>

using namespace hops;

// using visibility_element_type = std::complex<double>;
// using visibility_type = MHO_TableContainer< visibility_element_type, baseline_axis_pack >;


std::string GetVexString()
{
    std::string dummy_vex =
"{\"$ANTENNA\":{\"DUMMY_A\":{\"antenna_diam\":{\"units\":\"m\",\"value\":12.0},\"antenna_motion\":[{\"axis_type\":\"az\",\"settling_time\":{\"units\":\"sec\",\"value\":0.0},\"slew_rate\":{\"units\":\"deg/min\",\"value\":300.0}},{\"axis_type\":\"el\",\"settling_time\":{\"units\":\"sec\",\"value\":0.0},\"slew_rate\":{\"units\":\"deg/min\",\"value\":66.0}}],\"axis_offset\":{\"axis_type\":\"el\",\"offset\":{\"units\":\"m\",\"value\":0.0}},\"axis_type\":{\"primary_axis\":\"az\",\"secondary_axis\":\"el\"},\"pointing_sector\":[{\"axis1_lower_limit\":{\"units\":\"deg\",\"value\":180.0},\"axis1_type\":\"az\",\"axis1_upper_limit\":{\"units\":\"deg\",\"value\":720.0},\"axis2_lower_limit\":{\"units\":\"deg\",\"value\":6.5},\"axis2_type\":\"el\",\"axis2_upper_limit\":{\"units\":\"deg\",\"value\":88.0},\"sector_id\":\"&n\"}]},\"DUMMY_B\":{\"antenna_diam\":{\"units\":\"m\",\"value\":12.0},\"antenna_motion\":[{\"axis_type\":\"az\",\"settling_time\":{\"units\":\"sec\",\"value\":0.0},\"slew_rate\":{\"units\":\"deg/min\",\"value\":720.0}},{\"axis_type\":\"el\",\"settling_time\":{\"units\":\"sec\",\"value\":0.0},\"slew_rate\":{\"units\":\"deg/min\",\"value\":300.0}}],\"axis_offset\":{\"axis_type\":\"el\",\"offset\":{\"units\":\"m\",\"value\":0.0}},\"axis_type\":{\"primary_axis\":\"az\",\"secondary_axis\":\"el\"},\"pointing_sector\":[{\"axis1_lower_limit\":{\"units\":\"deg\",\"value\":180.0},\"axis1_type\":\"az\",\"axis1_upper_limit\":{\"units\":\"deg\",\"value\":720.0},\"axis2_lower_limit\":{\"units\":\"deg\",\"value\":6.5},\"axis2_type\":\"el\",\"axis2_upper_limit\":{\"units\":\"deg\",\"value\":88.0},\"sector_id\":\"&n\"}]}},\"$BBC\":{\"VGOS_std\":{\"BBC_assign\":[{\"logical_bbc_id\":\"&BBC01\",\"logical_if\":\"&IF_1N\",\"physical_bbc_id\":1},{\"logical_bbc_id\":\"&BBC02\",\"logical_if\":\"&IF_1N\",\"physical_bbc_id\":2},{\"logical_bbc_id\":\"&BBC03\",\"logical_if\":\"&IF_1N\",\"physical_bbc_id\":3},{\"logical_bbc_id\":\"&BBC04\",\"logical_if\":\"&IF_1N\",\"physical_bbc_id\":4},{\"logical_bbc_id\":\"&BBC05\",\"logical_if\":\"&IF_1N\",\"physical_bbc_id\":5},{\"logical_bbc_id\":\"&BBC06\",\"logical_if\":\"&IF_1N\",\"physical_bbc_id\":6},{\"logical_bbc_id\":\"&BBC07\",\"logical_if\":\"&IF_1N\",\"physical_bbc_id\":7},{\"logical_bbc_id\":\"&BBC08\",\"logical_if\":\"&IF_1N\",\"physical_bbc_id\":8},{\"logical_bbc_id\":\"&BBC09\",\"logical_if\":\"&IF_3N\",\"physical_bbc_id\":9},{\"logical_bbc_id\":\"&BBC10\",\"logical_if\":\"&IF_3N\",\"physical_bbc_id\":10},{\"logical_bbc_id\":\"&BBC11\",\"logical_if\":\"&IF_3N\",\"physical_bbc_id\":11},{\"logical_bbc_id\":\"&BBC12\",\"logical_if\":\"&IF_3N\",\"physical_bbc_id\":12},{\"logical_bbc_id\":\"&BBC13\",\"logical_if\":\"&IF_3N\",\"physical_bbc_id\":13},{\"logical_bbc_id\":\"&BBC14\",\"logical_if\":\"&IF_3N\",\"physical_bbc_id\":14},{\"logical_bbc_id\":\"&BBC15\",\"logical_if\":\"&IF_3N\",\"physical_bbc_id\":15},{\"logical_bbc_id\":\"&BBC16\",\"logical_if\":\"&IF_3N\",\"physical_bbc_id\":16},{\"logical_bbc_id\":\"&BBC17\",\"logical_if\":\"&IF_1N\",\"physical_bbc_id\":1},{\"logical_bbc_id\":\"&BBC18\",\"logical_if\":\"&IF_1N\",\"physical_bbc_id\":2},{\"logical_bbc_id\":\"&BBC19\",\"logical_if\":\"&IF_1N\",\"physical_bbc_id\":3},{\"logical_bbc_id\":\"&BBC20\",\"logical_if\":\"&IF_1N\",\"physical_bbc_id\":4},{\"logical_bbc_id\":\"&BBC21\",\"logical_if\":\"&IF_1N\",\"physical_bbc_id\":5},{\"logical_bbc_id\":\"&BBC22\",\"logical_if\":\"&IF_1N\",\"physical_bbc_id\":6},{\"logical_bbc_id\":\"&BBC23\",\"logical_if\":\"&IF_1N\",\"physical_bbc_id\":7},{\"logical_bbc_id\":\"&BBC24\",\"logical_if\":\"&IF_1N\",\"physical_bbc_id\":8},{\"logical_bbc_id\":\"&BBC25\",\"logical_if\":\"&IF_3N\",\"physical_bbc_id\":9},{\"logical_bbc_id\":\"&BBC26\",\"logical_if\":\"&IF_3N\",\"physical_bbc_id\":10},{\"logical_bbc_id\":\"&BBC27\",\"logical_if\":\"&IF_3N\",\"physical_bbc_id\":11},{\"logical_bbc_id\":\"&BBC28\",\"logical_if\":\"&IF_3N\",\"physical_bbc_id\":12},{\"logical_bbc_id\":\"&BBC29\",\"logical_if\":\"&IF_3N\",\"physical_bbc_id\":13},{\"logical_bbc_id\":\"&BBC30\",\"logical_if\":\"&IF_3N\",\"physical_bbc_id\":14},{\"logical_bbc_id\":\"&BBC31\",\"logical_if\":\"&IF_3N\",\"physical_bbc_id\":15},{\"logical_bbc_id\":\"&BBC32\",\"logical_if\":\"&IF_3N\",\"physical_bbc_id\":16},{\"logical_bbc_id\":\"&BBC33\",\"logical_if\":\"&IF_1N\",\"physical_bbc_id\":1},{\"logical_bbc_id\":\"&BBC34\",\"logical_if\":\"&IF_1N\",\"physical_bbc_id\":2},{\"logical_bbc_id\":\"&BBC35\",\"logical_if\":\"&IF_1N\",\"physical_bbc_id\":3},{\"logical_bbc_id\":\"&BBC36\",\"logical_if\":\"&IF_1N\",\"physical_bbc_id\":4},{\"logical_bbc_id\":\"&BBC37\",\"logical_if\":\"&IF_1N\",\"physical_bbc_id\":5},{\"logical_bbc_id\":\"&BBC38\",\"logical_if\":\"&IF_1N\",\"physical_bbc_id\":6},{\"logical_bbc_id\":\"&BBC39\",\"logical_if\":\"&IF_1N\",\"physical_bbc_id\":7},{\"logical_bbc_id\":\"&BBC40\",\"logical_if\":\"&IF_1N\",\"physical_bbc_id\":8},{\"logical_bbc_id\":\"&BBC41\",\"logical_if\":\"&IF_3N\",\"physical_bbc_id\":9},{\"logical_bbc_id\":\"&BBC42\",\"logical_if\":\"&IF_3N\",\"physical_bbc_id\":10},{\"logical_bbc_id\":\"&BBC43\",\"logical_if\":\"&IF_3N\",\"physical_bbc_id\":11},{\"logical_bbc_id\":\"&BBC44\",\"logical_if\":\"&IF_3N\",\"physical_bbc_id\":12},{\"logical_bbc_id\":\"&BBC45\",\"logical_if\":\"&IF_3N\",\"physical_bbc_id\":13},{\"logical_bbc_id\":\"&BBC46\",\"logical_if\":\"&IF_3N\",\"physical_bbc_id\":14},{\"logical_bbc_id\":\"&BBC47\",\"logical_if\":\"&IF_3N\",\"physical_bbc_id\":15},{\"logical_bbc_id\":\"&BBC48\",\"logical_if\":\"&IF_3N\",\"physical_bbc_id\":16},{\"logical_bbc_id\":\"&BBC49\",\"logical_if\":\"&IF_1N\",\"physical_bbc_id\":1},{\"logical_bbc_id\":\"&BBC50\",\"logical_if\":\"&IF_1N\",\"physical_bbc_id\":2},{\"logical_bbc_id\":\"&BBC51\",\"logical_if\":\"&IF_1N\",\"physical_bbc_id\":3},{\"logical_bbc_id\":\"&BBC52\",\"logical_if\":\"&IF_1N\",\"physical_bbc_id\":4},{\"logical_bbc_id\":\"&BBC53\",\"logical_if\":\"&IF_1N\",\"physical_bbc_id\":5},{\"logical_bbc_id\":\"&BBC54\",\"logical_if\":\"&IF_1N\",\"physical_bbc_id\":6},{\"logical_bbc_id\":\"&BBC55\",\"logical_if\":\"&IF_1N\",\"physical_bbc_id\":7},{\"logical_bbc_id\":\"&BBC56\",\"logical_if\":\"&IF_1N\",\"physical_bbc_id\":8},{\"logical_bbc_id\":\"&BBC57\",\"logical_if\":\"&IF_3N\",\"physical_bbc_id\":9},{\"logical_bbc_id\":\"&BBC58\",\"logical_if\":\"&IF_3N\",\"physical_bbc_id\":10},{\"logical_bbc_id\":\"&BBC59\",\"logical_if\":\"&IF_3N\",\"physical_bbc_id\":11},{\"logical_bbc_id\":\"&BBC60\",\"logical_if\":\"&IF_3N\",\"physical_bbc_id\":12},{\"logical_bbc_id\":\"&BBC61\",\"logical_if\":\"&IF_3N\",\"physical_bbc_id\":13},{\"logical_bbc_id\":\"&BBC62\",\"logical_if\":\"&IF_3N\",\"physical_bbc_id\":14},{\"logical_bbc_id\":\"&BBC63\",\"logical_if\":\"&IF_3N\",\"physical_bbc_id\":15},{\"logical_bbc_id\":\"&BBC64\",\"logical_if\":\"&IF_3N\",\"physical_bbc_id\":16}]}},\"$CLOCK\":{\"Aa\":{\"clock_early\":[{\"clock_early_offset\":{\"units\":\"usec\",\"value\":0.0},\"clock_rate\":{\"value\":0.0},\"origin_epoch\":\"2025y001d00h00m0s\",\"start_validity_epoch\":\"2025y001d00h00m\"}]},\"Bb\":{\"clock_early\":[{\"clock_early_offset\":{\"units\":\"usec\",\"value\":0.0},\"clock_rate\":{\"value\":0.0},\"origin_epoch\":\"2025y001d00h00m0s\",\"start_validity_epoch\":\"2025y001d00h00m\"}]}},\"$CORR_INIT\":{\"corr_init_std\":{\"$PBS_INIT\":[{\"keyword\":\"PBS_DUMMY\"}],\"bocf_period\":160000,\"system_tempo\":1.0}},\"$DAS\":{\"DBBC_DDC_rack\":{},\"FlexBuff_recorder\":{},\"Aa_Aa\":{},\"Bb_Bb\":{},\"Mark5B_recorder\":{},\"Mark6_recorder\":{},\"Oe_Oe\":{},\"RDBE_rack\":{},\"Wf_07\":{},\"Ws_WS\":{},\"Yj_Yj\":{}},\"$EOP\":{\"EOP003\":{\"A1-TAI\":{\"units\":\"sec\",\"value\":0.03439},\"TAI-UTC\":{\"units\":\"sec\",\"value\":37.0},\"eop_interval\":{\"units\":\"hr\",\"value\":24.0},\"eop_ref_epoch\":\"2025y001d\",\"num_eop_points\":5,\"ut1-utc\":{\"units\":\"sec\",\"values\":[-0.133051,-0.134035,-0.135226,-0.136583,-0.138049]},\"x_wobble\":{\"units\":\"asec\",\"values\":[0.05503,0.0553,0.05586,0.05674,0.05778]},\"y_wobble\":{\"units\":\"asec\",\"values\":[0.39823,0.39908,0.39992,0.4006,0.40113]}}},\"$EVEX\":{\"evex_std\":{\"$CORR_CONFIG\":[{\"keyword\":\"CDUM\"}],\"$SU_CONFIG\":[{\"keyword\":\"SDUM\"}],\"AP_length\":{\"units\":\"sec\",\"value\":1.0},\"corr_exp#\":1111,\"cvex_file\":\"dummy\",\"ovex_file\":\"dummy\",\"speedup_factor\":{\"units\":\"\",\"value\":1.0},\"svex_file\":\"dummy\"}},\"$EXPER\":{\"VT9105\":{\"PI_name\":\"HAYS\",\"exper_description\":\"\\\"VGOS broadband session\\\"\",\"exper_name\":\"VT9105\",\"exper_nominal_start\":\"2025y001d00h00m00s\",\"exper_nominal_stop\":\"2019y105d23h59m59s\",\"exper_num\":1111,\"target_correlator\":\"difx\"}},\"$FREQ\":{\"VGOS_std\":{\"chan_def\":[{\"band_id\":\"&X\",\"bandwidth\":{\"units\":\"MHz\",\"value\":32.0},\"bbc_id\":\"&BBC01\",\"channel_id\":\"&Ch01\",\"channel_name\":\"X07LX\",\"net_sideband\":\"L\",\"phase_cal_id\":\"&L_cal\",\"sky_frequency\":{\"units\":\"MHz\",\"value\":3480.4}},{\"band_id\":\"&X\",\"bandwidth\":{\"units\":\"MHz\",\"value\":32.0},\"bbc_id\":\"&BBC02\",\"channel_id\":\"&Ch02\",\"channel_name\":\"X06LX\",\"net_sideband\":\"L\",\"phase_cal_id\":\"&L_cal\",\"sky_frequency\":{\"units\":\"MHz\",\"value\":3448.4}},{\"band_id\":\"&X\",\"bandwidth\":{\"units\":\"MHz\",\"value\":32.0},\"bbc_id\":\"&BBC03\",\"channel_id\":\"&Ch03\",\"channel_name\":\"X05LX\",\"net_sideband\":\"L\",\"phase_cal_id\":\"&L_cal\",\"sky_frequency\":{\"units\":\"MHz\",\"value\":3384.4}},{\"band_id\":\"&X\",\"bandwidth\":{\"units\":\"MHz\",\"value\":32.0},\"bbc_id\":\"&BBC04\",\"channel_id\":\"&Ch04\",\"channel_name\":\"X04LX\",\"net_sideband\":\"L\",\"phase_cal_id\":\"&L_cal\",\"sky_frequency\":{\"units\":\"MHz\",\"value\":3320.4}},{\"band_id\":\"&X\",\"bandwidth\":{\"units\":\"MHz\",\"value\":32.0},\"bbc_id\":\"&BBC05\",\"channel_id\":\"&Ch05\",\"channel_name\":\"X03LX\",\"net_sideband\":\"L\",\"phase_cal_id\":\"&L_cal\",\"sky_frequency\":{\"units\":\"MHz\",\"value\":3224.4}},{\"band_id\":\"&X\",\"bandwidth\":{\"units\":\"MHz\",\"value\":32.0},\"bbc_id\":\"&BBC06\",\"channel_id\":\"&Ch06\",\"channel_name\":\"X02LX\",\"net_sideband\":\"L\",\"phase_cal_id\":\"&L_cal\",\"sky_frequency\":{\"units\":\"MHz\",\"value\":3096.4}},{\"band_id\":\"&X\",\"bandwidth\":{\"units\":\"MHz\",\"value\":32.0},\"bbc_id\":\"&BBC07\",\"channel_id\":\"&Ch07\",\"channel_name\":\"X01LX\",\"net_sideband\":\"L\",\"phase_cal_id\":\"&L_cal\",\"sky_frequency\":{\"units\":\"MHz\",\"value\":3064.4}},{\"band_id\":\"&X\",\"bandwidth\":{\"units\":\"MHz\",\"value\":32.0},\"bbc_id\":\"&BBC08\",\"channel_id\":\"&Ch08\",\"channel_name\":\"X00LX\",\"net_sideband\":\"L\",\"phase_cal_id\":\"&L_cal\",\"sky_frequency\":{\"units\":\"MHz\",\"value\":3032.4}},{\"band_id\":\"&X\",\"bandwidth\":{\"units\":\"MHz\",\"value\":32.0},\"bbc_id\":\"&BBC09\",\"channel_id\":\"&Ch09\",\"channel_name\":\"X07LY\",\"net_sideband\":\"L\",\"phase_cal_id\":\"&L_cal\",\"sky_frequency\":{\"units\":\"MHz\",\"value\":3480.4}},{\"band_id\":\"&X\",\"bandwidth\":{\"units\":\"MHz\",\"value\":32.0},\"bbc_id\":\"&BBC10\",\"channel_id\":\"&Ch10\",\"channel_name\":\"X06LY\",\"net_sideband\":\"L\",\"phase_cal_id\":\"&L_cal\",\"sky_frequency\":{\"units\":\"MHz\",\"value\":3448.4}},{\"band_id\":\"&X\",\"bandwidth\":{\"units\":\"MHz\",\"value\":32.0},\"bbc_id\":\"&BBC11\",\"channel_id\":\"&Ch11\",\"channel_name\":\"X05LY\",\"net_sideband\":\"L\",\"phase_cal_id\":\"&L_cal\",\"sky_frequency\":{\"units\":\"MHz\",\"value\":3384.4}},{\"band_id\":\"&X\",\"bandwidth\":{\"units\":\"MHz\",\"value\":32.0},\"bbc_id\":\"&BBC12\",\"channel_id\":\"&Ch12\",\"channel_name\":\"X04LY\",\"net_sideband\":\"L\",\"phase_cal_id\":\"&L_cal\",\"sky_frequency\":{\"units\":\"MHz\",\"value\":3320.4}},{\"band_id\":\"&X\",\"bandwidth\":{\"units\":\"MHz\",\"value\":32.0},\"bbc_id\":\"&BBC13\",\"channel_id\":\"&Ch13\",\"channel_name\":\"X03LY\",\"net_sideband\":\"L\",\"phase_cal_id\":\"&L_cal\",\"sky_frequency\":{\"units\":\"MHz\",\"value\":3224.4}},{\"band_id\":\"&X\",\"bandwidth\":{\"units\":\"MHz\",\"value\":32.0},\"bbc_id\":\"&BBC14\",\"channel_id\":\"&Ch14\",\"channel_name\":\"X02LY\",\"net_sideband\":\"L\",\"phase_cal_id\":\"&L_cal\",\"sky_frequency\":{\"units\":\"MHz\",\"value\":3096.4}},{\"band_id\":\"&X\",\"bandwidth\":{\"units\":\"MHz\",\"value\":32.0},\"bbc_id\":\"&BBC15\",\"channel_id\":\"&Ch15\",\"channel_name\":\"X01LY\",\"net_sideband\":\"L\",\"phase_cal_id\":\"&L_cal\",\"sky_frequency\":{\"units\":\"MHz\",\"value\":3064.4}},{\"band_id\":\"&X\",\"bandwidth\":{\"units\":\"MHz\",\"value\":32.0},\"bbc_id\":\"&BBC16\",\"channel_id\":\"&Ch16\",\"channel_name\":\"X00LY\",\"net_sideband\":\"L\",\"phase_cal_id\":\"&L_cal\",\"sky_frequency\":{\"units\":\"MHz\",\"value\":3032.4}},{\"band_id\":\"&X\",\"bandwidth\":{\"units\":\"MHz\",\"value\":32.0},\"bbc_id\":\"&BBC17\",\"channel_id\":\"&Ch17\",\"channel_name\":\"X15LX\",\"net_sideband\":\"L\",\"phase_cal_id\":\"&L_cal\",\"sky_frequency\":{\"units\":\"MHz\",\"value\":5720.4}},{\"band_id\":\"&X\",\"bandwidth\":{\"units\":\"MHz\",\"value\":32.0},\"bbc_id\":\"&BBC18\",\"channel_id\":\"&Ch18\",\"channel_name\":\"X14LX\",\"net_sideband\":\"L\",\"phase_cal_id\":\"&L_cal\",\"sky_frequency\":{\"units\":\"MHz\",\"value\":5688.4}},{\"band_id\":\"&X\",\"bandwidth\":{\"units\":\"MHz\",\"value\":32.0},\"bbc_id\":\"&BBC19\",\"channel_id\":\"&Ch19\",\"channel_name\":\"X13LX\",\"net_sideband\":\"L\",\"phase_cal_id\":\"&L_cal\",\"sky_frequency\":{\"units\":\"MHz\",\"value\":5624.4}},{\"band_id\":\"&X\",\"bandwidth\":{\"units\":\"MHz\",\"value\":32.0},\"bbc_id\":\"&BBC20\",\"channel_id\":\"&Ch20\",\"channel_name\":\"X12LX\",\"net_sideband\":\"L\",\"phase_cal_id\":\"&L_cal\",\"sky_frequency\":{\"units\":\"MHz\",\"value\":5560.4}},{\"band_id\":\"&X\",\"bandwidth\":{\"units\":\"MHz\",\"value\":32.0},\"bbc_id\":\"&BBC21\",\"channel_id\":\"&Ch21\",\"channel_name\":\"X11LX\",\"net_sideband\":\"L\",\"phase_cal_id\":\"&L_cal\",\"sky_frequency\":{\"units\":\"MHz\",\"value\":5464.4}},{\"band_id\":\"&X\",\"bandwidth\":{\"units\":\"MHz\",\"value\":32.0},\"bbc_id\":\"&BBC22\",\"channel_id\":\"&Ch22\",\"channel_name\":\"X10LX\",\"net_sideband\":\"L\",\"phase_cal_id\":\"&L_cal\",\"sky_frequency\":{\"units\":\"MHz\",\"value\":5336.4}},{\"band_id\":\"&X\",\"bandwidth\":{\"units\":\"MHz\",\"value\":32.0},\"bbc_id\":\"&BBC23\",\"channel_id\":\"&Ch23\",\"channel_name\":\"X09LX\",\"net_sideband\":\"L\",\"phase_cal_id\":\"&L_cal\",\"sky_frequency\":{\"units\":\"MHz\",\"value\":5304.4}},{\"band_id\":\"&X\",\"bandwidth\":{\"units\":\"MHz\",\"value\":32.0},\"bbc_id\":\"&BBC24\",\"channel_id\":\"&Ch24\",\"channel_name\":\"X08LX\",\"net_sideband\":\"L\",\"phase_cal_id\":\"&L_cal\",\"sky_frequency\":{\"units\":\"MHz\",\"value\":5272.4}},{\"band_id\":\"&X\",\"bandwidth\":{\"units\":\"MHz\",\"value\":32.0},\"bbc_id\":\"&BBC25\",\"channel_id\":\"&Ch25\",\"channel_name\":\"X15LY\",\"net_sideband\":\"L\",\"phase_cal_id\":\"&L_cal\",\"sky_frequency\":{\"units\":\"MHz\",\"value\":5720.4}},{\"band_id\":\"&X\",\"bandwidth\":{\"units\":\"MHz\",\"value\":32.0},\"bbc_id\":\"&BBC26\",\"channel_id\":\"&Ch26\",\"channel_name\":\"X14LY\",\"net_sideband\":\"L\",\"phase_cal_id\":\"&L_cal\",\"sky_frequency\":{\"units\":\"MHz\",\"value\":5688.4}},{\"band_id\":\"&X\",\"bandwidth\":{\"units\":\"MHz\",\"value\":32.0},\"bbc_id\":\"&BBC27\",\"channel_id\":\"&Ch27\",\"channel_name\":\"X13LY\",\"net_sideband\":\"L\",\"phase_cal_id\":\"&L_cal\",\"sky_frequency\":{\"units\":\"MHz\",\"value\":5624.4}},{\"band_id\":\"&X\",\"bandwidth\":{\"units\":\"MHz\",\"value\":32.0},\"bbc_id\":\"&BBC28\",\"channel_id\":\"&Ch28\",\"channel_name\":\"X12LY\",\"net_sideband\":\"L\",\"phase_cal_id\":\"&L_cal\",\"sky_frequency\":{\"units\":\"MHz\",\"value\":5560.4}},{\"band_id\":\"&X\",\"bandwidth\":{\"units\":\"MHz\",\"value\":32.0},\"bbc_id\":\"&BBC29\",\"channel_id\":\"&Ch29\",\"channel_name\":\"X11LY\",\"net_sideband\":\"L\",\"phase_cal_id\":\"&L_cal\",\"sky_frequency\":{\"units\":\"MHz\",\"value\":5464.4}},{\"band_id\":\"&X\",\"bandwidth\":{\"units\":\"MHz\",\"value\":32.0},\"bbc_id\":\"&BBC30\",\"channel_id\":\"&Ch30\",\"channel_name\":\"X10LY\",\"net_sideband\":\"L\",\"phase_cal_id\":\"&L_cal\",\"sky_frequency\":{\"units\":\"MHz\",\"value\":5336.4}},{\"band_id\":\"&X\",\"bandwidth\":{\"units\":\"MHz\",\"value\":32.0},\"bbc_id\":\"&BBC31\",\"channel_id\":\"&Ch31\",\"channel_name\":\"X09LY\",\"net_sideband\":\"L\",\"phase_cal_id\":\"&L_cal\",\"sky_frequency\":{\"units\":\"MHz\",\"value\":5304.4}},{\"band_id\":\"&X\",\"bandwidth\":{\"units\":\"MHz\",\"value\":32.0},\"bbc_id\":\"&BBC32\",\"channel_id\":\"&Ch32\",\"channel_name\":\"X08LY\",\"net_sideband\":\"L\",\"phase_cal_id\":\"&L_cal\",\"sky_frequency\":{\"units\":\"MHz\",\"value\":5272.4}},{\"band_id\":\"&X\",\"bandwidth\":{\"units\":\"MHz\",\"value\":32.0},\"bbc_id\":\"&BBC33\",\"channel_id\":\"&Ch33\",\"channel_name\":\"X23LX\",\"net_sideband\":\"L\",\"phase_cal_id\":\"&L_cal\",\"sky_frequency\":{\"units\":\"MHz\",\"value\":6840.4}},{\"band_id\":\"&X\",\"bandwidth\":{\"units\":\"MHz\",\"value\":32.0},\"bbc_id\":\"&BBC34\",\"channel_id\":\"&Ch34\",\"channel_name\":\"X22LX\",\"net_sideband\":\"L\",\"phase_cal_id\":\"&L_cal\",\"sky_frequency\":{\"units\":\"MHz\",\"value\":6808.4}},{\"band_id\":\"&X\",\"bandwidth\":{\"units\":\"MHz\",\"value\":32.0},\"bbc_id\":\"&BBC35\",\"channel_id\":\"&Ch35\",\"channel_name\":\"X21LX\",\"net_sideband\":\"L\",\"phase_cal_id\":\"&L_cal\",\"sky_frequency\":{\"units\":\"MHz\",\"value\":6744.4}},{\"band_id\":\"&X\",\"bandwidth\":{\"units\":\"MHz\",\"value\":32.0},\"bbc_id\":\"&BBC36\",\"channel_id\":\"&Ch36\",\"channel_name\":\"X20LX\",\"net_sideband\":\"L\",\"phase_cal_id\":\"&L_cal\",\"sky_frequency\":{\"units\":\"MHz\",\"value\":6680.4}},{\"band_id\":\"&X\",\"bandwidth\":{\"units\":\"MHz\",\"value\":32.0},\"bbc_id\":\"&BBC37\",\"channel_id\":\"&Ch37\",\"channel_name\":\"X19LX\",\"net_sideband\":\"L\",\"phase_cal_id\":\"&L_cal\",\"sky_frequency\":{\"units\":\"MHz\",\"value\":6584.4}},{\"band_id\":\"&X\",\"bandwidth\":{\"units\":\"MHz\",\"value\":32.0},\"bbc_id\":\"&BBC38\",\"channel_id\":\"&Ch38\",\"channel_name\":\"X18LX\",\"net_sideband\":\"L\",\"phase_cal_id\":\"&L_cal\",\"sky_frequency\":{\"units\":\"MHz\",\"value\":6456.4}},{\"band_id\":\"&X\",\"bandwidth\":{\"units\":\"MHz\",\"value\":32.0},\"bbc_id\":\"&BBC39\",\"channel_id\":\"&Ch39\",\"channel_name\":\"X17LX\",\"net_sideband\":\"L\",\"phase_cal_id\":\"&L_cal\",\"sky_frequency\":{\"units\":\"MHz\",\"value\":6424.4}},{\"band_id\":\"&X\",\"bandwidth\":{\"units\":\"MHz\",\"value\":32.0},\"bbc_id\":\"&BBC40\",\"channel_id\":\"&Ch40\",\"channel_name\":\"X16LX\",\"net_sideband\":\"L\",\"phase_cal_id\":\"&L_cal\",\"sky_frequency\":{\"units\":\"MHz\",\"value\":6392.4}},{\"band_id\":\"&X\",\"bandwidth\":{\"units\":\"MHz\",\"value\":32.0},\"bbc_id\":\"&BBC41\",\"channel_id\":\"&Ch41\",\"channel_name\":\"X23LY\",\"net_sideband\":\"L\",\"phase_cal_id\":\"&L_cal\",\"sky_frequency\":{\"units\":\"MHz\",\"value\":6840.4}},{\"band_id\":\"&X\",\"bandwidth\":{\"units\":\"MHz\",\"value\":32.0},\"bbc_id\":\"&BBC42\",\"channel_id\":\"&Ch42\",\"channel_name\":\"X22LY\",\"net_sideband\":\"L\",\"phase_cal_id\":\"&L_cal\",\"sky_frequency\":{\"units\":\"MHz\",\"value\":6808.4}},{\"band_id\":\"&X\",\"bandwidth\":{\"units\":\"MHz\",\"value\":32.0},\"bbc_id\":\"&BBC43\",\"channel_id\":\"&Ch43\",\"channel_name\":\"X21LY\",\"net_sideband\":\"L\",\"phase_cal_id\":\"&L_cal\",\"sky_frequency\":{\"units\":\"MHz\",\"value\":6744.4}},{\"band_id\":\"&X\",\"bandwidth\":{\"units\":\"MHz\",\"value\":32.0},\"bbc_id\":\"&BBC44\",\"channel_id\":\"&Ch44\",\"channel_name\":\"X20LY\",\"net_sideband\":\"L\",\"phase_cal_id\":\"&L_cal\",\"sky_frequency\":{\"units\":\"MHz\",\"value\":6680.4}},{\"band_id\":\"&X\",\"bandwidth\":{\"units\":\"MHz\",\"value\":32.0},\"bbc_id\":\"&BBC45\",\"channel_id\":\"&Ch45\",\"channel_name\":\"X19LY\",\"net_sideband\":\"L\",\"phase_cal_id\":\"&L_cal\",\"sky_frequency\":{\"units\":\"MHz\",\"value\":6584.4}},{\"band_id\":\"&X\",\"bandwidth\":{\"units\":\"MHz\",\"value\":32.0},\"bbc_id\":\"&BBC46\",\"channel_id\":\"&Ch46\",\"channel_name\":\"X18LY\",\"net_sideband\":\"L\",\"phase_cal_id\":\"&L_cal\",\"sky_frequency\":{\"units\":\"MHz\",\"value\":6456.4}},{\"band_id\":\"&X\",\"bandwidth\":{\"units\":\"MHz\",\"value\":32.0},\"bbc_id\":\"&BBC47\",\"channel_id\":\"&Ch47\",\"channel_name\":\"X17LY\",\"net_sideband\":\"L\",\"phase_cal_id\":\"&L_cal\",\"sky_frequency\":{\"units\":\"MHz\",\"value\":6424.4}},{\"band_id\":\"&X\",\"bandwidth\":{\"units\":\"MHz\",\"value\":32.0},\"bbc_id\":\"&BBC48\",\"channel_id\":\"&Ch48\",\"channel_name\":\"X16LY\",\"net_sideband\":\"L\",\"phase_cal_id\":\"&L_cal\",\"sky_frequency\":{\"units\":\"MHz\",\"value\":6392.4}},{\"band_id\":\"&X\",\"bandwidth\":{\"units\":\"MHz\",\"value\":32.0},\"bbc_id\":\"&BBC49\",\"channel_id\":\"&Ch49\",\"channel_name\":\"X31LX\",\"net_sideband\":\"L\",\"phase_cal_id\":\"&L_cal\",\"sky_frequency\":{\"units\":\"MHz\",\"value\":10680.4}},{\"band_id\":\"&X\",\"bandwidth\":{\"units\":\"MHz\",\"value\":32.0},\"bbc_id\":\"&BBC50\",\"channel_id\":\"&Ch50\",\"channel_name\":\"X30LX\",\"net_sideband\":\"L\",\"phase_cal_id\":\"&L_cal\",\"sky_frequency\":{\"units\":\"MHz\",\"value\":10648.4}},{\"band_id\":\"&X\",\"bandwidth\":{\"units\":\"MHz\",\"value\":32.0},\"bbc_id\":\"&BBC51\",\"channel_id\":\"&Ch51\",\"channel_name\":\"X29LX\",\"net_sideband\":\"L\",\"phase_cal_id\":\"&L_cal\",\"sky_frequency\":{\"units\":\"MHz\",\"value\":10584.4}},{\"band_id\":\"&X\",\"bandwidth\":{\"units\":\"MHz\",\"value\":32.0},\"bbc_id\":\"&BBC52\",\"channel_id\":\"&Ch52\",\"channel_name\":\"X28LX\",\"net_sideband\":\"L\",\"phase_cal_id\":\"&L_cal\",\"sky_frequency\":{\"units\":\"MHz\",\"value\":10520.4}},{\"band_id\":\"&X\",\"bandwidth\":{\"units\":\"MHz\",\"value\":32.0},\"bbc_id\":\"&BBC53\",\"channel_id\":\"&Ch53\",\"channel_name\":\"X27LX\",\"net_sideband\":\"L\",\"phase_cal_id\":\"&L_cal\",\"sky_frequency\":{\"units\":\"MHz\",\"value\":10424.4}},{\"band_id\":\"&X\",\"bandwidth\":{\"units\":\"MHz\",\"value\":32.0},\"bbc_id\":\"&BBC54\",\"channel_id\":\"&Ch54\",\"channel_name\":\"X26LX\",\"net_sideband\":\"L\",\"phase_cal_id\":\"&L_cal\",\"sky_frequency\":{\"units\":\"MHz\",\"value\":10296.4}},{\"band_id\":\"&X\",\"bandwidth\":{\"units\":\"MHz\",\"value\":32.0},\"bbc_id\":\"&BBC55\",\"channel_id\":\"&Ch55\",\"channel_name\":\"X25LX\",\"net_sideband\":\"L\",\"phase_cal_id\":\"&L_cal\",\"sky_frequency\":{\"units\":\"MHz\",\"value\":10264.4}},{\"band_id\":\"&X\",\"bandwidth\":{\"units\":\"MHz\",\"value\":32.0},\"bbc_id\":\"&BBC56\",\"channel_id\":\"&Ch56\",\"channel_name\":\"X24LX\",\"net_sideband\":\"L\",\"phase_cal_id\":\"&L_cal\",\"sky_frequency\":{\"units\":\"MHz\",\"value\":10232.4}},{\"band_id\":\"&X\",\"bandwidth\":{\"units\":\"MHz\",\"value\":32.0},\"bbc_id\":\"&BBC57\",\"channel_id\":\"&Ch57\",\"channel_name\":\"X31LY\",\"net_sideband\":\"L\",\"phase_cal_id\":\"&L_cal\",\"sky_frequency\":{\"units\":\"MHz\",\"value\":10680.4}},{\"band_id\":\"&X\",\"bandwidth\":{\"units\":\"MHz\",\"value\":32.0},\"bbc_id\":\"&BBC58\",\"channel_id\":\"&Ch58\",\"channel_name\":\"X30LY\",\"net_sideband\":\"L\",\"phase_cal_id\":\"&L_cal\",\"sky_frequency\":{\"units\":\"MHz\",\"value\":10648.4}},{\"band_id\":\"&X\",\"bandwidth\":{\"units\":\"MHz\",\"value\":32.0},\"bbc_id\":\"&BBC59\",\"channel_id\":\"&Ch59\",\"channel_name\":\"X29LY\",\"net_sideband\":\"L\",\"phase_cal_id\":\"&L_cal\",\"sky_frequency\":{\"units\":\"MHz\",\"value\":10584.4}},{\"band_id\":\"&X\",\"bandwidth\":{\"units\":\"MHz\",\"value\":32.0},\"bbc_id\":\"&BBC60\",\"channel_id\":\"&Ch60\",\"channel_name\":\"X28LY\",\"net_sideband\":\"L\",\"phase_cal_id\":\"&L_cal\",\"sky_frequency\":{\"units\":\"MHz\",\"value\":10520.4}},{\"band_id\":\"&X\",\"bandwidth\":{\"units\":\"MHz\",\"value\":32.0},\"bbc_id\":\"&BBC61\",\"channel_id\":\"&Ch61\",\"channel_name\":\"X27LY\",\"net_sideband\":\"L\",\"phase_cal_id\":\"&L_cal\",\"sky_frequency\":{\"units\":\"MHz\",\"value\":10424.4}},{\"band_id\":\"&X\",\"bandwidth\":{\"units\":\"MHz\",\"value\":32.0},\"bbc_id\":\"&BBC62\",\"channel_id\":\"&Ch62\",\"channel_name\":\"X26LY\",\"net_sideband\":\"L\",\"phase_cal_id\":\"&L_cal\",\"sky_frequency\":{\"units\":\"MHz\",\"value\":10296.4}},{\"band_id\":\"&X\",\"bandwidth\":{\"units\":\"MHz\",\"value\":32.0},\"bbc_id\":\"&BBC63\",\"channel_id\":\"&Ch63\",\"channel_name\":\"X25LY\",\"net_sideband\":\"L\",\"phase_cal_id\":\"&L_cal\",\"sky_frequency\":{\"units\":\"MHz\",\"value\":10264.4}},{\"band_id\":\"&X\",\"bandwidth\":{\"units\":\"MHz\",\"value\":32.0},\"bbc_id\":\"&BBC64\",\"channel_id\":\"&Ch64\",\"channel_name\":\"X24LY\",\"net_sideband\":\"L\",\"phase_cal_id\":\"&L_cal\",\"sky_frequency\":{\"units\":\"MHz\",\"value\":10232.4}}],\"sample_rate\":{\"units\":\"Ms/sec\",\"value\":64.0}}},\"$GLOBAL\":{\"$EOP\":[{\"keyword\":\"EOP003\"}],\"$EXPER\":[{\"keyword\":\"VT9105\"}],\"$SCHEDULING_PARAMS\":[{\"keyword\":\"SKED_PARAMS\"}]},\"$IF\":{\"VGOS_std\":{\"if_def\":[{\"effective_LO\":{\"units\":\"MHz\",\"value\":8080.0},\"if_id\":\"&IF_1N\",\"net_sideband\":\"U\",\"phase_cal_base\":{\"units\":\"Hz\",\"value\":0.0},\"phase_cal_interval\":{\"units\":\"MHz\",\"value\":5.0},\"physical_if_id\":\"3N\",\"polarization\":\"X\"},{\"effective_LO\":{\"units\":\"MHz\",\"value\":8080.0},\"if_id\":\"&IF_3N\",\"net_sideband\":\"U\",\"phase_cal_base\":{\"units\":\"Hz\",\"value\":0.0},\"phase_cal_interval\":{\"units\":\"MHz\",\"value\":5.0},\"physical_if_id\":\"3N\",\"polarization\":\"Y\"}]}},\"$LOG\":{\"log_std\":null},\"$MODE\":{\"VGOS\":{\"$BBC\":[{\"keyword\":\"VGOS_std\",\"qualifiers\":[\"Aa\",\"Bb\"]}],\"$FREQ\":[{\"keyword\":\"VGOS_std\",\"qualifiers\":[\"Aa\",\"Bb\"]}],\"$IF\":[{\"keyword\":\"VGOS_std\",\"qualifiers\":[\"Aa\",\"Bb\"]}],\"$TRACKS\":[{\"keyword\":\"trax_2bits\",\"qualifiers\":[\"Aa\",\"Bb\"]}]}},\"$PBS_INIT\":{\"PBS_DUMMY\":null},\"$SCHED\":{\"105-1800\":{\"fourfit_reftime\":\"2025y001d00h00m15s\",\"mode\":\"VGOS\",\"source\":[{\"source\":\"0016+731\"}],\"start\":\"2025y001d00h00m00s\",\"station\":[{\"data_good\":{\"units\":\"sec\",\"value\":0.0},\"data_stop\":{\"units\":\"sec\",\"value\":30.0},\"media_position\":{\"units\":\"ft\",\"value\":0.0},\"pass\":\"1A\",\"record_flag\":1,\"station\":\"Aa\",\"wrap_id\":\"&ccw\"},{\"data_good\":{\"units\":\"sec\",\"value\":0.0},\"data_stop\":{\"units\":\"sec\",\"value\":30.0},\"media_position\":{\"units\":\"ft\",\"value\":0.0},\"pass\":\"1A\",\"record_flag\":1,\"station\":\"Bb\",\"wrap_id\":\"&n\"}]}},\"$SITE\":{\"DUMMY_A\":{\"horizon_map_az\":{\"units\":\"deg\",\"values\":[0.0,15.0,58.0,90.0,130.0,154.0,162.0,167.0,172.0,212.0,222.0,227.0,232.0,290.0,360.0]},\"horizon_map_el\":{\"units\":\"deg\",\"values\":[6.0,5.5,3.5,4.5,3.0,28.0,33.0,36.0,40.0,36.0,33.0,28.0,6.0,6.0,6.0]},\"mk4_site_ID\":\"A\",\"occupation_code\":\"71025302\",\"site_ID\":\"Aa\",\"site_name\":\"DUMMY_A\",\"site_position\":{\"x\":{\"units\":\"m\",\"value\":1130730.245},\"y\":{\"units\":\"m\",\"value\":-4831245.953},\"z\":{\"units\":\"m\",\"value\":3994228.228}},\"site_type\":\"fixed\"},\"DUMMY_B\":{\"horizon_map_az\":{\"units\":\"deg\",\"values\":[0.0,238.0,244.0,260.0,280.0,295.0,300.0,305.0,310.0,325.0,330.0,335.0,340.0,350.0,360.0]},\"horizon_map_el\":{\"units\":\"deg\",\"values\":[10.0,14.0,10.0,5.0,10.0,25.0,35.0,40.0,45.0,40.0,35.0,30.0,10.0,5.0,10.0]},\"mk4_site_ID\":\"B\",\"occupation_code\":\"72983001\",\"site_ID\":\"Bb\",\"site_name\":\"DUMMY_B\",\"site_position\":{\"x\":{\"units\":\"m\",\"value\":-5543831.705},\"y\":{\"units\":\"m\",\"value\":-2054585.983},\"z\":{\"units\":\"m\",\"value\":2387828.776}},\"site_type\":\"fixed\"}},\"$SOURCE\":{\"0016+731\":{\"IAU_name\":\"0016+731\",\"dec\":\"73d27'30.0174\\\"\",\"ra\":\"00h19m45.78642s\",\"ref_coord_frame\":\"J2000\",\"source_name\":\"0016+731\",\"source_type\":{\"experiment_type\":\"dummy\",\"physical_type\":\"dummy\"}}},\"$STATION\":{\"Aa\":{\"$ANTENNA\":[{\"keyword\":\"DUMMY_A\"}],\"$CLOCK\":[{\"keyword\":\"Aa\"}],\"$SITE\":[{\"keyword\":\"DUMMY_A\"}]},\"Bb\":{\"$ANTENNA\":[{\"keyword\":\"DUMMY_B\"}],\"$CLOCK\":[{\"keyword\":\"Bb\"}],\"$SITE\":[{\"keyword\":\"DUMMY_B\"}]}},\"$TRACKS\":{\"VDIF_format\":{\"track_frame_format\":\"VDIF\"},\"trax_2bits\":{\"bits/sample\":2}},\"VEX_rev\":\"ovex\",\"difx_input_filename\":\"./dummy.input\"}";

return dummy_vex;
};

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
                    // source_noise_re.GetSample(dummy_time, samp_re);
                    // source_noise_im.GetSample(dummy_time, samp_im);
                    // std::complex<double> source_signal(samp_re, samp_im);
                    // source_signal *= 1.0/std::abs(source_signal);//normalize
                    std::complex<double> source_signal(1.0, 0.0);

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
                    vis(pp, ch, ap, sc) = correlated_vis + 50.0*noise;
                }
            }
        }
    }

}


void GenerateStationData(station_coord_type& st_coord, double duration, std::string model_start)
{
    std::size_t n_order = 6;
    std::size_t n_coord = 7; //note we do not manufacture a phase-spline (e.g. type_302)
    std::size_t n_poly = 1; //nspline intervals in d2m4

    st_coord.Resize(n_coord, n_poly, n_order + 1); //p = n_order is included!

    //label the coordinate axis
    std::get< COORD_AXIS >(st_coord)[0] = "delay";
    std::get< COORD_AXIS >(st_coord)[1] = "azimuth";
    std::get< COORD_AXIS >(st_coord)[2] = "elevation";
    std::get< COORD_AXIS >(st_coord)[3] = "parallactic_angle";
    std::get< COORD_AXIS >(st_coord)[4] = "u";
    std::get< COORD_AXIS >(st_coord)[5] = "v";
    std::get< COORD_AXIS >(st_coord)[6] = "w";

    //label the spline interval axis
    for(std::size_t i = 0; i < n_poly; i++)
    {
        //duration should be scan duration for this
        std::get< INTERVAL_AXIS >(st_coord)[i] = i * duration;
    }

    //label polynomial order axis
    for(std::size_t i = 0; i <= n_order; i++)
    {
        std::get< COEFF_AXIS >(st_coord)[i] = i;
    }

    for(std::size_t i = 0; i < n_poly; i++)
    {
        for(std::size_t p = 0; p <= n_order; p++)
        {
            st_coord.at(0, i, p) = 0.0; //delay;
            st_coord.at(1, i, p) = 0.0; //az;
            st_coord.at(2, i, p) = 0.0; // el;
            st_coord.at(3, i, p) = 0.0; //par;
            st_coord.at(4, i, p) = 0.0; //u;
            st_coord.at(5, i, p) = 0.0; //v;
            st_coord.at(6, i, p) = 0.0; //w;
        }
    }

    st_coord.Insert(std::string("model_interval"), duration);
    st_coord.Insert(std::string("model_start"), model_start);
    st_coord.Insert(std::string("mount"), "dummy");
    st_coord.Insert(std::string("X"), 0.0);
    st_coord.Insert(std::string("Y"), 0.0);
    st_coord.Insert(std::string("Z"), 0.0);

    //store n_poly as int
    int nsplines = n_poly;
    st_coord.Insert(std::string("nsplines"), nsplines);
}



int main(int argc, char** argv)
{
    CLI::App app{"dummy_vis"};

    double fringe_amplitude = 1.0;
    std::string output_file = "AB.Aa-Bb.abcdef.cor";
    double residual_delay = 1e-5;
    double residual_delay_rate = 0.0;
    double residual_phase = 0.0;
    int num_channels = 16;
    double channel_width = 32.0;
    std::vector<double> channel_frequencies;
    double ref_freq_mhz = 10000.0;
    double start_freq = 8000.0;
    int num_subchannels = 32;
    int num_aps = 30;
    double ap_length = 1.0;
    double snr = 100.0;
    double system_noise_rms = -1.0;
    int random_seed = -1;
    std::string polprod = "XX";
    int message_level = 0;
    std::vector< std::string > message_categories;
    std::string source_name = "SIMULATED";
    std::string baseline_name = "Aa-Bb";
    std::string baseline_shortname = "AB";
    std::string ref_station = "A";
    std::string rem_station = "B";
    std::string ref_station_name = "DUMMY_A";
    std::string rem_station_name = "DUMMY_B";
    std::string root_code = "abcdef";
    std::string start_time = "2025y001d00h00m00s";


    // Remove help flag because it shortcuts all processing
    app.set_help_flag();
    auto* help = app.add_flag("-h,--help", "print this help message and exit");

    app.add_option("-o,--output", output_file, "name of output file for simulated visibilities");
    app.add_option("-A,--amplitude", fringe_amplitude, "fringe amplitude (correlated flux density)")->default_val(1.0);
    app.add_option("-d,--delay", residual_delay, "residual delay in microseconds")->default_val(1e-5);
    app.add_option("-r,--delay-rate", residual_delay_rate, "residual delay-rate in microseconds/second")->default_val(0.0);
    app.add_option("-p,--phase", residual_phase, "residual phase offset in degrees")->default_val(0.0);
    app.add_option("-c,--channels", num_channels, "total number of frequency channels")->default_val(32);
    app.add_option("-w,--channel-width", channel_width, "channel width in MHz")->default_val(32.0);
    app.add_option("-R,--reference-frequency", ref_freq_mhz, "reference frequency in MHz")->default_val(10000.0);
    app.add_option("-f,--frequencies", channel_frequencies,
        "channel center frequencies in MHz (comma-separated). If not specified, will be auto-generated")->delimiter(',');
    app.add_option("-F,--start-freq", start_freq, "starting frequency in MHz (if frequencies not explicitly given)")
        ->default_val(8000.0);
    app.add_option("-s,--subchannels", num_subchannels, "number of sub-channels (lags) per channel")->default_val(32);
    app.add_option("-n,--num-aps", num_aps, "total number of accumulation periods")->default_val(30);
    app.add_option("-t,--ap-length", ap_length, "accumulation period length in seconds")->default_val(1.0);
    // app.add_option("-T,--start-time", start_time, "start time in seconds")->default_val(0.0);
    app.add_option("--snr", snr, "signal-to-noise ratio")->default_val(100.0);
    app.add_option("--noise-rms", system_noise_rms, "system noise RMS (if specified, overrides SNR parameter)");
    app.add_option("--seed", random_seed, "random number generator seed (-1 for time-based)")->default_val(-1);
    app.add_option("-P,--polprod", polprod, "polarization product (e.g., XX, YY, XY, RR, LL)")->default_val("XX");
    app.add_option("--source", source_name, "source name")->default_val("SIMULATED");
    //app.add_option("--baseline", baseline_shortname, "baseline baseline_shortname (e.g. GE)")->default_val("AB");
    app.add_option("--root-code", root_code, "root code string")->default_val("abcdef");

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

    // Clamp message level
    if(message_level > 5) { message_level = 5; }
    if(message_level < -2) { message_level = -2; }
    MHO_Message::GetInstance().AcceptAllKeys();
    MHO_Message::GetInstance().SetLegacyMessageLevel(message_level);

    if(message_categories.size() != 0)
    {
        for(std::size_t m = 0; m < message_categories.size(); m++)
        {
            MHO_Message::GetInstance().AddKey(message_categories[m]);
        }
        MHO_Message::GetInstance().LimitToKeySet();
    }
    MHO_Message::GetInstance().SetLegacyMessageLevel(message_level);

    //start, stop, duration
    int64_t duration_ns = ap_length * num_aps * SEC_TO_NANOSEC;
    double duration = duration_ns/SEC_TO_NANOSEC;
    auto start_tp = hops_clock::from_vex_format(start_time);
    auto stop_tp = start_tp + hops_clock::duration(duration_ns);
    std::string stop_time = hops_clock::to_vex_format(stop_tp);

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

    // Handle random seed
    if(random_seed < 0)
    {
        random_seed = static_cast<int>(std::time(nullptr));
        std::cout << "Using time-based random seed: " << random_seed << std::endl;
    }

    double residual_phase_rad = residual_phase * M_PI / 180.0;

    // Derive station names from baseline
    if(baseline_shortname.size() == 2)
    {
        ref_station = std::string(1, baseline_shortname[0]);
        rem_station = std::string(1, baseline_shortname[1]);
    }

    std::cout << "\n=== Simulation Parameters ===" << std::endl;
    std::cout << "Fringe amplitude: " << fringe_amplitude << std::endl;
    std::cout << "Residual delay: " << residual_delay << " us" << std::endl;
    std::cout << "Residual delay-rate: " << residual_delay_rate << " us/s" << std::endl;
    std::cout << "Residual phase: " << residual_phase << " deg" << std::endl;
    std::cout << "Reference frequency: " << ref_freq_mhz << " MHz" << std::endl;
    std::cout << "Channels: " << num_channels << std::endl;
    std::cout << "Sub-channels: " << num_subchannels << std::endl;
    std::cout << "Channel width: " << channel_width << " MHz" << std::endl;
    std::cout << "APs: " << num_aps << " x " << ap_length << " s" << std::endl;
    std::cout << "SNR: " << snr << std::endl;
    std::cout << "System noise RMS: " << system_noise_rms << std::endl;
    std::cout << "Polarization: " << polprod << std::endl;
    std::cout << "Baseline: " << baseline_name << std::endl;
    std::cout << "Source: " << source_name << std::endl;
    std::cout << "Random seed: " << random_seed << std::endl;
    std::cout << "Output file: " << output_file << std::endl;
    std::cout << "============================\n" << std::endl;

    // =========================================================
    // Generate visibility data
    // =========================================================
    visibility_type vis;

    GenerateSimulatedVisibilities(
        vis,
        fringe_amplitude,
        residual_delay,
        residual_delay_rate,
        residual_phase_rad,
        channel_frequencies,
        channel_width,
        num_subchannels,
        num_aps,
        ap_length,
        system_noise_rms,
        random_seed,
        ref_freq_mhz
    );

    // =========================================================
    // Construct weight container
    // =========================================================
    // Weights have the same shape as visibilities except the
    // sub-channel (FREQ) axis dimension is 1.
    // All weight entries are 1.0 (scalar, not complex).

    weight_type wt;
    std::size_t wdims[VIS_NDIM];
    wdims[POLPROD_AXIS] = 1;           // single pol product
    wdims[CHANNEL_AXIS] = num_channels;
    wdims[TIME_AXIS]    = num_aps;
    wdims[FREQ_AXIS]    = 1;           // single weight per AP per channel
    wt.Resize(wdims);
    wt.ZeroArray();

    // Fill all weights to 1.0
    for(int pp = 0; pp < 1; ++pp)
    {
        for(int ch = 0; ch < num_channels; ++ch)
        {
            for(int ap = 0; ap < num_aps; ++ap)
            {
                wt(pp, ch, ap, 0) = 1.0;
            }
        }
    }

    // =========================================================
    // Fill weight axis values to match visibility axes
    // =========================================================
    auto* wpolprod_axis = &(std::get< POLPROD_AXIS >(wt));
    wpolprod_axis->at(0) = polprod;

    auto* wch_axis = &(std::get< CHANNEL_AXIS >(wt));
    for(int ch = 0; ch < num_channels; ++ch)
    {
        wch_axis->at(ch) = channel_frequencies[ch];
    }

    auto* wap_axis = &(std::get< TIME_AXIS >(wt));
    for(int ap = 0; ap < num_aps; ++ap)
    {
        wap_axis->at(ap) = ap * ap_length;
    }

    auto* wsp_axis = &(std::get< FREQ_AXIS >(wt));
    wsp_axis->at(0) = 0.0;

    // =========================================================
    // Tag the visibility container
    // =========================================================


    std::cout<<"baseline_name"<<baseline_name<<std::endl;
    std::cout<<"ref station"<<ref_station<<std::endl;
    std::cout<<"rem station"<<rem_station<<std::endl;

    vis.Insert(std::string("name"), std::string("visibilities"));
    vis.Insert(std::string("baseline"), baseline_name);
    vis.Insert(std::string("baseline_shortname"), baseline_shortname);
    vis.Insert(std::string("reference_station"), ref_station);
    vis.Insert(std::string("remote_station"), rem_station);
    vis.Insert(std::string("reference_station_name"), ref_station);
    vis.Insert(std::string("remote_station_name"), rem_station);
    vis.Insert(std::string("reference_station_mk4id"), ref_station);
    vis.Insert(std::string("remote_station_mk4id"), rem_station);
    vis.Insert(std::string("correlation_date"), start_time);
    vis.Insert(std::string("root_code"), root_code);
    vis.Insert(std::string("origin"), std::string("simulated"));
    vis.Insert(std::string("start"), start_time);
    vis.Insert(std::string("stop"), stop_time);
    vis.Insert(std::string("source"), source_name);

    // Tag visibility axes
    auto* vpolprod_axis = &(std::get< POLPROD_AXIS >(vis));
    vpolprod_axis->Insert(std::string("name"), std::string("polarization_product"));

    auto* vch_axis = &(std::get< CHANNEL_AXIS >(vis));
    vch_axis->Insert(std::string("name"), std::string("channel"));
    vch_axis->Insert(std::string("units"), std::string("MHz"));

    // Add per-channel label objects
    for(int ch = 0; ch < num_channels; ++ch)
    {
        mho_json ch_label;
        ch_label["sky_freq"] = channel_frequencies[ch];
        ch_label["bandwidth"] = channel_width;
        ch_label["net_sideband"] = std::string("U");    // upper sideband
        ch_label["difx_freqindex"] = ch;
        ch_label["channel"] = ch;
        ch_label["frequency_band"] = std::string("X");  // dummy band label
        ch_label["mk4_channel_id"] = std::string("sim") + std::to_string(ch);
        vch_axis->SetLabelObject(ch_label, ch);

        // Same labels on the weight channel axis
        wch_axis->SetLabelObject(ch_label, ch);
    }

    auto* vap_axis = &(std::get< TIME_AXIS >(vis));
    vap_axis->Insert(std::string("name"), std::string("time"));
    vap_axis->Insert(std::string("units"), std::string("s"));

    auto* vsp_axis = &(std::get< FREQ_AXIS >(vis));
    vsp_axis->Insert(std::string("name"), std::string("frequency"));
    vsp_axis->Insert(std::string("units"), std::string("MHz"));

    // =========================================================
    // Tag the weight container
    // =========================================================
    wt.Insert(std::string("name"), std::string("weights"));
    wt.Insert(std::string("baseline"), baseline_name);
    wt.Insert(std::string("baseline_shortname"), baseline_shortname);
    wt.Insert(std::string("reference_station"), ref_station);
    wt.Insert(std::string("remote_station"), rem_station);
    wt.Insert(std::string("reference_station_name"), ref_station);
    wt.Insert(std::string("remote_station_name"), rem_station);
    wt.Insert(std::string("reference_station_mk4id"), ref_station);
    wt.Insert(std::string("remote_station_mk4id"), rem_station);
    wt.Insert(std::string("correlation_date"), std::string("2025-01-01T00:00:00"));
    wt.Insert(std::string("root_code"), root_code);
    wt.Insert(std::string("origin"), std::string("simulated"));
    wt.Insert(std::string("start"), start_time);
    wt.Insert(std::string("stop"), stop_time);
    wt.Insert(std::string("source"), source_name);

    // Tag weight axes
    wpolprod_axis->Insert(std::string("name"), std::string("polarization_product"));
    wch_axis->Insert(std::string("name"), std::string("channel"));
    wch_axis->Insert(std::string("units"), std::string("MHz"));
    wap_axis->Insert(std::string("name"), std::string("time"));
    wap_axis->Insert(std::string("units"), std::string("s"));
    wsp_axis->Insert(std::string("name"), std::string("frequency"));
    wsp_axis->Insert(std::string("units"), std::string("MHz"));

    // =========================================================
    // Construct the tags object
    // =========================================================
    MHO_ObjectTags tags;
    tags.Insert(std::string("root_code"), root_code);
    tags.Insert(std::string("baseline"), baseline_name);
    tags.Insert(std::string("baseline_shortname"), baseline_shortname);
    tags.Insert(std::string("reference_station"), ref_station);
    tags.Insert(std::string("remote_station"), rem_station);
    tags.Insert(std::string("reference_station_name"), ref_station_name);
    tags.Insert(std::string("remote_station_name"), rem_station);
    tags.Insert(std::string("reference_station_mk4id"), ref_station_name);
    tags.Insert(std::string("remote_station_mk4id"), rem_station);
    tags.Insert(std::string("correlation_date"), std::string("2025-01-01T00:00:00"));
    tags.Insert(std::string("origin"), std::string("simulated"));
    tags.Insert(std::string("name"), std::string("object_tags"));
    tags.Insert(std::string("start"), start_time);
    tags.Insert(std::string("stop"), stop_time);
    tags.Insert(std::string("source"), source_name);

    // Attach UUIDs of the vis and weight objects to the tags
    tags.AddObjectUUID(vis.GetObjectUUID());
    tags.AddObjectUUID(wt.GetObjectUUID());

    // Attach frequency band set and polarization product set
    std::vector<std::string> fband_vec = {"X"};  // dummy band label
    std::vector<std::string> pprod_vec = {polprod};
    tags.Insert("frequency_band_set", fband_vec);
    tags.Insert("polarization_product_set", pprod_vec);

    // =========================================================
    // Write to file
    // =========================================================
    MHO_BinaryFileInterface inter;
    bool status = inter.OpenToWrite(output_file);
    if(status)
    {
        inter.Write(tags, "tags");
        inter.Write(vis, "vis");
        inter.Write(wt, "weight");
        inter.Close();
        std::cout << "Wrote simulated data to: " << output_file << std::endl;
    }
    else
    {
        std::cerr << "Error: could not open output file: " << output_file << std::endl;
        return 1;
    }

    //reference and remote station data
    for(int stid = 0; stid < 2; stid++)
    {
        std::string station_file;


        station_coord_type sta;
        GenerateStationData(sta, duration, start_time);

        if(stid == 0)
        {
            station_file = "A.Aa.abcdef.sta";
            //add some tags to the station coord data
            sta.Insert(std::string("station_code"), "Aa");
            sta.Insert(std::string("station_mk4id"), "A");
            sta.Insert(std::string("station_name"), "DUMMY_A");
            sta.Insert(std::string("root_code"), root_code);
            sta.Insert(std::string("name"), std::string("station_data"));
        }

        if(stid == 1)
        {
            station_file = "B.Ba.abcdef.sta";
            //add some tags to the station coord data
            sta.Insert(std::string("station_code"), "Bb");
            sta.Insert(std::string("station_mk4id"), "B");
            sta.Insert(std::string("station_name"), "DUMMY_B");
            sta.Insert(std::string("root_code"), root_code);
            sta.Insert(std::string("name"), std::string("station_data"));
        }

        MHO_BinaryFileInterface inter2;
        bool status2 = inter2.OpenToWrite(station_file);
        if(status2)
        {
            inter2.Write(sta, "sta");
            inter2.Write(tags, "tags");
            inter2.Close();
            std::cout << "Wrote station data to: " << station_file << std::endl;
        }
        else
        {
            std::cerr << "Error: could not open output file: " << station_file << std::endl;
            return 1;
        }
    }








    return 0;
}
