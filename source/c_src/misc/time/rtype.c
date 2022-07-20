/*
 * $Id: rtype.c 3802 2008-12-21 01:12:04Z gbc $
 *
 * utility for letters to types
 */

#include <stdio.h>
#include "htime.h"

/*
 * Report the various format options
 * Note the default ctime format changed
 * from this:  Sat Dec 14 21:36:08 UTC 2019
 * to this:    Sat 14 Dec 2019 09:29:00 PM UTC
 * with releases sometime in 2019 or so.
 * This can be traced to the locale date_fmt which changed
 * from '%a %b %e %H:%M:%S %Z %Y' to '%a %d %b %Y %r %Z'
 */
void Ht_format_help(void)
{
    fprintf(stderr,
	"Legal time format options are\n"
	"* A            (unused)\n"
	"  B  Burst     express as Gamma Ray Burst time (YYMMDD+Sod)\n"
	"* C  SPICE     express as SPICE ephemeris time (J2000 epoch)\n"
	"  D  Date      express as traditional date (ctime)\n"
	"     DateA     express as alternate locale date (ctime)\n"
	"  E  Maestro__ express as MAESTRO YYYY/MM:DD:HH:MM:SS\n"
	"     Maestro:: express as MAESTRO YYYY:DOY:HH:MM:SS\n"
	"     Maestro.. express as MAESTRO YYYY.DOY.HH.MM.SS\n"
	"  F  File__    express as Filename Time (YYYY_DOY_HHMM)\n"
	"     File___   express as Filename Time (YYYY_DOY_HH_MM)\n"
	"  G  GPS       express as seconds of GPS epoch\n\n"
	"  H  HOPS      express as hops rootcode ([a-z]x[67])\n\n"
	"  I  ISOusz    express as ISO Time (with us)\n"
	"     ISOmsz    express as ISO Time (with ms)\n"
	"     ISOsz     express as ISO Time (no us/ms)\n"
	"     ISOusnz   express as ISO Time (with us, no Z)\n"
	"     ISOmsnz   express as ISO Time (with ms, no Z)\n"
	"     ISOsnz    express as ISO Time (no us/ms, no Z)\n"
	"  J  Julian    express as Julian Day\n"
	"  K  Clock     express as UNIX clock seconds\n"
	"* L            (unused)\n"
	"  M  MJD       express as Modified Julian Day\n"
	"  N  Plan      express as Planning Time (YYYY/DOY.HHMMSS)\n"
	"  P  PIGMI     express as PIGMI Time (YYYYMMDDHHMMSS)\n"
        "     Plog      express as Python Log (YYYY-MM-DD HH:MM:SS,mmm)\n"
	"  Q  YDF       express as YYYY/DOY.fracday\n"
	"  R            express as relative time\n"
	"  S  MET       express as S/C (MET) time\n"
	"* T            (unused -- Truncated JD)\n"
	"  U  UNIX      express as UNIX time\n"
	"  V  SVNnz     express as SVN Date (no Z)\n"
	"     SVNz      express as SVN Date (with Z)\n"
	"     SVNusnz   express as SVN Date (no Z, with us)\n"
	"     SVNusz    express as SVN Date (with Z, with us)\n"
	"  W  Weeks     express as GPS Weeks\n"
	"  X  Vex       express as VLBI experiment time\n"
	"     Vdif      express as VLBI data interchange format time\n"
        "     Vrdbe     express as RDBE version of Vex time\n"
	"  Y  YDS       express as Year/DoY/SoD\n"
	"* Z            (unused)\n"
	"* == planned or reserved, but not yet implemented.\n"
	"\n"
    );
}

