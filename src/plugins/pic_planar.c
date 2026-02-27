// =================================================================
#include "plugin.h"
#include "plugin_startup.c"

// =================================================================
int process(PLUGIN_COMMAND *cmd_struct)
{
    int i;
    int j;
    
    cmd_struct->error = PLUGIN_NO_ERROR;
    switch(cmd_struct->command)
    {
        case PLUGIN_GET_NAME:
            cmd_struct->name = "Planar picture";
            break;
        
        case PICTURE_GET_CALC_MAX_SIZE:
            cmd_struct->size = (cmd_struct->bytes * cmd_struct->height * cmd_struct->bitplanes);
            break;

        case PICTURE_GEN_PICTURE:
            // write it as-is
            for(j = 0; j < cmd_struct->height; j++)
            {
                for(i = 0; i < cmd_struct->bitplanes; i++)
                {
                    if(fwrite(&cmd_struct->entry[(i * (cmd_struct->height * cmd_struct->bytes)) + (j * cmd_struct->bytes)], 1, 
                               cmd_struct->bytes, cmd_struct->output_file) != cmd_struct->bytes)
                    {
                        cmd_struct->error = PLUGIN_ERROR_WRITE;
                        return 0;
                    }
                }
            }
            break;
    }
    return 0;
}
