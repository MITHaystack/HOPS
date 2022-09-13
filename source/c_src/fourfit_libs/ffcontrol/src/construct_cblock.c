#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ffcontrol.h"

//same basic fuctionality as generate_cblock but without dependence on pass/param struct
int construct_cblock (char* filename, 
                      struct c_block* cb_out, 
                      char baseline[2], 
                      char source[31],
                      char fgroup,
                      int time)
{
    //init control record buffers
    char* control_file_buff = NULL;
    char* set_string_buff = NULL;
    struct c_block* cb_hd = NULL;
    cb_hd = (struct c_block *) malloc (sizeof (struct c_block) );
    nullify_cblock (cb_hd);
    struct c_block* cb_ptr = cb_hd;

    parse_control_file(filename, &control_file_buff, &set_string_buff);
    cb_ptr = cb_hd;

    nullify_cblock( cb_out );
    default_cblock( cb_out );

    for (cb_ptr=cb_hd; cb_ptr!=NULL; cb_ptr=cb_ptr->cb_chain)
    {
        if(criteria_match (cb_ptr, baseline, source, fgroup, time))
        {
            copy_cblock_parts(cb_ptr,cb_out);
        }
    }

    if(control_file_buff != NULL){free(control_file_buff);};
    if(set_string_buff != NULL){free(set_string_buff);};
    if(cb_hd != NULL){free(cb_hd);};

    return 0;
}
