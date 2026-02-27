// =================================================================
#include "plugin.h"
#include "plugin_startup.c"

// =================================================================
int process(PLUGIN_COMMAND *cmd_struct)
{
    int size_to_write;
    
    cmd_struct->error = PLUGIN_NO_ERROR;
    switch(cmd_struct->command)
    {
        case PLUGIN_GET_NAME:
            cmd_struct->name = "Interleaved planar picture";
            break;
        
        case PICTURE_GET_CALC_MAX_SIZE:
            cmd_struct->size = (cmd_struct->bytes * cmd_struct->height * cmd_struct->bitplanes);
            break;

        case PICTURE_GEN_PICTURE:
            size_to_write = (cmd_struct->bytes * cmd_struct->height * cmd_struct->bitplanes);
            // write it as-is
            if(fwrite(cmd_struct->entry, 1, size_to_write, cmd_struct->output_file) != size_to_write)
            {
                cmd_struct->error = PLUGIN_ERROR_WRITE;
            }
            break;

    }
    return 0;
}
