// =================================================================
#include "plugin.h"
#include "plugin_startup.c"

// =================================================================
void process(PLUGIN_COMMAND *plugin_struct)
{
    int i;
    int j;
    unsigned char *planar_mem = NULL;
    unsigned char *source_data;
    int planar_offset;
    
    plugin_struct->error = PLUGIN_NO_ERROR;
    switch(plugin_struct->command)
    {
        case PLUGIN_GET_NAME:
            strcpy(plugin_struct->name, "8 bit chunky picture");
            break;
        
        case PICTURE_GET_CALC_MAX_SIZE:
            plugin_struct->size = ((plugin_struct->width + plugin_struct->stride) * plugin_struct->height);
            break;

        case PICTURE_GEN_PICTURE:
            source_data = plugin_struct->entry;
            planar_mem = malloc(plugin_struct->bytes * plugin_struct->bitplanes);
            if(planar_mem)
            {
                for(j = 0; j < plugin_struct->height; j++)
                {
                    memset(planar_mem, 0, plugin_struct->bytes * plugin_struct->bitplanes);
                    for(i = 0; i < plugin_struct->width; i++)
                    {
                        // Convert a line to planar
                        planar_offset = source_data[i];
                        if(planar_offset & 1)
                        {
                            planar_mem[(i / 8) + (0 * plugin_struct->bytes)] |= 1 << (7 - (i % 8));
                        }
                        if(planar_offset & 2)
                        {
                            if(plugin_struct->bitplanes < 2)
                            {
                                plugin_struct->error = PLUGIN_ERROR_BOUNDS;
                                goto bailout;
                            }
                            planar_mem[(i / 8) + (1 * plugin_struct->bytes)] |= 1 << (7 - (i % 8));
                        }
                        if(planar_offset & 4)
                        {
                            if(plugin_struct->bitplanes < 3)
                            {
                                plugin_struct->error = PLUGIN_ERROR_BOUNDS;
                                goto bailout;
                            }
                            planar_mem[(i / 8) + (2 * plugin_struct->bytes)] |= 1 << (7 - (i % 8));
                        }
                        if(planar_offset & 8)
                        {
                            if(plugin_struct->bitplanes < 4)
                            {
                                plugin_struct->error = PLUGIN_ERROR_BOUNDS;
                                goto bailout;
                            }
                            planar_mem[(i / 8) + (3 * plugin_struct->bytes)] |= 1 << (7 - (i % 8));
                        }
                        if(planar_offset & 16)
                        {
                            if(plugin_struct->bitplanes < 5)
                            {
                                plugin_struct->error = PLUGIN_ERROR_BOUNDS;
                                goto bailout;
                            }
                            planar_mem[(i / 8) + (4 * plugin_struct->bytes)] |= 1 << (7 - (i % 8));
                        }
                        if(planar_offset & 32)
                        {
                            if(plugin_struct->bitplanes < 6)
                            {
                                plugin_struct->error = PLUGIN_ERROR_BOUNDS;
                                goto bailout;
                            }
                            planar_mem[(i / 8) + (5 * plugin_struct->bytes)] |= 1 << (7 - (i % 8));
                        }
                        if(planar_offset & 64)
                        {
                            if(plugin_struct->bitplanes < 7)
                            {
                                plugin_struct->error = PLUGIN_ERROR_BOUNDS;
                                goto bailout;
                            }
                            planar_mem[(i / 8) + (6 * plugin_struct->bytes)] |= 1 << (7 - (i % 8));
                        }
                        if(planar_offset & 128)
                        {
                            if(plugin_struct->bitplanes < 8)
                            {
                                plugin_struct->error = PLUGIN_ERROR_BOUNDS;
                                goto bailout;
                            }
                            planar_mem[(i / 8) + (7 * plugin_struct->bytes)] |= 1 << (7 - (i % 8));
                        }
                    }
                    if(fwrite(planar_mem, 1, plugin_struct->bytes * plugin_struct->bitplanes, plugin_struct->output_file) != (plugin_struct->bytes * plugin_struct->bitplanes))
                    {
                        plugin_struct->error = PLUGIN_ERROR_WRITE;
                        break;
                    }
                    source_data += plugin_struct->width + plugin_struct->stride;
                }
            }
bailout:;
            if(planar_mem)
            {
                free(planar_mem);
            }
            else
            {
                plugin_struct->error = PLUGIN_MEMORY;
            }
            planar_mem = NULL;
            break;

    }
    return;
}
