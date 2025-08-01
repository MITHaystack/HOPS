/*******************************************************************************
*                                                                              *
* parser uses a table-driven finite-state-machine to verify the syntactic      *
*        validity of the input token string, and to use the parameters and     *
*        conditions to generate a chained string of control blocks.            *
*                                                                              *
*                                                   rjc  92.12.22              *
*******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"
#include "control.h"
#include "ffcontrol.h"
#include "mk4_sizes.h"
#include "msg.h"
//#include "ff_misc_if.h"

/* eliminate some messages */
extern int    nullify_cblock (struct c_block *cb_ptr);
static int append_cblocks (struct c_block **cb_start, struct c_block **cb_end, int num);
static void parsing_error (int state_num, int ntok);





#define FALSE 0
#define TRUE 1
                                    // multiple pols. mapped into index 0 and 1
#define LXH 0
#define RYV 1




/*******************************************************************************
*    append_cblocks appends an arbitrary number (num) of c_blocks to the end   *
*    of the current chain. On entry, cb_end points to the pointer to the tail  *
*    of the current c_block chain. Pointers to the start and end of the new    *
*    section are returned in *cb_start and *cb_end.        rjc  92.2.19        *
*******************************************************************************/

static int append_cblocks (struct c_block **cb_start, struct c_block **cb_end, int num)
   {
   int i;
   struct c_block *cb_ptr;

   for (i=0; i<num; i++)
      {
      if ((cb_ptr = (struct c_block *) malloc (sizeof (struct c_block))) == NULL)
         {
         msg ("Error allocating c_block memory.",2);
         return (-1);
         }

      if (i == 0)
         *cb_start = cb_ptr;     /* need to return pointer to first new block */

      nullify_cblock (cb_ptr);                     /* nullify the new c_block */

      (*cb_end) -> cb_chain = cb_ptr;           /* splice into existing chain */
      *cb_end = cb_ptr;
      }
   return (0);
   }



/*******************************************************************************
*     parsing_error reports an error in parsing of the control file            *
*                                                                              *
*     Input:                                                                   *
*       state_num   - number of the current state in the FSM table             *
*       ntok        - token number of the encountered token                    *
*                                                                              *
*       94.1.13  rjc  initial code                                             *
*******************************************************************************/

static void parsing_error (int state_num, int ntok)
   {
   extern struct token_struct *tokens;   /* input struct of tokens & values   */
   extern char *token_string[];

   char *state[MAX_STATES];

                                    /* Initialize names of various FSM states */
   state[BLOCK_INTERIOR]    = "BLOCK_INTERIOR";    
   state[NEED_INT]          = "NEED_INT";         
   state[NEED_FLOAT]        = "NEED_FLOAT";      
   state[NEED_TWO_FLOAT_1]  = "NEED_TWO_FLOAT_1";
   state[NEED_TWO_FLOAT_2]  = "NEED_TWO_FLOAT_2";
   state[NEED_VECTOR_INT]   = "NEED_VECTOR_INT";
   state[NEED_VECTOR_FLOAT] = "NEED_VECTOR_FLOAT";
   state[NEED_CONDITION]    = "NEED_CONDITION";  
   state[NEED_F_GROUP]      = "NEED_F_GROUP";   
   state[END_STATE]         = "END_STATE";     
   state[NEED_STATION]      = "NEED_STATION"; 
   state[NEED_SCAN]         = "NEED_SCAN";   
   state[NEED_SOURCE]       = "NEED_SOURCE"; 
   state[NEED_BASELINE]     = "NEED_BASELINE";  
   state[NEED_VECTOR_CHAR]  = "NEED_VECTOR_CHAR";  
   state[NEED_CODES]        = "NEED_CODES";        
   state[NEED_OR]           = "NEED_OR";        
   state[MAY_HAVE_TO]       = "MAY_HAVE_TO";  
   state[NEED_2ND_SCAN]     = "NEED_2ND_SCAN";  

   msg ("Parser semantic error, line %d of control file:",2,tokens[ntok].line);
   msg ("In state %s, encountered illegal token %s", 2,
         state[state_num], token_string[tokens[ntok].symbol]);
   }

inline int fcode (char c, char *codes)
	{
	int i;
	
	for (i = 0; i < MAXFREQ; i++)
		if (c == codes[i]) 
            return i;
    return -1;
	}

// easier to put this in a separate file
#define ALLOW_FRQS_CODE
#include <ctype.h>
#include "frqsupport.h"
#undef  ALLOW_FRQS_CODE

// Made master_codes static; as an automatic variable
// its contents are unpredictable and we want it to
// persist between calls to the parser().  As str
// operations are called NULL termination is needed.
static char master_codes[MAXFREQ+1];
// flag that frqs, rather than freqs has been used
// at some time during the lex/parser cycles.
static int withfrqs = 0;

extern struct token_struct *tokens;   /* input struct of tokens & values   */
extern double *float_values;          /* array of actual fl. pt. values    */
extern struct fsm_table_entry *fsm_base;             /* start of fsm table */
extern struct c_block *cb_head;                  /* start of c_block chain */
extern char *char_values;             /* pointer to array of actual strings*/
extern int msglev;

