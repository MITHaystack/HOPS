/************************************************************************/
/*                                                                      */
/* This routine prints the contents of a source struct on the screen    */
/* in a nice format. It assumes summ_data() has been called             */
/*                                                                      */
/*      Inputs:         none                                            */
/*                                                                      */
/*      Output:         none                                            */
/*                                                                      */
/* Created 9 April 1989 by CJL                                          */
/*                                                                      */
/************************************************************************/

#include <stdio.h>
#include "summary.h"
#include "aedit.h"

int pr_source(srcsum *src)
    {
    int i, year, day, hour, min, sec, type, ret;
    char qclist[28], buf[150], buf2[150];

    if(src->count == 0) 
        {
        msg("No data!",2);
        return(-1);
        }

    int_to_time(src->begin,&year,&day,&hour,&min,&sec);
    printf("Time range:       %d-%03d-%02d%02d to ",year,day,hour,min);
    int_to_time(src->end,&year,&day,&hour,&min,&sec);
    printf("%d-%03d-%02d%02d\n",year,day,hour,min);
    printf("Stations present:    %s\n",src->stations);
    printf("Baselines present:   ");
    for (i=0; i<src->nbtq; i++) printf ("%s ", src->btq + 3*i);
    printf ("\n");
    printf("SNR extrema:         %#5.4g  %#5.4g\n",src->snrmin, src->snrmax);
    printf("Quality code summary:\n");
    sprintf(qclist,"ABCDEFGH0123456789?");
    buf[0] = '\0';                      /* Make nice format */
    for(i=0; i<150; i++){buf[i] = '\0'; buf2[i] = '\0';}
    for(i=0;i<19;i++) {
        sprintf(buf,"%s%c ", buf2, qclist[i]);
        if(src->qcodes[i] >= 10000) sprintf(buf2,"%s    ",buf);
        else if(src->qcodes[i] >= 1000) sprintf(buf2,"%s   ",buf);
        else if(src->qcodes[i] >= 100) sprintf(buf2,"%s  ",buf);
        else if(src->qcodes[i] >= 10) sprintf(buf2,"%s ",buf);
        else sprintf(buf2,"%s",buf);
    }
    printf("\t%s\n",buf2);
    printf("\t");
    for(i=0;i<19;i++) printf("%d ",src->qcodes[i]);
    printf("      Total %d\n", src->count);
    return(0);
    }
