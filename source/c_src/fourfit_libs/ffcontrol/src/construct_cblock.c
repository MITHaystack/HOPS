#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ffcontrol.h"

//same basic fuctionality as generate_cblock but without dependence on pass/param struct
int construct_cblock (char* filename, 
                      struct c_block* cb_head,
                      struct c_block* cb_out,
                      char* baseline,//[2], 
                      char* source,//[31],
                      char fgroup,
                      int time)
{
    printf("flag0\n");
    
    printf("filename %s\n", filename);
    printf("baseline ptr = %p", baseline);
    printf("source ptr = %p", source);
    printf("baseline = %s", baseline);
    printf("source = %s", source);
    
    //init control record buffers
    char* control_file_buff = NULL;
    char* set_string_buff = NULL;
    nullify_cblock (cb_head);
    struct c_block* cb_ptr = cb_head;

    nullify_cblock( cb_out );
    default_cblock( cb_out );
    printf("flag2\n");
    parse_control_file(filename, &control_file_buff, &set_string_buff);
    cb_ptr = cb_head;

    printf("flag3\n");
    for (cb_ptr=cb_head; cb_ptr!=NULL; cb_ptr=cb_ptr->cb_chain)
    {
        if(criteria_match (cb_ptr, baseline, source, fgroup, time))
        {
            // printf("matched \n");
            // printf("cb_ptr baseline: %c%c \n", cb_ptr->baseline[0], cb_ptr->baseline[1]);
            // printf("ref_freq: %f \n", cb_ptr->ref_freq);
            copy_cblock_parts(cb_ptr,cb_out);
        }
    }
        printf("flag4\n");
    
    if(control_file_buff != NULL){free(control_file_buff);};
    if(set_string_buff != NULL){free(set_string_buff);};
    printf("flag10\n");
    return 0;
}
