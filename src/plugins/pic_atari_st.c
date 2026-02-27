// =================================================================
#include "plugin.h"
#include "plugin_startup.c"

// =================================================================
int process(PLUGIN_COMMAND *cmd_struct)
{
    int j;
    int k;
    int l;
    unsigned char *planar_mem = NULL;
    
    cmd_struct->error = PLUGIN_NO_ERROR;
    switch(cmd_struct->command)
    {
        case PLUGIN_GET_NAME:
            cmd_struct->name = "Atari ST interleaved picture";
            break;
        
        case PICTURE_GET_CALC_MAX_SIZE:
            cmd_struct->size = (cmd_struct->bytes * cmd_struct->height * cmd_struct->bitplanes);
            break;

        case PICTURE_GEN_PICTURE:
            planar_mem = malloc(cmd_struct->bytes * cmd_struct->bitplanes + 2);
            if(planar_mem)
            {
                for(j = 0; j < cmd_struct->height; j++)
                {
                    memset(planar_mem, 0, cmd_struct->bytes * cmd_struct->bitplanes + 2);
                    for(k = 0; k < cmd_struct->bitplanes; k++)
                    {
                        for(l = 0; l < cmd_struct->bytes; l += 2)
                        {
                            planar_mem[(k * cmd_struct->bytes) + l] = cmd_struct->entry[(j * cmd_struct->bytes * cmd_struct->bitplanes) +
                                                                                        (l * cmd_struct->bitplanes) +
                                                                                        (k * 2)];
                            planar_mem[(k * cmd_struct->bytes) + (l + 1)] = cmd_struct->entry[(j * cmd_struct->bytes * cmd_struct->bitplanes) +
                                                                                              (l * cmd_struct->bitplanes) +
                                                                                              (k * 2) + 1];
                        }
                    }
                    if(fwrite(planar_mem, 1, cmd_struct->bytes * cmd_struct->bitplanes, cmd_struct->output_file) != (cmd_struct->bytes * cmd_struct->bitplanes))
                    {
                        cmd_struct->error = PLUGIN_ERROR_WRITE;
                        break;
                    }
                }
                free(planar_mem);
            }
            else
            {
                cmd_struct->error = PLUGIN_MEMORY;
            }
            planar_mem = NULL;
            break;

    }
    return 0;
}