int parser (void)
   {
   int i, tctr = (msglev <= -2)?0:-1, need_is_save,
       nstat, ntok, next_state, found, action, tnum, tval, toknum,
       negation, nv, sideband, parsed_scan[2], parsed_knot[5], ns;
   char parsed_f_group, parsed_station, parsed_baseline[2],
        parsed_source[32], parsed_codes[MAXFREQ+1], *psc, chan[4];
   float fval;      // historical choice
   double dval;     // some things require this

   struct c_block *cond_start,
                  *cb_start,      /* start of appplicable blocks in the event
                                                    of a complex IF condition */
                  *cb_ptr,
                  *cb_tail;                 /* points to last cblock in chain */

   if (master_codes[0] == '\0')
       strncpy (master_codes, FCHARS, sizeof (master_codes));
   parsed_codes[MAXFREQ] =
   master_codes[MAXFREQ] = '\0';

   msg ("Parser triples are token_number:state:category", -2);

   ntok = 0;                   /* point to beginning of input token structure */

   negation = FALSE;                     /* start out not negating conditions */

                                        /* find tail of current c_block chain */
   if (cb_head != NULL)
      for (cb_ptr=cb_head; cb_ptr != NULL; cb_ptr=cb_ptr->cb_chain)
         cb_tail = cb_ptr;

                                        /* allocate space for generic c_block */
   if ((cb_ptr =  (struct c_block *) malloc (sizeof (struct c_block))) == NULL)
      {
      msg ("Error allocating c_block memory.",2);
      return (-1);
      }

   nullify_cblock (cb_ptr);                    /* nullify the generic c_block */

   if (cb_head == NULL)
      cb_head = cb_ptr;                             /* this is start of chain */

   else
      {
      cb_tail->cb_chain = cb_ptr;                       /* chain in new block */
      cb_tail = cb_ptr;                                  /* and readjust tail */
      }

   cond_start = cb_ptr;               /* initialize condition block start ptr */

   next_state = BLOCK_INTERIOR;                    /* init. to starting state */

   do
      {
      found = FALSE;

                        /* loop over states to find matching state transition */

      if (msglev <= -2)                    /* replace with new msg if no lf's */
          {
          /* print line feed after every 9 tokens */
          if (tctr % 9 == 0) fprintf (stderr, "%s: ", progname);
          fprintf (stderr, " %d:%d:%d",ntok,next_state,tokens[ntok].category);
          if ((++tctr % 9) == 0) { fprintf (stderr, "\n"); fflush(stderr); }
          }

      for (nstat=0; fsm_base[nstat].current_state != 0; nstat++)
         if (fsm_base[nstat].current_state == next_state
          && (   fsm_base[nstat].token_type == tokens[ntok].category
              || fsm_base[nstat].token_type == MATCH_ALL))
            {
            next_state = fsm_base[nstat].next_state;
            action = fsm_base[nstat].action;
            tval = tokens[ntok].value;
            tnum = tokens[ntok].symbol;
            found = TRUE;
            break;
            }

      if (found)
         {
         switch (action)         /* perform action appropriate for this state */
            {
            case CLEAR_FREQS:                /* clear all freq codes to start */
               for (cb_ptr=cond_start; cb_ptr!=NULL; cb_ptr=cb_ptr->cb_chain)
                  for (i=0; i<MAXFREQ; i++)
                     cb_ptr->accept_sbs[i] = 0;
                                                /* and then save token number */
               
            case SAVE_TOKEN_NUM:
               toknum = tnum;
               nv = 0;                          /* clear vector element index */
               // if it is a CHAN_PARAM requesting SAVE_TOKEN_NUM,
               // then we need to note that if frqs was ever used.
               if (tctr>=0) { fprintf (stderr, "\n"); tctr=0; }
               need_is_save = (frqs_CHAN_PARAM(toknum) && withfrqs) ? 1 : 0;
               break;


            case INSERT_PAR:    /* insert scalar integer and float parameters */
               for (cb_ptr=cond_start; cb_ptr!=NULL; cb_ptr=cb_ptr->cb_chain)
                   if (toknum == X_CRC_)
                       cb_ptr -> x_crc = tval;
                   else if (toknum == Y_CRC_)
                       cb_ptr -> y_crc = tval;
                   else if (toknum == X_SLIP_SYNC_)
                       cb_ptr -> x_slip_sync = tval;
                   else if (toknum == Y_SLIP_SYNC_)
                       cb_ptr -> y_slip_sync = tval;
                   else if (toknum == ADHOC_PHASE_)
                       cb_ptr -> adhoc_phase = tval;
                   else if (toknum == SKIP_)
                       cb_ptr -> skip = TRUE;
                   else if (toknum == MIN_WEIGHT_)
                       cb_ptr -> min_weight = float_values[tval];
                   else if (toknum == REF_FREQ_)
                       cb_ptr -> ref_freq = float_values[tval];
                   else if (toknum == RA_OFFSET_)
                       cb_ptr -> ra_offset = float_values[tval];
                   else if (toknum == DEC_OFFSET_)
                       cb_ptr -> dec_offset = float_values[tval];
                   else if (toknum == ADHOC_TREF_)
                       cb_ptr -> adhoc_tref = float_values[tval];
                   else if (toknum == ADHOC_PERIOD_)
                       {
                       if (float_values[tval] > 0)
                           cb_ptr -> adhoc_period = float_values[tval];
                       else
                           {
                           msg ("Ad hoc period must be greater than 0!",2);
                           return (-1);
                           }
                       }
                   else if (toknum == ADHOC_AMP_)
                       cb_ptr -> adhoc_amp = float_values[tval];
                   else if (toknum == PC_MODE_)      /* insert phase cal mode */
                       {                             /* into non-wildcard stn */
                       if (cb_ptr -> baseline[0] == WILDCARD)
                           cb_ptr -> pc_mode.rem = tval; 
                       if (cb_ptr -> baseline[1] == WILDCARD)
                           cb_ptr -> pc_mode.ref = tval;
                       }
                   else if (toknum == PC_PERIOD_)    // insert phase cal period
                       {                             // into non-wildcard stn
                       if (cb_ptr -> baseline[0] == WILDCARD)
                           cb_ptr -> pc_period.rem = tval; 
                       if (cb_ptr -> baseline[1] == WILDCARD)
                           cb_ptr -> pc_period.ref = tval;
                       }
                   else if (toknum == LSB_OFFSET_)       /* insert LSB offset */
                       {
                       if (cb_ptr -> baseline[0] == WILDCARD)
                           cb_ptr -> lsb_offset.rem = float_values[tval];
                       if (cb_ptr -> baseline[1] == WILDCARD)
                           cb_ptr -> lsb_offset.ref = float_values[tval];
                       }
                   else if (toknum == START_)
                       cb_ptr -> time_span[0] = tval;
                   else if (toknum == STOP_)
                       cb_ptr -> time_span[1] = tval;
                   else if (toknum == SWITCHED_)
                       cb_ptr -> switched_mode = tval;
                   else if (toknum == PERIOD_)
                       cb_ptr -> switched_period = tval;
                   else if (toknum == USE_SAMPLES_)
                       cb_ptr -> use_samples = tval;
                   else if (toknum == T_COHERE_)
                       cb_ptr -> t_cohere = float_values[tval];
                   else if (toknum == IONOSPHERE_)
                       {
                       if (cb_ptr -> baseline[0] == WILDCARD)
                           cb_ptr -> ionosphere.rem = float_values[tval];
                       if (cb_ptr -> baseline[1] == WILDCARD)
                           cb_ptr -> ionosphere.ref = float_values[tval];
                       }
                   else if (toknum == STATION_DELAY_)
                       {
                       if (cb_ptr -> baseline[0] == WILDCARD)
                           cb_ptr -> station_delay.rem = 1e-9 * float_values[tval];
                       if (cb_ptr -> baseline[1] == WILDCARD)
                           cb_ptr -> station_delay.ref = 1e-9 * float_values[tval];
                       }
                   else if (toknum == PC_DELAY_L_
                         || toknum == PC_DELAY_X_)
                       {
                       if (cb_ptr -> baseline[0] == WILDCARD)
                           cb_ptr -> pc_delay_l.rem = 1e-9 * float_values[tval];
                       if (cb_ptr -> baseline[1] == WILDCARD)
                           cb_ptr -> pc_delay_l.ref = 1e-9 * float_values[tval];
                       }
                   else if (toknum == PC_DELAY_R_ 
                         || toknum == PC_DELAY_Y_)
                       {
                       if (cb_ptr -> baseline[0] == WILDCARD)
                           cb_ptr -> pc_delay_r.rem = 1e-9 * float_values[tval];
                       if (cb_ptr -> baseline[1] == WILDCARD)
                           cb_ptr -> pc_delay_r.ref = 1e-9 * float_values[tval];
                       }
                   else if (toknum == PC_PHASE_OFFSET_L_
                         || toknum == PC_PHASE_OFFSET_X_)
                       {
                       if (cb_ptr -> baseline[0] == WILDCARD)
                           cb_ptr -> pc_phase_offset[LXH].rem = float_values[tval];
                       if (cb_ptr -> baseline[1] == WILDCARD)
                           cb_ptr -> pc_phase_offset[LXH].ref = float_values[tval];
                       }
                   else if (toknum == PC_PHASE_OFFSET_R_
                         || toknum == PC_PHASE_OFFSET_Y_)
                       {
                       if (cb_ptr -> baseline[0] == WILDCARD)
                           cb_ptr -> pc_phase_offset[RYV].rem = float_values[tval];
                       if (cb_ptr -> baseline[1] == WILDCARD)
                           cb_ptr -> pc_phase_offset[RYV].ref = float_values[tval];
                       }
                   else if (toknum == DC_BLOCK_)
                       cb_ptr -> dc_block = tval;
                   else if (toknum == SAMPLERS_)
                       {
                       if (tval >= MAX_SAMP)
                           {
                           msg ("too many samplers specified", 2);
                           return (-1);
                           }
                       cb_ptr -> nsamplers = tval;
                       ns = 0;           // next string encountered will be 0th
                       psc = cb_ptr -> sampler_codes;  // point to beg of array
                       }
                   else if (toknum == GEN_CF_RECORD_)
                       cb_ptr -> gen_cf_record = tval;
                   else if (toknum == OPTIMIZE_CLOSURE_)
                       cb_ptr -> optimize_closure = tval;
                   else if (toknum == ION_NPTS_)
                       cb_ptr -> ion_npts = tval;
                   else if (toknum == INTERPOLATOR_)
                       cb_ptr -> interpolator = tval;
                   else if (toknum == WEAK_CHANNEL_)
                       cb_ptr -> weak_channel = float_values[tval];
                   else if (toknum == PC_AMP_HCODE_)
                       cb_ptr -> pc_amp_hcode = float_values[tval];
                   else if (toknum == FMATCH_BW_PCT_)
                       cb_ptr -> fmatch_bw_pct = float_values[tval];
                   else if (toknum == MBD_ANCHOR_)
                       cb_ptr -> mbd_anchor = tval;
                   else if (toknum == ION_SMOOTH_)
                       cb_ptr -> ion_smooth = tval;
                   else if (toknum == EST_PC_MANUAL_)
                       cb_ptr -> est_pc_manual = tval;
                   else if (toknum == CLONE_SNR_CHK_)
                       cb_ptr -> clone_snr_chk = tval;
                   else if (toknum == VBP_CORRECT_)
                       cb_ptr -> vbp_correct = tval;
                   else if (toknum == VBP_FIT_)
                       cb_ptr -> vbp_fit = tval;

                   else if (toknum == MOUNT_TYPE_)
                       {
                       if (cb_ptr -> baseline[1] == WILDCARD)      // ref station
                           cb_ptr -> mount_type[0] = tval;
                       else if (cb_ptr -> baseline[0] == WILDCARD) // rem station
                           cb_ptr -> mount_type[1] = tval;
                       }
                   else if (toknum == MIXED_MODE_ROT_)
                       cb_ptr -> mixed_mode_rot = tval;
                   else if (toknum == NOAUTOFRINGES_)
                       cb_ptr -> noautofringes = tval;
                   else if (toknum == MOD4NUMBERING_)
                       cb_ptr -> mod4numbering = tval;
                   else if (toknum == POLFRINGNAMES_)
                       cb_ptr -> polfringnames = tval;

               break;


            case INSERT_V_PAR:
               for (cb_ptr=cond_start; cb_ptr!=NULL; cb_ptr=cb_ptr->cb_chain)
                   if (toknum == INDEX_)    // deprecated
                       {
                       if (nv == 2*MAXFREQ)
                           {
                           msg ("Too many index numbers",2);
                           return (-1);
                           }
                       cb_ptr -> index[nv] = tval;
                       }

                   else if (toknum == PC_PHASE_)  /* is this phase cal phase? */
                       {
                       i = fcode(parsed_codes[nv], master_codes);
                       if (i<0 || i>MAXFREQ-1)
                           {
                           msg ("Invalid phase cal frequency code",2);
                           return (-1);
                           }
                                         /* get phases from appropriate place */
                       if (tokens[ntok].category == INTEGER)
                           fval = tval;
                       else
                           fval = float_values[tval];

                                                   /* phase normally gets stored
                                                      for only one station, and
                                                      into correct freq slot.
                                                      If both specified, set remote
                                                      phase cal to value, and zero
                                                      out the reference value. */
                       if (cb_ptr -> baseline[1] == WILDCARD)
                           {        // both polarizations get the same value
                           cb_ptr -> pc_phase[i][LXH].ref = fval;
                           cb_ptr -> pc_phase[i][RYV].ref = fval;
                           }
                       else if (cb_ptr -> baseline[0] == WILDCARD)
                           {
                           cb_ptr -> pc_phase[i][LXH].rem = fval;
                           cb_ptr -> pc_phase[i][RYV].rem = fval;
                           }
                       else 
                           {
                           cb_ptr -> pc_phase[i][LXH].ref = 0.0;
                           cb_ptr -> pc_phase[i][RYV].ref = 0.0;
                           cb_ptr -> pc_phase[i][LXH].rem = fval;
                           cb_ptr -> pc_phase[i][RYV].rem = fval;
                           }
                       }

                   else if (toknum == PC_PHASE_L_
                         || toknum == PC_PHASE_X_)  // is this L/X/H phase cal phase?
                       {
                       i = fcode(parsed_codes[nv], master_codes);
                       if (i<0 || i>MAXFREQ-1)
                           {
                           msg ("Invalid phase cal frequency code",2);
                           return (-1);
                           }
                                         /* get phases from appropriate place */
                       if (tokens[ntok].category == INTEGER)
                           fval = tval;
                       else
                           fval = float_values[tval];

                                                   /* phase normally gets stored
                                                      for only one station, and
                                                      into correct freq slot.
                                                      If both specified, set remote
                                                      phase cal to value, and zero
                                                      out the reference value. */
                       if (cb_ptr -> baseline[1] == WILDCARD)
                           cb_ptr -> pc_phase[i][LXH].ref = fval;
                       else if (cb_ptr -> baseline[0] == WILDCARD)
                           cb_ptr -> pc_phase[i][LXH].rem = fval;
                       else 
                           {
                           cb_ptr -> pc_phase[i][LXH].ref = 0.0;
                           cb_ptr -> pc_phase[i][LXH].rem = fval;
                           }
                       }

                   else if (toknum == PC_PHASE_R_
                         || toknum == PC_PHASE_Y_)  // is this R/Y/V phase cal phase?
                       {
                       i = fcode(parsed_codes[nv], master_codes);
                       if (i<0 || i>MAXFREQ-1)
                           {
                           msg ("Invalid phase cal frequency code",2);
                           return (-1);
                           }
                                         /* get phases from appropriate place */
                       if (tokens[ntok].category == INTEGER)
                           fval = tval;
                       else
                           fval = float_values[tval];

                                                   /* phase normally gets stored
                                                      for only one station, and
                                                      into correct freq slot.
                                                      If both specified, set remote
                                                      phase cal to value, and zero
                                                      out the reference value. */
                       if (cb_ptr -> baseline[1] == WILDCARD)
                           cb_ptr -> pc_phase[i][RYV].ref = fval;
                       else if (cb_ptr -> baseline[0] == WILDCARD)
                           cb_ptr -> pc_phase[i][RYV].rem = fval;
                       else 
                           {
                           cb_ptr -> pc_phase[i][RYV].ref = 0.0;
                           cb_ptr -> pc_phase[i][RYV].rem = fval;
                           }
                       }

                   else if (toknum == PC_FREQ_)    /* is this phase cal freq? */
                       {
                       i = fcode(parsed_codes[nv], master_codes);
                       if (i<0 || i>MAXFREQ-1)
                           {
                           msg ("Invalid phase cal frequency code",2);
                           return (-1);
                           }
                                          /* get freqs from appropriate place */
                       if (tokens[ntok].category == INTEGER)
                           fval = tval;
                       else
                           fval = float_values[tval];
                                                   /* freq normally gets stored
                                                      for only one station, and
                                                      into correct freq slot  */
                       if (cb_ptr -> baseline[1] == WILDCARD)
                           {
                           cb_ptr -> pc_freq[i].ref = fval;
                                          /* if both are wild cards, set both */
                           if (cb_ptr -> baseline[0] == WILDCARD)
                               cb_ptr -> pc_freq[i].rem = fval;
                           }
                       else if (cb_ptr -> baseline[0] == WILDCARD)
                           cb_ptr -> pc_freq[i].rem = fval;
                       else                     /* both stations are specific */
                           {
                           cb_ptr -> pc_freq[i].ref = fval;
                           cb_ptr -> pc_freq[i].rem = fval;
                           }
                       }


                   else if (toknum == PC_TONEMASK_)  // pcal tone exclusion mask?
                       {
                       i = fcode(parsed_codes[nv], master_codes);
                       if (i<0 || i>MAXFREQ-1)
                           {
                           msg ("Invalid pcal freq code in tonemask",2);
                           return (-1);
                           }
                                                   // tonemask normally gets stored
                                                   // for only one station, and
                                                   // into correct freq chan slot
                       if (cb_ptr -> baseline[1] == WILDCARD)
                           cb_ptr -> pc_tonemask[i].ref = tval;
                       if (cb_ptr -> baseline[0] == WILDCARD)
                           cb_ptr -> pc_tonemask[i].rem = tval;
                       }


                   else if (toknum == GATES_) /*are these freq. switch gates? */
                       {
                       i = fcode(parsed_codes[nv/2], master_codes);
                       if (i<0 || i>MAXFREQ-1)
                           {
                           msg ("Invalid gates frequency code",2);
                           return (-1);
                           }
                                          /* get gates from appropriate place */
                       if (tokens[ntok].category == INTEGER)
                           fval = tval;
                       else
                           fval = float_values[tval];
                       
                       if (nv % 2)                   /* on_delay or duration? */
                           cb_ptr -> gates[i].duration = fval;
                       else
                           cb_ptr -> gates[i].on_delay = fval;
                       }

                   else if (toknum == SB_WIN_)  
                       {
                       if (nv > 1)
                           {
                           msg ("Too many sb_win numbers",2);
                           return (-1);
                           }
                       cb_ptr -> sb_window[nv] = 
                           (tokens[ntok].category == INTEGER) ?  tval : float_values[tval];
                       }

                   else if (toknum == MB_WIN_)
                       {
                       if (nv > 1)
                           {
                           msg ("Too many mb_win numbers",2);
                           return (-1);
                           }
                       cb_ptr -> mb_window[nv] = 
                           (tokens[ntok].category == INTEGER) ?  tval : float_values[tval];
                       }

                   else if (toknum == DR_WIN_)
                       {
                       if (nv > 1)
                           {
                           msg ("Too many dr_win numbers",2);
                           return (-1);
                           }
                       cb_ptr -> dr_window[nv] = 
                           (tokens[ntok].category == INTEGER) ?  tval : float_values[tval];
                       }

                   else if (toknum == ION_WIN_)
                       {
                       if (nv > 1)
                           {
                           msg ("Too many ion_win numbers",2);
                           return (-1);
                           }
                       cb_ptr -> ion_window[nv] = 
                           (tokens[ntok].category == INTEGER) ?  tval : float_values[tval];
                       }

                   else if (toknum == ADHOC_POLY_)
                       {
                       if (nv > 5)
                           {
                           msg ("More than max. of 6 phase polynomial numbers",2);
                           return (-1);
                           }
                                   /* get coefficients from appropriate place */
                       if (tokens[ntok].category == INTEGER)
                           fval = tval;
                       else
                           fval = float_values[tval];

                       cb_ptr -> adhoc_poly[nv] = fval;
                       }

                   else if (toknum == VBP_COEFFS_)
                       {
                       if (nv > 4)
                           {
                           msg ("Must have 5 video bandpass coeffs",2);
                           return (-1);
                           }
                                    // treat integer and float numbers the same way
                       if (tokens[ntok].category == INTEGER)
                           fval = tval;
                       else
                           fval = float_values[tval];
                                    // handle specification by either stn or baseline
                       if (cb_ptr -> baseline[1] == WILDCARD)
                           cb_ptr -> vbp_coeffs[nv].ref = fval;
                       else if (cb_ptr -> baseline[0] == WILDCARD)
                           cb_ptr -> vbp_coeffs[nv].rem = fval;
                       else
                           {
                           cb_ptr -> vbp_coeffs[nv].ref = 0.0;
                           cb_ptr -> vbp_coeffs[nv].rem = fval;
                           }
                       }

                   else if (toknum == NOTCHES_)
                       {
                       if (nv > MAXNOTCH*2)
                           {
                           msg ("Only %d notches are allowed",2,MAXNOTCH);
                           return (-1);
                           }
                       if (tokens[ntok].category == INTEGER)
                           fval = tval;
                       else
                           fval = float_values[tval];

                       cb_ptr -> notches[nv / 2][nv % 2] = fval;
                       cb_ptr -> nnotches = (1 + nv) / 2;
                       }

                   else if (toknum == PASSBAND_)
                       {
                       if (nv > 1)
                           {
                           msg ("Too many passband numbers",2);
                           return (-1);
                           }
                       if (tokens[ntok].category == INTEGER)
                           cb_ptr -> passband[nv] = tval;
                       else
                           cb_ptr -> passband[nv] = float_values[tval];
                       }

                   else if (toknum == AVXPZOOM_)
                       {
                       if (nv > 1)
                           {
                           msg ("Too many avxpzoom numbers",2);
                           return (-1);
                           }
                       if (tokens[ntok].category == INTEGER)
                           cb_ptr -> avxpzoom[nv] = tval;
                       else
                           cb_ptr -> avxpzoom[nv] = float_values[tval];
                       /* check for legal values */
                       if (nv == 1 && (cb_ptr->avxpzoom[0] < 0.0 ||
                           cb_ptr->avxpzoom[0] > 1.0 ||
                           cb_ptr->avxpzoom[1] > 1.0))
                           {
                           msg ("illegal avxpzoom paramters", 2);
                           return (-1);
                           }
                       }

                   else if (toknum == AVXPLOPT_)
                       {
                       if (nv > 1)
                           {
                           msg ("Too many avxplopt numbers",2);
                           return (-1);
                           }
                       if (tokens[ntok].category == INTEGER)
                           {
                           cb_ptr -> avxplopt[nv] = tval;
                           }
                       else
                           {
                           msg ("avxplopt numbers must be integers",2);
                           return (-1);
                           }
                       /* check for legal values */
                       if (nv == 1 && (!(cb_ptr->avxplopt[nv] == -7 ||
                           cb_ptr->avxplopt[nv] == -1 ||
                           cb_ptr->avxplopt[nv] == 0 ||
                           cb_ptr->avxplopt[nv] == 1 ||
                           cb_ptr->avxplopt[nv] == 7)))
                           {
                           msg ("illegal avxplopt values",2);
                           return (-1);
                           }
                       }

                   else if (toknum == MBDRPLOPT_)
                       {
                       if (nv > 2)
                           {
                           msg ("Too many mbdrplopt integers",2);
                           return (-1);
                           }
                       if (tokens[ntok].category == INTEGER)
                           {
                           cb_ptr -> mbdrplopt[nv] = tval;
                           }
                       else
                           {
                           msg ("mbdrplopt numbers must be integers",2);
                           return (-1);
                           }
                       }

                   else if (toknum == DELAY_OFFS_) // is this a channel delay offset?
                       {
                       i = fcode(parsed_codes[nv], master_codes);
                       if (i<0 || i>MAXFREQ-1)
                           {
                           msg ("Invalid delay offset frequency code",2);
                           return (-1);
                           }
                                         /* get phases from appropriate place */
                       if (tokens[ntok].category == INTEGER)
                           fval = tval;
                       else
                           fval = float_values[tval];
                                                   /* delays normally get stored
                                                      for only one station, and
                                                      into correct freq slot.
                                                      If both specified, set remote
                                                      delay offset to value, and zero
                                                      out the reference value. */
                       if (cb_ptr -> baseline[1] == WILDCARD)
                           cb_ptr -> delay_offs[i].ref = fval;
                       else if (cb_ptr -> baseline[0] == WILDCARD)
                           cb_ptr -> delay_offs[i].rem = fval;
                       else 
                           {
                           cb_ptr -> delay_offs[i].ref = 0.0;
                           cb_ptr -> delay_offs[i].rem = fval;
                           // Consider making this illegal to avoid pilot error.
                           }
                       }

                   else if (toknum == DELAY_OFFS_L_ || toknum == DELAY_OFFS_X_) 
                       {
                       i = fcode(parsed_codes[nv], master_codes);
                       if (i<0 || i>MAXFREQ-1)
                           {
                           msg ("Invalid delay offset frequency code",2);
                           return (-1);
                           }
                                         /* get phases from appropriate place */
                       if (tokens[ntok].category == INTEGER)
                           fval = tval;
                       else
                           fval = float_values[tval];
                                         /* unclear what should happen if specified on a baseline */
                       if (cb_ptr -> baseline[1] == WILDCARD)
                           cb_ptr -> delay_offs_pol[i][LXH].ref = fval;
                       else if (cb_ptr -> baseline[0] == WILDCARD)
                           cb_ptr -> delay_offs_pol[i][LXH].rem = fval;
                       else
                           {
                           msg ("Must specify delay_offs_? on a station, not a baseline",2);
                           return (-1);
                           }
                       }

                   else if (toknum == DELAY_OFFS_R_ || toknum == DELAY_OFFS_Y_) 
                       {
                       i = fcode(parsed_codes[nv], master_codes);
                       if (i<0 || i>MAXFREQ-1)
                           {
                           msg ("Invalid delay offset frequency code",2);
                           return (-1);
                           }
                                         /* get phases from appropriate place */
                       if (tokens[ntok].category == INTEGER)
                           fval = tval;
                       else
                           fval = float_values[tval];
                                         /* unclear what should happen if specified on a baseline */
                       if (cb_ptr -> baseline[1] == WILDCARD)
                           cb_ptr -> delay_offs_pol[i][RYV].ref = fval;
                       else if (cb_ptr -> baseline[0] == WILDCARD)
                           cb_ptr -> delay_offs_pol[i][RYV].rem = fval;
                       else
                           {
                           msg ("Can only specify delay_offs_? on a station, not a baseline",2);
                           return (-1);
                           }
                       }

                   else if (toknum == SAMPLER_DELAY_L_ || toknum == SAMPLER_DELAY_X_)
                       {
                       if (nv >= MAX_SAMP)
                           {
                           msg ("Too many (%d) sampler delays specified", 2, nv+1);
                           return (-1);
                           }
                                   /* get coefficients from appropriate place */
                       if (tokens[ntok].category == INTEGER)
                           fval = tval;
                       else
                           fval = float_values[tval];
                       fval *= 1e-9;      // convert ns -> sec
                                          // insert to station, or rem if no wildcard
                       if (cb_ptr -> baseline[1] == WILDCARD)
                           cb_ptr -> sampler_delay[nv][LXH].ref = fval;
                       else if (cb_ptr -> baseline[0] == WILDCARD)
                           cb_ptr -> sampler_delay[nv][LXH].rem = fval;
                       else 
                           {
                           cb_ptr -> sampler_delay[nv][LXH].ref = fval;
                           cb_ptr -> sampler_delay[nv][LXH].rem = fval;
                           }
                       }

                   else if (toknum == SAMPLER_DELAY_R_ || toknum == SAMPLER_DELAY_Y_)
                       {
                       if (nv >= MAX_SAMP)
                           {
                           msg ("Too many (%d) sampler delays specified", 2, nv+1);
                           return (-1);
                           }
                                   /* get coefficients from appropriate place */
                       if (tokens[ntok].category == INTEGER)
                           fval = tval;
                       else
                           fval = float_values[tval];
                       fval *= 1e-9;      // convert ns -> sec
                                          // insert to station, or rem if no wildcard
                       if (cb_ptr -> baseline[1] == WILDCARD)
                           cb_ptr -> sampler_delay[nv][RYV].ref = fval;
                       else if (cb_ptr -> baseline[0] == WILDCARD)
                           cb_ptr -> sampler_delay[nv][RYV].rem = fval;
                       else 
                           {
                           cb_ptr -> sampler_delay[nv][RYV].ref = 0.0;
                           cb_ptr -> sampler_delay[nv][RYV].rem = fval;
                           }
                       }
                   else if (toknum == CHAN_IDS_)  // is this channel id override?
                       {
                       if (nv == 0)       // save ch string as new master copy
                           {
                           strncpy (master_codes, parsed_codes, MAXFREQ);
                           strncpy (cb_ptr->chid, parsed_codes, MAXFREQ);
                           // ensure null termination, anal compulsive, yes.
                           master_codes[MAXFREQ] = cb_ptr->chid[MAXFREQ] = '\0';
                           }
                                          // allow int or float input values
                       if (tokens[ntok].category == INTEGER)
                           dval = tval;
                       else
                           dval = float_values[tval];
                       cb_ptr -> chid_rf[nv] = dval;
                       }
               // end of for cb_ptr=cond_start...
               nv++;                       /* bump index for next vector parm */
               break;
               // end of case INSERT_V_PAR:



            case INSERT_V_CHAR:                 /* first figure out side band */
               if (toknum == CLONE_IDS_ && tnum != TWO_CHAR_)
                   {
                   msg("Clone ids must be 2,4,... characters", 3);
                   return(-2);
                   }
               if (tnum == ONE_CHAR_)
                   {
                   if (toknum == CHAN_NOTCHES_)
                       { // chan_notches case with just one notch -- handle & break
                       if (tctr>=0) { fprintf (stderr, "\n"); tctr=0; }
                       if (frqs_chan_notches(char_values+tval, master_codes, cond_start))
                           return(-3);  // appropriate complaint will already be made.
                       break;
                       }
                   else
                       { // freqs usage
                       chan[0] = char_values[tval];
                       sideband = DSB;
                       }
                   }
               else if (tnum == TWO_CHAR_)
                   {
                   if (toknum == CLONE_IDS_)
                       {
                       memcpy (chan, char_values+tval, 2);
                       chan[2] = '\0';  // null terminate
                       if (tctr>=0) { fprintf (stderr, "\n"); tctr=0; }
                       if (frqs_clone_ids(chan, master_codes, cond_start))
                           return(-4);  // appropriate complaint will already be made.
                       break;
                       }
                   else if (toknum == CHAN_NOTCHES_)
                       { // chan_notches case with two notches -- handle & break
                       if (tctr>=0) { fprintf (stderr, "\n"); tctr=0; }
                       if (frqs_chan_notches(char_values+tval, master_codes, cond_start))
                           return(-5);  // appropriate complaint will already be made.
                       break;
                       }
                   // otherwise continue with INSERT_V_CHAR for 2 chars
                   memcpy (chan, char_values+tval, 2);
                   if (chan[1] == '+')
                       sideband = USB;
                   else if (chan[1] == '-')
                       sideband = LSB;
                   else
                       sideband = -1;                        /* denotes error */
                   }
               else                                // must be an integer
                   {
                   if (tval >=0 && tval <= 9)
                       {
                       chan[0] = '0' + tval;
                       sideband = DSB;
                       }
                   else
                       chan[0] = '#';                 // null char will cause error
                       chan[1] = '#';
                   }

               i = fcode(chan[0], master_codes);     /* get freq. array index */
               if (i<0 || i>MAXFREQ-1 || sideband<0) /* trap error conditions */
                   {
                   msg ("Errant freq element: %c%c", 2, chan[0], chan[1]);
                   return (-1);
                   }
                                        /* OK, load into appropriate c_blocks */
               for (cb_ptr=cond_start; cb_ptr!=NULL; cb_ptr=cb_ptr->cb_chain)
                   cb_ptr -> accept_sbs[i] = sideband; 
               break;

           case INSERT_STRING:                   // handle all string arguments
               if (toknum == SAMPLERS_)
                   {               // add string and change concatenation point
                   strcat (psc, char_values+tval);
                   for (cb_ptr=cond_start; cb_ptr!=NULL; cb_ptr=cb_ptr->cb_chain)
                       cb_ptr -> psamplers[ns] = psc;
                   psc += strlen (char_values+tval) + 1;
                   ns++;
                                // transition if we've read all expected strings
                   if (ns == cond_start -> nsamplers)
                       next_state = BLOCK_INTERIOR;
                   }
                                // store adhoc (pcal) file names
               else if (toknum == ADHOC_FILE_)
                   for (cb_ptr=cond_start; cb_ptr!=NULL; cb_ptr=cb_ptr->cb_chain)
                       {
                       if (cb_ptr -> baseline[1] == WILDCARD)      // ref station
                           strncpy (cb_ptr -> adhoc_file[0], char_values+tval, 256);
                       else if (cb_ptr -> baseline[0] == WILDCARD) // rem station
                           strncpy (cb_ptr -> adhoc_file[1], char_values+tval, 256);
                       }
                                // store adhoc (pcal) file channels
               else if (toknum == ADHOC_FILE_CHANS_)
                   for (cb_ptr=cond_start; cb_ptr!=NULL; cb_ptr=cb_ptr->cb_chain)
                       {
                       if (cb_ptr -> baseline[1] == WILDCARD)      // ref station
                           strncpy (cb_ptr -> adhoc_file_chans[0], 
                                    char_values+tval, 128);
                       else if (cb_ptr -> baseline[0] == WILDCARD) // rem station
                           strncpy (cb_ptr -> adhoc_file_chans[1], 
                                    char_values+tval, 128);
                       }
               else if (toknum == ADHOC_FLAG_FILE_)
                   {
                   // update char_values+tval according to frqs rules
                   // this is the only case where STRING, rather than CHAN_PARMS
                   // was used for the channel list
                   if (withfrqs && tctr>=0) { fprintf (stderr, "\n"); tctr=0; }
                   char *revised_channels = withfrqs
                       ? frqs_expand_codes(char_values+tval, master_codes)
                       : char_values+tval;
                   if (withfrqs && !revised_channels)
                       {
                       msg ("Error ~expanding adhoc flag file channels.",2);
                       return (-1);
                       }
                   for (cb_ptr=cond_start; cb_ptr!=NULL; cb_ptr=cb_ptr->cb_chain)
                       {
                       if (cb_ptr -> baseline[1] == WILDCARD)      // ref station
                           strncpy (cb_ptr -> adhoc_flag_files[0],
                                    revised_channels, 256);
                       else if (cb_ptr -> baseline[0] == WILDCARD) // rem station
                           strncpy (cb_ptr -> adhoc_flag_files[1],
                                    revised_channels, 256);
                       }
                   if (withfrqs) free(revised_channels);
                   }
               else if (toknum == PLOT_DATA_DIR_)
                   for (cb_ptr=cond_start; cb_ptr!=NULL; cb_ptr=cb_ptr->cb_chain)
                       {
                       if (cb_ptr -> baseline[1] == WILDCARD)      // ref station
                           strncpy (cb_ptr -> plot_data_dir[0],
                                    char_values+tval, 256);
                       else if (cb_ptr -> baseline[0] == WILDCARD) // rem station
                           strncpy (cb_ptr -> plot_data_dir[1],
                                    char_values+tval, 256);
                       }
               else if (toknum == FRINGEOUT_DIR_)
                   for (cb_ptr=cond_start; cb_ptr!=NULL; cb_ptr=cb_ptr->cb_chain)
                       {
                       if (cb_ptr -> baseline[1] == WILDCARD ||
                           cb_ptr -> baseline[0] == WILDCARD)
                           strncpy (cb_ptr -> fringeout_dir, char_values+tval, 256);
                       }
                                // video bandpass file name
               else if (toknum == DISPLAY_CHANS_)
                   {
                   if (tctr>=0) { fprintf (stderr, "\n"); tctr=0; }
                   if (frqs_display_chans(char_values+tval, master_codes, cond_start))
                        return(-6);  // appropriate complaint will already be made.
                   }
               else if (toknum == VBP_FILE_)
                   for (cb_ptr=cond_start; cb_ptr!=NULL; cb_ptr=cb_ptr->cb_chain)
                       {
                       if (cb_ptr -> baseline[1] == WILDCARD)      // ref station
                           strncpy (cb_ptr -> vbp_file[0], char_values+tval, 256);
                       else if (cb_ptr -> baseline[0] == WILDCARD) // rem station
                           strncpy (cb_ptr -> vbp_file[1], char_values+tval, 256);
                       }
               else if (toknum == CHAN_NOTCHES_)
                   {
                   if (tctr>=0) { fprintf (stderr, "\n"); tctr=0; }
                   if (frqs_chan_notches(char_values+tval, master_codes, cond_start))
                       return(-7);  // appropriate complaint will already be made.
                   }
               else if (toknum == CLONE_IDS_)
                   {
                   if (tctr>=0) { fprintf (stderr, "\n"); tctr=0; }
                   if (frqs_clone_ids(char_values+tval, master_codes, cond_start))
                       return(-8);  // appropriate complaint will already be made.
                   }
               break;
               // end of INSERT_STRING

            case POP_TOKEN:
               ntok--;                                  /* stay on same token */
               break;

            case CLEAR_CONDS:                    /* clear condition variables */
               if (tnum == IF_)
                  cb_start = NULL;       /* range of condition pointers grows */
               else if (tnum == OR_)          /* when they are OR'ed together */
                  cb_start = cond_start;

               parsed_f_group = WILDCARD;
               parsed_station = WILDCARD;
               for (i=0; i<2; i++) 
                  {
                  parsed_baseline[i] = WILDCARD;
                  parsed_scan[i] = NULLINT;
                  }
               for (i=0; i<5; i++) 
                  parsed_knot[i] = FALSE;
               for (i=0; i<32; i++) 
                  parsed_source[i] = WILDCARD;
               break;

            case SAVE_FG:
               if (tnum == WILDCARD_)
                   parsed_f_group = WILDCARD;
               else
                   parsed_f_group = char_values[tval];
               parsed_knot[2] = negation;
               negation = FALSE;
               break;

            case SAVE_STAT:
               if (tnum == WILDCARD_)
                   parsed_station = WILDCARD;
               else
                   parsed_station = char_values[tval];
               parsed_knot[4] = negation;
               negation = FALSE;
               break;

            case SAVE_BASE:
               memcpy (parsed_baseline, char_values + tval, 2);
               parsed_knot[0] = negation;
               negation = FALSE;
               break;

            case SAVE_SCAN:
               parsed_scan[0] = tval;
               parsed_scan[1] = tval;
               parsed_knot[3] = negation;
               negation = FALSE;
               break;

            case SAVE_2ND_SCAN:
               if (toknum == TO_)                /* overwrite end of interval */
                  parsed_scan[1] = tval;

               else if (toknum == LESS_THAN_)
                  {
                  parsed_scan[0] = 0;           /* start at beginning of year */
                  parsed_scan[1] = tval - 1;   /* note: interval is inclusive */
                  }

               else if (toknum == GREATER_THAN_)
                  {
                  parsed_scan[0] = tval + 1;
                  parsed_scan[1] = 99999999;          /* include rest of year */
                  }

               break;

            case SAVE_SOURCE:
               if (tnum != WILDCARD_)
                   {
                   memset (parsed_source,' ',32);           /* pad with blanks */
                   i = strlen (char_values + tval);
                   i = (i > 32) ? 32 : i;               /* move at most 32 chars */
                   memcpy (parsed_source, char_values + tval, i);
                   }
               parsed_knot[1] = negation;
               negation = FALSE;
               break;

            case NEW_CODES:
               withfrqs++;
            case SAVE_CODES:
               memset (parsed_codes,'\0',MAXFREQ+1);     /* pad with nulls */
               i = strlen (char_values + tval);
               i = (i > MAXFREQ) ? MAXFREQ : i;      /* move at most MAXFREQ chars */
               // should probably complain about that.
               if (need_is_save) {   /* SAVE_CODES for CHAN_PARAM */
                   /* re-interpret parsed_codes frqs_new_codes() */
                   if (tctr>=0) { fprintf (stderr, "\n"); tctr=0; }
                   char *revised = frqs_expand_codes(char_values + tval, master_codes);
                   if (!revised) {
                       if (tctr>=0) { fprintf (stderr, "\n"); tctr=0; }
                       msg("Unable to save codes for CHAN_PARAM, %s", 3,
                           char_values + tval);
                       return(-9); // appropriate complaint will already be made.
                   }
                   memcpy (parsed_codes, revised, strlen(revised));
                   free(revised);
                   msg("Revised codes for CHAN_PARAM %s", 1, parsed_codes);
               } else if (action == SAVE_CODES) {
                   /* original processing for original action */
                   if (tctr>=0) { fprintf (stderr, "\n"); tctr=0; }
                   msg("SAVE_CODES %s (%d)", 1, char_values + tval, i);
                   memcpy (parsed_codes, char_values + tval, i);
               } else if (action == NEW_CODES) {
                    /* re-interpret parsed_codes frqs_new_codes() returns 0 normally */
                    /* cond_start so that it propages as in INSERT_V_CHAR */
                    if (tctr>=0) { fprintf (stderr, "\n"); tctr=0; }
                    msg("NEW_CODES %s (%d)", 1, char_values + tval, i);
                    memcpy (parsed_codes, char_values + tval, i);
                    if (0>frqs_new_codes(parsed_codes, master_codes, cond_start))
                        return(-10); // appropriate complaint will already be made.
               }
               need_is_save = 0;
               break;

            case GEN_CBLOCKS:           /* generate one or more c_blocks that
                                             represent the current conditions */

               i = (parsed_station == WILDCARD)? 1 : 2;
               if (append_cblocks (&cond_start, &cb_tail, i))
                  return (-1);

               cond_start -> f_group = parsed_f_group;
               cb_tail    -> f_group = parsed_f_group;

               for (i=0; i<2; i++)
                  {
                  cond_start -> scan[i] = parsed_scan[i];
                  cb_tail    -> scan[i] = parsed_scan[i];
                  }
               memcpy (cond_start -> baseline, parsed_baseline, 2);
               memcpy (cb_tail    -> baseline, parsed_baseline, 2);

               memcpy (cond_start -> source, parsed_source, 32);
               memcpy (cb_tail    -> source, parsed_source, 32);

               for (i=0; i<4; i++)
                  {
                  cond_start -> knot[i] = parsed_knot[i];
                  cb_tail    -> knot[i] = parsed_knot[i];
                  }
               if (parsed_station != WILDCARD)  /* station overrides baseline */
                  {
                  cond_start -> baseline[0] = parsed_station;
                  cond_start -> baseline[1] = WILDCARD;
                  cb_tail ->    baseline[0] = WILDCARD;
                  cb_tail ->    baseline[1] = parsed_station;
                  cond_start -> knot[0] = parsed_knot[4];
                  cb_tail    -> knot[0] = parsed_knot[4];
                  }

               if (cb_start != NULL)
                  cond_start = cb_start;    /* extended range of active c_b's */
               ntok--;             /* pop token for block interior processing */
               break;

            case NEGATE:             /* get set to negate following condition */
               negation = TRUE;
               break;

            case EOF_CLEANUP:
               break;


            case NO_OP:
            default:
               ;
            }
         ntok++;
         }

      else                 /* no appropriate state transition with this token */
         {
         parsing_error (next_state, ntok);
         return (-1);
         }
      }

   while (next_state != END_STATE);

   if (tctr>0) fprintf (stderr, "\n");
   if (msglev<-2) { msg("exit parser", -2); fflush(stderr); }
   return (0);                                          /* successful return! */
   }

/* eof */
