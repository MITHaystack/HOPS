#ifndef FSTRUCT_DONE
#define FSTRUCT_DONE

/* 180 dual-pol baselines breaks this */
#define MAXFILES 500000
/* an old restriction for filenum to be 1-9999 */
#define MAXFNDIGITS 4
#define LEGALPOLCHARS   "HVLRXY"

#define BADSTRING 0x01
#define BADFORM   0x02
#define BADBASE   0x04
#define BADFNUM   0x08
#define BADFREQ   0x10
#define BADSRC    0x20
#define BADROOT   0x40
#define BADSTAT   0x80

#ifndef FALSE
#define TRUE 1
#define FALSE 0
#endif

/* defaults to indicate a cleared structure are in () below */

/* type 0 is root file SOURCE.stamp    (2 parts)
 * type 1 is corel, AB.nn.abcdef       (3 parts)
 * type 2 is fringe, AB.X.nn.abcdef    (4 parts)
 *      2 is fringe, AB.X.nn-pp.abcdef (4 parts) new with 3.26
 * type 3 is sdata, A.nn.abcdef        (3 parts)
 * type 4 is log.<sourcename>?         (2 parts) */

typedef struct
    {
    int order;		    /* For sorting operations, (-1) */
    char *name;		    /* Full name of file (NULL) */
    short type;		    /* File type according to name 0...4 (see above; -1) */
    char source[32];	/* Source name (root files only;  0) */
    char baseline[3];	/* Baseline id (fringe and corel files only; 0) */
    char station;	    /* Type 3 (pcal) files only (' ') */
    char freq_code;	    /* Frequency id code (fringe files only) (' ') */
    int filenum;	    /* File number in fileset (-1)*/
    char rootcode[7];	/* root id code 'abcdef' (0) */
    short done;		    /* Has this file has been done (see below; FALSE) */
    int intparm[4];	    /* Auxiliary info if needed (see below; 0) */
    float floatparm[4];	/* Auxiliary info if needed (see below; 0.0) */
    /* new features */
    int nalloc;         /* nonzero if name is malloc'd (0) */
    char poln[3];       /* Polarization info ('  ') */
    } fstruct;

/* The 'done' field is used in sort_names and alist.c */

/* fringex uses intparm for fdata.srch_cotime and fdata.noloss_cotime
 *   and   uses floatparm for fdata.delay_rate and fdata.mbdelay */

/* nalloc was introduced to allow name to be freed when fset is used */
/* poln is populated if the fringe number includes the polarization */

/* sub/utils */
extern void   clear_fstruct(fstruct *);
extern int    check_name (char *, fstruct *);
extern int    extract_filenames (char *, int, fstruct **, int *, int *, int *);
extern int    get_filelist (int, char **, int, fstruct **);

#endif