/* returns Ht_DS_* as appropriate */
int Ht_rtype_to_style(char *rtype)
{
    /* single letter cases */
    if (!rtype[1]) switch (*rtype) {
    /*     C                                               */
    /* A                     L     O         T           Z */
    /*   B   D E F G H I J K   M N   P Q R S   U V W X Y   */
    case 'K':   return(Ht_DS_UNIX); /*  0 */
                /* Ht_DS_PASS */    /*  1 */
    case 'D':   return(Ht_DS_DATE); /*  2 */
    case 'G':   return(Ht_DS_SECS); /*  3 */
    case 'U':   return(Ht_DS_Y2KP); /*  4 */
    case 'S':   return(Ht_DS_HT00); /*  5 */
    case 'Q':   return(Ht_DS_YDOY); /*  6 */
    case 'B':   return(Ht_DS_GRBS); /*  7 */
                /* Ht_DS_HT03 */    /*  8 */
    case 'J':   return(Ht_DS_JULD); /*  9 */
    case 'M':   return(Ht_DS_MJUL); /* 10 */
                /* Ht_DS_Y2DS */    /* 11 */
    case 'Y':   return(Ht_DS_Y4DS); /* 12 */
    case 'W':   return(Ht_DS_UWKS); /* 13 */
                /* Ht_DS_USVN */    /* 14 */
    case 'V':   return(Ht_DS_USVZ); /* 15 */
                /* Ht_DS_ISON */    /* 16 */
    case 'I':   return(Ht_DS_ISOT); /* 17 */
    case 'P':   return(Ht_DS_PGMI); /* 18 */
                /* Ht_DS_ISOM */    /* 19 */
    case 'E':   return(Ht_DS_MAES); /* 20 */
    case 'N':   return(Ht_DS_PLAN); /* 21 */
    case 'F':   return(Ht_DS_FILE); /* 22 */
                /* Ht_DS_MY4D */    /* 23 */
                /* Ht_DS_MY4C */    /* 24 */
                /* Ht_DS_FILH */    /* 25 */
    case 'H':   return(Ht_DS_HOPS); /* 26 */
    case 'R':   return(Ht_DS_UTRL); /* 27 */
    case 'X':	return(Ht_DS_VEXT); /* 28 */
		/* Ht_DS_VDIF */    /* 29 */

    default:    fprintf(stderr, "%c is not defined\n", *rtype);
                return(Ht_DS_NUMB); /* 30 */
    }
    else if (!strcmp(rtype, "Burst"))     return(Ht_DS_GRBS);
    else if (!strcmp(rtype, "Date"))      return(Ht_DS_DATE);
    else if (!strcmp(rtype, "DateA"))     return(Ht_DS_ADAT);
    else if (!strcmp(rtype, "Maestro__")) return(Ht_DS_MAES);
    else if (!strcmp(rtype, "Maestro::")) return(Ht_DS_MY4C);
    else if (!strcmp(rtype, "Maestro..")) return(Ht_DS_MY4D);
    else if (!strcmp(rtype, "GPS"))       return(Ht_DS_SECS);
    else if (!strcmp(rtype, "HOPS"))      return(Ht_DS_HOPS);
    else if (!strcmp(rtype, "ISOusz"))    return(Ht_suppress_Z=0,Ht_DS_ISOT);
    else if (!strcmp(rtype, "ISOmsz"))    return(Ht_suppress_Z=0,Ht_DS_ISOM);
    else if (!strcmp(rtype, "ISOsz"))     return(Ht_suppress_Z=0,Ht_DS_ISON);
    else if (!strcmp(rtype, "ISOusnz"))   return(Ht_suppress_Z=1,Ht_DS_ISOT);
    else if (!strcmp(rtype, "ISOmsnz"))   return(Ht_suppress_Z=1,Ht_DS_ISOM);
    else if (!strcmp(rtype, "ISOsnz"))    return(Ht_suppress_Z=1,Ht_DS_ISON);
    else if (!strcmp(rtype, "ISO"))       return(Ht_suppress_Z=1,Ht_DS_ISON);
    else if (!strcmp(rtype, "Julian"))    return(Ht_DS_JULD);
    else if (!strcmp(rtype, "Clock"))     return(Ht_DS_UNIX);
    else if (!strcmp(rtype, "File___"))   return(Ht_DS_FILH);
    else if (!strcmp(rtype, "File__"))    return(Ht_DS_FILE);
    else if (!strcmp(rtype, "MJD"))       return(Ht_DS_MJUL);
    else if (!strcmp(rtype, "Plan"))      return(Ht_DS_PLAN);
    else if (!strcmp(rtype, "PIGMI"))     return(Ht_DS_PGMI);
    else if (!strcmp(rtype, "Plog"))      return(Ht_DS_PLOG);
    else if (!strcmp(rtype, "YDF"))       return(Ht_DS_YDOY);
    else if (!strcmp(rtype, "MET"))       return(Ht_DS_HT00);
    else if (!strcmp(rtype, "UNIX"))      return(Ht_DS_Y2KP);
    else if (!strcmp(rtype, "SVNz"))      return(Ht_suppress_Z=0,Ht_DS_USVZ);
    else if (!strcmp(rtype, "SVNnz"))     return(Ht_suppress_Z=1,Ht_DS_USVZ);
    else if (!strcmp(rtype, "SVNus"))     return(Ht_suppress_Z=0,Ht_DS_USVN);
    else if (!strcmp(rtype, "SVNusnz"))   return(Ht_suppress_Z=1,Ht_DS_USVN);
    else if (!strcmp(rtype, "Vex"))       return(Ht_DS_VEXT);
    else if (!strcmp(rtype, "Vdif"))      return(Ht_DS_VDIF);
    else if (!strcmp(rtype, "Vrdbe"))     return(Ht_DS_RDBE);
    else if (!strcmp(rtype, "Weeks"))     return(Ht_DS_UWKS);
    else if (!strcmp(rtype, "YDS"))       return(Ht_DS_Y4DS);
    else fprintf(stderr, "%s is not defined\n", *rtype);
    return(Ht_DS_NUMB); /* can't happen */
}

