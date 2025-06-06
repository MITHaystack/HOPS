/*****************************************************/
/*                                                   */
/* output() takes care of the output functions       */
/* of fourfit.  It creates the data for the fringe   */
/* plot, optionally displays it (depending on the    */
/* program control parameters), fills in the fringe  */
/* records (type 4xxx and 5000), and writes a fully  */
/* backward-compatible fringe file to disk, properly */
/* named.                                            */
/*                                                   */
/* Created October 3 1991 by CJL                     */
/*                                                   */
/*****************************************************/

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "msg.h"
#include "mk4_data.h"                   /* Definitions of data structures */
#include "mk4_dfio.h"
#include "mk4_util.h"
//#include "print_page.h"
#include "pass_struct.h"
#include "param_struct.h"
#include "meta_struct.h"
#include "vex.h"
#include "ffio.h"
#include "fileset.h"
#include "write_lock_mechanism.h"
#include "plot_data_dir.h"

int
output (struct vex* root, struct type_pass* pass)
    {
    char fringe_name[256];
    char sg;
    int i, dret;
    extern int base, test_mode, do_accounting;
    extern struct mk4_fringe fringe;
    char **fplot;
    int the_seq_no;
    static struct type_221 *t221;
    static struct type_222 *t222;
    extern int msglev;
    extern struct type_param param;
    extern struct type_status status;
    extern struct type_meta meta;
    extern struct type_plot plot;

    extern int make_plotdata(struct type_pass*);
    extern int create_fname (struct scan_struct*,
        struct type_pass*, int seq, char fname[]);
    extern int display_fplot (struct mk4_fringe*);
    extern int make_postplot (struct scan_struct*,
                       struct type_pass*,
                       char*,
                       struct type_221**);
    extern void est_pc_manual(int, char*, struct type_pass*);

    // for locking, see below and include/write_lock_mechanism.h
    int lock_retval = LOCK_PROCESS_NO_PRIORITY;
    char lockfile_name[512] = {'\0'};

                                /* Generate information to create fringe plot */
                                /* Some of this also goes into fringe file */
    if (make_plotdata (pass) != 0)
        {
        msg ("Error creating plot data", 2);
        return (1);
        }
        
    //try to get a lock on the root directory in order to write the fringe
    //this is used to signal any other possible fourfit processes in this
    //directory that we are about to create a file so we can avoid name 
    //collisions.  The lock persists from point of acqusition until the
    //eventual call to write_mk4fringe() below.
    // FIXME: should worry about stale locks if ^C is hit.
    if(!test_mode)
        {
        struct fileset fset;
        //wait until we are the next process allowed to write an output file
        lock_retval = wait_for_write_lock(root->ovex->filename,
            lockfile_name, &fset);
        //this is the last filenumber that exists on disk
        the_seq_no = fset.maxfile;
        }
    else
        {
        // in test mode, nothing should be written, so the number is moot.
        the_seq_no = -1;
        }

    /* create_fname() will put the next seq number into the fringe name */
    the_seq_no++;
                                    /* Figure out the correct, full pathname */
    if (create_fname (root->ovex, pass, the_seq_no, fringe_name) != 0)
        {
        msg ("Error figuring out proper fringe filename", 2);
        return (1);
        }
                                    /* Fill in fringe file structure */
    if (fill_fringe_info (root, pass, fringe_name) != 0)
        {
        msg ("Error filling fringe records", 2);
        return (1);
        }
        
    // provide suggestions
    if (param.est_pc_manual)
        est_pc_manual(param.est_pc_manual, root->ovex->filename, pass);

    if (make_postplot (root->ovex, pass, fringe_name, &t221) != 0)
        {
        msg ("Error creating postscript plot", 2);
        return (1);
        }
    fringe.t221 = t221;
    fringe.t221->ps_length = strlen (fringe.t221->pplot);
                                        /* Record the memory allocation */
    fringe.allocated[fringe.nalloc] = fringe.t221;
    fringe.nalloc += 1;
    
                                       /* Fill in the control file record */
                                       /* if desired */
    fringe.t222 = NULL;
    if(param.gen_cf_record)
        {
        if (fill_222 (&param, &t222) != 0)
            {
            msg ("Error filling control record", 2);
            return (1);
            }
        
        fringe.t222 = t222;
                                        /* Record the memory allocation */
        fringe.allocated[fringe.nalloc] = fringe.t222;
        fringe.nalloc += 1;
        }
                                        /* possibly dump the data */
    DUMP_PLOT_DATA2DIR(root, pass, &param, &status, &meta, &plot, &fringe);
                                        /* Actually write output fringe file */
    if( !test_mode)
        {
        if( lock_retval == LOCK_STATUS_OK)
            {
            //kludge to get fourfit to feed the generated fringe file name 
            //(but nothing else) as a return value to a
            //a python calling script (requires passing option "-m 4"); see
            //e.g. chops/source/python_src/hopstest_module/hopstestb/hopstestb.py
            //around line 74 in the FourFitThread class.
            if(msglev==4){msg ("%s",4,fringe_name);} //iff msglev=4
            if (write_mk4fringe (&fringe, fringe_name) < 0)
                {
                // pause 50ms, if a lock file was created, delete it now
                usleep(50000); remove_lockfile();
                msg ("Error writing fringe file", 2);
                return (1);
                }
            //if a lock file was created, delete it now
            usleep(50000); remove_lockfile();
            }
        else
            {
            msg ("Error getting write lock on directory.", 2);
            return (1);
            }
        }

    if (do_accounting) account ("Write output files/plots");
    dret = display_fplot (&fringe);
    if (do_accounting) account ("Wait for Godot");
    if (dret > 0) msg ("Display of fringe plot failed, continuing", 2);
    if (dret < 0) return (-1);
    return (0);
    }
