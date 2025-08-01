/******************************************************************************
*    init_tokens initializes the actual string values of each of fourfit's    *
*                recognizable tokens. It also initializes the category for    *
*                each token.                                                  *
*                                                 rjc  92.10.1                *
******************************************************************************/
#include "msg.h"
#include "parser.h"
#include "control.h"
#include "ffcontrol.h"
#include <stdio.h>

#define tokenize(aa,bb,cc) {token_string[aa] = bb; token_cat[aa] = cc;}

// Warning: ISO C++11 does not allow conversion from string literal to
// 'char *' [-Wwritable-strings]; this can be fixed by allocating memory
// for the strings and freeing once parsing is completely done:
//#include <string.h>
//#include <stdlib.h>
//#define tokenize(aa,bb,cc) do{\
// token_string[aa] = (char*) malloc(sizeof(char)* strlen(bb) );\
// strcpy(token_string[aa], bb); token_cat[aa] = cc;} while(0)
//
// parse_control_file.c:char *token_string[MAX_TOKENS];
void free_tokens(int where)
    {
    // nothing for now, but introduce the stub.
    msg("free_tokens @ %d",-2,where);
    }

// Note that the caller, parse_control_file() is called from 5 places:
//  where=3: set_defaults.c (hops & chops)
//      for the default default_cfile (called from top-level fourfit)
//  where=0: parse_control_file..c (hops & chops)
//      on control_filename (from parse_cmdline on -c arg)
//  where=1: parse_control_file.c (hops & chops)
//      on control_string (from parse_cmdline after a set option)
//  where=2: construct_cblock.c (chops)
//      after nullify_cblock()
//  where=4: for_python_construct_cblock.c (HOPS4)
//      after nullify_cblock()
// and a msgs are emitted at init_tokens() and free_tokens().
void init_tokens (int where)
    {
    int i;
    extern char *token_string[];          /* these token arrays could be put */
    extern int  token_cat[];              /* in a structure & initialized    */

    for (i=0; i<MAX_TOKENS; i++)
    tokenize (i, "", 0);                  /* initialize all tokens to null */
    msg("init_tokens @ %d",-2,where);

    /*       token #        string        category      */
    tokenize (STATION_,     "station",    STATION)
    tokenize (BASELINE_,    "baseline",   BASELINE)
    tokenize (SOURCE_,      "source",     SOURCE)
    tokenize (SCAN_,        "scan",       SCAN)
    tokenize (F_GROUP_,     "f_group",    F_GROUP)
    tokenize (IF_,          "if",         IF)
    tokenize (ELSE_,        "else",       0)
    tokenize (AND_,         "and",        AND)
    tokenize (OR_,          "or",         OR)
    tokenize (NOT_,         "not",        NOT)
    tokenize (LPAREN_,      "(",          0)
    tokenize (RPAREN_,      ")",          0)
    tokenize (LESS_THAN_,   "<",          LESS_THAN)
    tokenize (GREATER_THAN_,">",          GREATER_THAN)
    tokenize (MIN_WEIGHT_,  "min_weight", FLOAT_PARAM)
    tokenize (X_CRC_,       "x_crc",      INT_PARAM)
    tokenize (Y_CRC_,       "y_crc",      INT_PARAM)
    tokenize (X_SLIP_SYNC_, "x_slip_sync",INT_PARAM)
    tokenize (Y_SLIP_SYNC_, "y_slip_sync",INT_PARAM)
    tokenize (FREQS_,       "freqs",      VECTOR_CHAR_PARAM)
    tokenize (FRQS_,        "frqs",       SAVE_CHAR_PARAM)
    tokenize (INDEX_,       "index",      VECTOR_INT_PARAM) // deprecated
    tokenize (PC_PHASE_,    "pc_phases",  CHAN_PARAM)
    tokenize (PC_MODE_,     "pc_mode",    INT_PARAM)
    tokenize (PC_PERIOD_,   "pc_period",  INT_PARAM)
    tokenize (PC_TONEMASK_, "pc_tonemask",CHAN_PARAM)
    tokenize (LSB_OFFSET_,  "lsb_offset", FLOAT_PARAM)
    tokenize (SB_WIN_,      "sb_win",     TWO_FLOAT_PARAM)
    tokenize (MB_WIN_,      "mb_win",     TWO_FLOAT_PARAM) 
    tokenize (DR_WIN_,      "dr_win",     TWO_FLOAT_PARAM)
    tokenize (SKIP_,        "skip",       INT_PARAM)
    tokenize (WILDCARD_,    "?",          ONE_CHAR)
    tokenize (INT_,         "~INTEGER~",  INTEGER)
    tokenize (FLOAT_,       "~FLOAT~",    FLOAT)
    tokenize (TIME_VAL_,    "~TIME_VAL~", TIME_VAL)
    tokenize (ONE_CHAR_,    "~1 CHAR~",   ONE_CHAR)
    tokenize (TWO_CHAR_,    "~2 CHARS~",  TWO_CHAR)
    tokenize (MANY_CHAR_,   "~STRING~",   MANY_CHAR)
    tokenize (START_,       "start",      INT_PARAM)
    tokenize (STOP_,        "stop",       INT_PARAM)
    tokenize (KEEP_,        "keep",       INT_CONST + KEEP)
    tokenize (DISCARD_,     "discard",    INT_CONST + DISCARD)
    tokenize (NORMAL_,      "normal",     INT_CONST + NORMAL)
    tokenize (AP_BY_AP_,    "ap_by_ap",   INT_CONST + AP_BY_AP)
    tokenize (MANUAL_,      "manual",     INT_CONST + MANUAL)
    tokenize (MULTITONE_,   "multitone",  INT_CONST + MULTITONE)
    tokenize (SCAN_START_,  "scan_start", INT_CONST + SCAN_START)
    tokenize (EACH_MINUTE_, "each_minute",INT_CONST + EACH_MINUTE)
    tokenize (TRUE_,        "true",       INT_CONST + 1)
    tokenize (FALSE_,       "false",      INT_CONST + 0)
    tokenize (REF_FREQ_,    "ref_freq",   FLOAT_PARAM)
    tokenize (SWITCHED_,    "switched",   INT_PARAM)
    tokenize (PERIOD_,      "period",     INT_PARAM)
    tokenize (GATES_,       "gates",      CHAN_PARAM)
    tokenize (RA_OFFSET_,   "ra_offset",  FLOAT_PARAM)
    tokenize (DEC_OFFSET_,  "dec_offset", FLOAT_PARAM)
    tokenize (TO_,          "to",         TO)
    tokenize (ADHOC_PHASE_, "adhoc_phase",INT_PARAM)
    tokenize (SINEWAVE_,    "sinewave",   INT_CONST + SINEWAVE)
    tokenize (POLYNOMIAL_,  "polynomial", INT_CONST + POLYNOMIAL)
    tokenize (ADHOC_PERIOD_,"adhoc_period",FLOAT_PARAM)
    tokenize (ADHOC_AMP_,   "adhoc_amp",  FLOAT_PARAM)
    tokenize (ADHOC_POLY_,  "adhoc_poly", VECTOR_FLOAT_PARAM)
    tokenize (ADHOC_TREF_,  "adhoc_tref", FLOAT_PARAM)
    tokenize (PC_FREQ_,     "pc_freqs"  , CHAN_PARAM)
    tokenize (USE_SAMPLES_, "use_samples",INT_PARAM)
    tokenize (PASSBAND_,    "passband",   TWO_FLOAT_PARAM)
    tokenize (AVXPZOOM_,    "avxpzoom",   TWO_FLOAT_PARAM)
    tokenize (AVXPLOPT_,    "avxplopt",   TWO_FLOAT_PARAM)
    tokenize (GEN_CF_RECORD_,"gen_cf_record", INT_PARAM)
    tokenize (NOTCHES_,     "notches",    VECTOR_FLOAT_PARAM)
    tokenize (CHAN_NOTCHES_,    "chan_notches",STRING_PARAM)
    tokenize (T_COHERE_,    "t_cohere",   FLOAT_PARAM)
    tokenize (IONOSPHERE_,  "ionosphere", FLOAT_PARAM)
    tokenize (DELAY_OFFS_,  "delay_offs", CHAN_PARAM)
    tokenize (DELAY_OFFS_L_,"delay_offs_l",CHAN_PARAM)
    tokenize (DELAY_OFFS_R_,"delay_offs_r",CHAN_PARAM)
    tokenize (DELAY_OFFS_X_,"delay_offs_x",CHAN_PARAM)
    tokenize (DELAY_OFFS_Y_,"delay_offs_y",CHAN_PARAM)
    tokenize (DC_BLOCK_,    "dc_block",   INT_PARAM)
    tokenize (SAMPLERS_,    "samplers",   VECTOR_STRING_PARAM)
    tokenize (OPTIMIZE_CLOSURE_, "optimize_closure", INT_PARAM)
    tokenize (PC_PHASE_L_,  "pc_phases_l",CHAN_PARAM)
    tokenize (PC_PHASE_R_,  "pc_phases_r",CHAN_PARAM)
    tokenize (PC_PHASE_X_,  "pc_phases_x",CHAN_PARAM)
    tokenize (PC_PHASE_Y_,  "pc_phases_y",CHAN_PARAM)
    tokenize (PC_PHASE_OFFSET_L_,  "pc_phase_offset_l",FLOAT_PARAM)
    tokenize (PC_PHASE_OFFSET_R_,  "pc_phase_offset_r",FLOAT_PARAM)
    tokenize (PC_PHASE_OFFSET_X_,  "pc_phase_offset_x",FLOAT_PARAM)
    tokenize (PC_PHASE_OFFSET_Y_,  "pc_phase_offset_y",FLOAT_PARAM)
    tokenize (ION_WIN_,     "ion_win",    TWO_FLOAT_PARAM)
    tokenize (ION_NPTS_,    "ion_npts",   INT_PARAM)
    tokenize (INTERPOLATOR_,"interpolator",INT_PARAM)
    tokenize (ITERATE_,     "iterate",    INT_CONST + ITERATE)
    tokenize (SIMUL_,       "simul",      INT_CONST + SIMUL)
    tokenize (STATION_DELAY_,"station_delay", FLOAT_PARAM)
    tokenize (PC_DELAY_L_,  "pc_delay_l", FLOAT_PARAM)
    tokenize (PC_DELAY_R_,  "pc_delay_r", FLOAT_PARAM)
    tokenize (PC_DELAY_X_,  "pc_delay_x", FLOAT_PARAM)
    tokenize (PC_DELAY_Y_,  "pc_delay_y", FLOAT_PARAM)
    tokenize (WEAK_CHANNEL_,"weak_channel", FLOAT_PARAM)
    tokenize (PC_AMP_HCODE_,"pc_amp_hcode", FLOAT_PARAM)
    tokenize (FMATCH_BW_PCT_,"fmatch_bw_pct", FLOAT_PARAM)
    tokenize (FILE_,        "file",       INT_CONST + PHYLE)
    tokenize (ADHOC_FILE_,  "adhoc_file", STRING_PARAM)
    tokenize (ADHOC_FILE_CHANS_, "adhoc_file_chans", STRING_PARAM)
    tokenize (ADHOC_FLAG_FILE_,  "adhoc_flag_file", STRING_PARAM)
    tokenize (PLOT_DATA_DIR_,  "plot_data_dir", STRING_PARAM)
    tokenize (MBD_ANCHOR_,  "mbd_anchor", INT_PARAM)
    tokenize (MODEL_,       "model",      INT_CONST + MODEL)
    tokenize (SBD_,         "sbd",        INT_CONST + SBD)
    tokenize (SAMPLER_DELAY_L_,"sampler_delay_l", VECTOR_FLOAT_PARAM)
    tokenize (SAMPLER_DELAY_R_,"sampler_delay_r", VECTOR_FLOAT_PARAM)
    tokenize (SAMPLER_DELAY_X_,"sampler_delay_x", VECTOR_FLOAT_PARAM)
    tokenize (SAMPLER_DELAY_Y_,"sampler_delay_y", VECTOR_FLOAT_PARAM)
    tokenize (ION_SMOOTH_,  "ion_smooth", INT_PARAM)
    tokenize (EST_PC_MANUAL_, "est_pc_manual", INT_PARAM)
    tokenize (CHAN_IDS_,    "chan_ids",   CHAN_PARAM)
    tokenize (CLONE_IDS_,   "clone_ids",  STRING_PARAM)
    tokenize (DISPLAY_CHANS_,"display_chans", STRING_PARAM)
    tokenize (CLONE_SNR_CHK_,"clone_snr_chk", INT_PARAM)
    tokenize (VBP_CORRECT_, "vbp_correct", INT_PARAM)
    tokenize (VBP_FIT_,     "vbp_fit",    INT_PARAM)
    tokenize (VBP_COEFFS_,  "vbp_coeffs", VECTOR_FLOAT_PARAM)
    tokenize (VBP_FILE_,    "vbp_file",   STRING_PARAM)
    tokenize (MOUNT_TYPE_,    "mount_type",   INT_PARAM)
    tokenize (NO_MOUNT_TYPE_, "no_mount",     INT_CONST + NO_MOUNT_TYPE)
    tokenize (CASSEGRAIN_,    "cassegrain",   INT_CONST + CASSEGRAIN)
    tokenize (NASMYTHLEFT_,   "nasmythleft",  INT_CONST + NASMYTHLEFT)
    tokenize (NASMYTHRIGHT_,  "nasmythright", INT_CONST + NASMYTHRIGHT)
    tokenize (MIXED_MODE_ROT_, "mixed_pol_yshift90", INT_PARAM)
    tokenize (NOAUTOFRINGES_, "noautofringes", INT_PARAM)
    tokenize (MOD4NUMBERING_, "mod4numbering", INT_PARAM)
    tokenize (POLFRINGNAMES_, "polfringnames", INT_PARAM)
    tokenize (MBDRPLOPT_,     "mbdrplopt",     VECTOR_INT_PARAM)
    tokenize (FRINGEOUT_DIR_, "fringeout_dir", STRING_PARAM)
    /*       token #        string        category      */

    }
/* eof */
