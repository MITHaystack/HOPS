#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ffcontrol.h"

//same basic fuctionality as generate_cblock but without dependence on pass/param struct
int construct_cblock (char* filename, 
                      struct c_block* cb_head,
                      struct c_block* cb_out,
                      char baseline[2], 
                      char source[31],
                      char fgroup,
                      int time)
{
    //init control record buffers
    char* control_file_buff = NULL;
    char* set_string_buff = NULL;
    nullify_cblock (cb_head);
    struct c_block* cb_ptr = cb_head;

    nullify_cblock( cb_out );
    default_cblock( cb_out );

    parse_control_file(filename, &control_file_buff, &set_string_buff);
    cb_ptr = cb_head;

    for (cb_ptr=cb_head; cb_ptr!=NULL; cb_ptr=cb_ptr->cb_chain)
    {
        if(criteria_match (cb_ptr, baseline, source, fgroup, time))
        {
            printf("matched \n");
            printf("cb_ptr baseline: %c%c \n", cb_ptr->baseline[0], cb_ptr->baseline[1]);
            printf("ref_freq: %f \n", cb_ptr->ref_freq);
            copy_cblock_parts(cb_ptr,cb_out);
        }
    }
    
    printf("booo ref_freq: %f \n", cb_out->ref_freq);
    printf("addr %p \n", (void*)cb_out);
    
    printf("pcal rem 0,0 = %f ", cb_out->pc_phase[0][0].rem);

    if(control_file_buff != NULL){free(control_file_buff);};
    if(set_string_buff != NULL){free(set_string_buff);};

    return 0;
}