/* returns Ht_STR_* as appropriate */
int Ht_qtype_to_style(char *qtype)
{
    if (!qtype[1]) switch(*qtype) {
				     /* Ht_STR_NTYP   0x000 // unknown typ */
				     /* Ht_STR_UTYP   0x100 // a UT string */
    case 'K':	return(Ht_STR_UTNX); /* Ht_STR_UTNX   0x101 // a UNIX time */
    case 'U':	return(Ht_STR_UTPS); /* Ht_STR_UTPS   0x102 // a PASS date */
    case 'V':	return(Ht_STR_USVN); /* Ht_STR_USVN   0x103 // a SVN  date */
    case 'D':   return(Ht_STR_UTDT); /* Ht_STR_UTDT   0x104 // a DATE date */
    case 'I':	return(Ht_STR_ISOT); /* Ht_STR_ISOT   0x105 // an ISO time */
				     /* Ht_STR_ISON   0x106 // an ISO (s)  */
				     /* Ht_STR_ISOM   0x107 // an ISO (ms) */
    case 'R':	return(Ht_STR_UTRL); /* Ht_STR_UTRL   0x108 // relative UT */
    case 'H':	return(Ht_STR_HOPS); /* Ht_STR_HOPS   0x109 // UNIX base26 */
				     /* Ht_STR_PLOG   0x10A // Python  Log */
				     /* Ht_STR_ADAT   0x10B // Python  Log */
    case 'G':	return(Ht_STR_UGPS); /* Ht_STR_UGPS   0x110 // GPS in secs */
    case 'W':	return(Ht_STR_UWKS); /* Ht_STR_UWKS   0x120 // GPS wks+sec */
    case 'E':	return(Ht_STR_MAES); /* Ht_STR_MAES   0x139 // YYYY/MM:DD: */
				     /* Ht_STR_MY4D   0x13A // M. YYYY.HH. */
				     /* Ht_STR_MY2D   0x13B // Mae. YY.HH. */
				     /* Ht_STR_MY0D   0x13C // Maestr. HH. */
				     /* Ht_STR_MY4C   0x13D // M. YYYY:HH: */
				     /* Ht_STR_MY2C   0x13E // Mae. YY:HH: */
				     /* Ht_STR_MY0C   0x13F // Maestr. HH: */
    case 'B':	return(Ht_STR_UGRB); /* Ht_STR_UGRB   0x140 // GRB secs UT */
    case 'J':	return(Ht_STR_JDAY); /* Ht_STR_JDAY   0x160 // Julian Date */
    case 'M':	return(Ht_STR_MODJ); /* Ht_STR_MODJ   0x161 // Mod JulianD */
    case 'N':	return(Ht_STR_PLAN); /* Ht_STR_PLAN   0x170 // yyyydoy.hms */
    case 'F':	return(Ht_STR_FILE); /* Ht_STR_FILE   0x171 // yyyy_doy_hm */
				     /* Ht_STR_FILH   0x172 //  _doy_hh_mm */
				     /* Ht_STR_Y2DS   0x180 //  yy/doy/sod */
    case 'Y':	return(Ht_STR_Y4DS); /* Ht_STR_Y4DS   0x181 //yyyy/doy/sod */
    case 'Q':	return(Ht_STR_YDOY); /* Ht_STR_YDOY   0x182 //yyyy/doy.frd */
    case 'P':	return(Ht_STR_PGMI); /* Ht_STR_PGMI   0x190 // pygmi  time */
    case 'X':	return(Ht_STR_VEXT); /* Ht_STR_VEXT   0x1A0 // vlbi  (vex) */
				     /* Ht_STR_VDIF   0x1A1 // vlbi (vdif) */
				     /* Ht_STR_VRDBE  0x1A2 // vlbi (rdbe) */
				     /* Ht_STR_HTYP   0x200 // a HT string */
				     /* Ht_STR_HT0x   0x202 // sc_clock 0x */
				     /* Ht_STR_HT0X   0x204 // sc_clock 0X */
    case 'S':	return(Ht_STR_HT00); /* Ht_STR_HT00   0x208 // without 0xX */
				     /* Ht_STR_HT03   0x210 // 3wds no 0xX */
    }
    else if (!strcmp(qtype, "Burst"))     return(Ht_STR_UGRB);
    else if (!strcmp(qtype, "Date"))      return(Ht_STR_UTDT);
    else if (!strcmp(qtype, "DateA"))     return(Ht_STR_ADAT);
    else if (!strcmp(qtype, "Maestro__")) return(Ht_STR_MAES);
    else if (!strcmp(qtype, "Maestro::")) return(Ht_STR_MY4C);
    else if (!strcmp(qtype, "Maestro..")) return(Ht_STR_MY4D);
    else if (!strcmp(qtype, "GPS"))       return(Ht_STR_UGPS);
    else if (!strcmp(qtype, "HOPS"))      return(Ht_STR_HOPS);
    else if (!strcmp(qtype, "ISOusz"))    return(Ht_STR_ISOT);
    else if (!strcmp(qtype, "ISOmsz"))    return(Ht_STR_ISOM);
    else if (!strcmp(qtype, "ISOsz"))     return(Ht_STR_ISON);
    else if (!strcmp(qtype, "ISOusnz"))   return(Ht_STR_ISOT);
    else if (!strcmp(qtype, "ISOmsnz"))   return(Ht_STR_ISOM);
    else if (!strcmp(qtype, "ISOsnz"))    return(Ht_STR_ISON);
    else if (!strcmp(qtype, "Julian"))    return(Ht_STR_JDAY);
    else if (!strcmp(qtype, "Clock"))     return(Ht_STR_UTNX);
    else if (!strcmp(qtype, "File___"))   return(Ht_STR_FILH);
    else if (!strcmp(qtype, "File__"))    return(Ht_STR_FILE);
    else if (!strcmp(qtype, "MJD"))       return(Ht_STR_MODJ);
    else if (!strcmp(qtype, "Plan"))      return(Ht_STR_PLAN);
    else if (!strcmp(qtype, "PIGMI"))     return(Ht_STR_PGMI);
    else if (!strcmp(qtype, "Plog"))      return(Ht_STR_PLOG);
    else if (!strcmp(qtype, "YDF"))       return(Ht_STR_YDOY);
    else if (!strcmp(qtype, "MET"))       return(Ht_STR_HT00);
    else if (!strcmp(qtype, "UNIX"))      return(Ht_STR_UTPS);
    else if (!strcmp(qtype, "Vex"))       return(Ht_STR_VEXT);
    else if (!strcmp(qtype, "Vdif"))      return(Ht_STR_VDIF);
    else if (!strcmp(qtype, "Vrdbe"))     return(Ht_STR_RDBE);
    else if (!strcmp(qtype, "SVNz"))      return(Ht_STR_USVN);
    else if (!strcmp(qtype, "SVNnz"))     return(Ht_STR_USVN);
    else if (!strcmp(qtype, "SVNus"))     return(Ht_STR_USVN);
    else if (!strcmp(qtype, "SVNusnz"))   return(Ht_STR_USVN);
    else if (!strcmp(qtype, "Weeks"))     return(Ht_STR_UWKS);
    else if (!strcmp(qtype, "YDS"))       return(Ht_STR_Y4DS);
    else fprintf(stderr, "%s is not defined\n", *qtype);
    return(Ht_STR_NULL);
}

/*
 * eof
 */
