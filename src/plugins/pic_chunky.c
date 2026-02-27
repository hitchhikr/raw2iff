// =================================================================
#include "plugin.h"
#include "plugin_startup.c"

// =================================================================
int process(PLUGIN_COMMAND *cmd_struct)
{
    int i;
    int j;
    unsigned char *planar_mem = NULL;
    int planar_offset;
    
    cmd_struct->error = PLUGIN_NO_ERROR;
    switch(cmd_struct->command)
    {
        case PLUGIN_GET_NAME:
            cmd_struct->name = "8 bit chunky picture";
            break;
        
        case PICTURE_GET_CALC_MAX_SIZE:
            cmd_struct->size = (cmd_struct->width * cmd_struct->height);
            break;

        case PICTURE_GEN_PICTURE:
            planar_mem = malloc(cmd_struct->bytes * cmd_struct->bitplanes);
            if(planar_mem)
            {
                for(j = 0; j < cmd_struct->height; j++)
                {
                    memset(planar_mem, 0, cmd_struct->bytes * cmd_struct->bitplanes);
                    for(i = 0; i < cmd_struct->width; i++)
                    {
                        // Convert a line to planar
                        planar_offset = cmd_struct->entry[i + (j * cmd_struct->width)];
                        if(planar_offset & 1)
                        {
                            planar_mem[(i / 8) + (0 * cmd_struct->bytes)] |= 1 << (7 - (i % 8));
                        }
                        if(planar_offset & 2)
                        {
                            if(cmd_struct->bitplanes < 2)
                            {
                                cmd_struct->error = PLUGIN_ERROR_BOUNDS;
                                goto bailout;
                            }
                            planar_mem[(i / 8) + (1 * cmd_struct->bytes)] |= 1 << (7 - (i % 8));
                        }
                        if(planar_offset & 4)
                        {
                            if(cmd_struct->bitplanes < 3)
                            {
                                cmd_struct->error = PLUGIN_ERROR_BOUNDS;
                                goto bailout;
                            }
                            planar_mem[(i / 8) + (2 * cmd_struct->bytes)] |= 1 << (7 - (i % 8));
                        }
                        if(planar_offset & 8)
                        {
                            if(cmd_struct->bitplanes < 4)
                            {
                                cmd_struct->error = PLUGIN_ERROR_BOUNDS;
                                goto bailout;
                            }
                            planar_mem[(i / 8) + (3 * cmd_struct->bytes)] |= 1 << (7 - (i % 8));
                        }
                        if(planar_offset & 16)
                        {
                            if(cmd_struct->bitplanes < 5)
                            {
                                cmd_struct->error = PLUGIN_ERROR_BOUNDS;
                                goto bailout;
                            }
                            planar_mem[(i / 8) + (4 * cmd_struct->bytes)] |= 1 << (7 - (i % 8));
                        }
                        if(planar_offset & 32)
                        {
                            if(cmd_struct->bitplanes < 6)
                            {
                                cmd_struct->error = PLUGIN_ERROR_BOUNDS;
                                goto bailout;
                            }
                            planar_mem[(i / 8) + (5 * cmd_struct->bytes)] |= 1 << (7 - (i % 8));
                        }
                        if(planar_offset & 64)
                        {
                            if(cmd_struct->bitplanes < 7)
                            {
                                cmd_struct->error = PLUGIN_ERROR_BOUNDS;
                                goto bailout;
                            }
                            planar_mem[(i / 8) + (6 * cmd_struct->bytes)] |= 1 << (7 - (i % 8));
                        }
                        if(planar_offset & 128)
                        {
                            if(cmd_struct->bitplanes < 8)
                            {
                                cmd_struct->error = PLUGIN_ERROR_BOUNDS;
                                goto bailout;
                            }
                            planar_mem[(i / 8) + (7 * cmd_struct->bytes)] |= 1 << (7 - (i % 8));
                        }
                    }
                    if(fwrite(planar_mem, 1, cmd_struct->bytes * cmd_struct->bitplanes, cmd_struct->output_file) != (cmd_struct->bytes * cmd_struct->bitplanes))
                    {
                        cmd_struct->error = PLUGIN_ERROR_WRITE;
                        break;
                    }
                }
            }
bailout:;
            if(planar_mem)
            {
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
