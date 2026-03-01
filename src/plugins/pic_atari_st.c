// =================================================================
#include "plugin.h"
#include "plugin_startup.c"

// =================================================================
void process(PLUGIN_COMMAND *plugin_struct)
{
    int j;
    int k;
    int l;
    unsigned char *planar_mem = NULL;
    unsigned char *source_data;
    
    plugin_struct->error = PLUGIN_NO_ERROR;
    switch(plugin_struct->command)
    {
        case PLUGIN_GET_NAME:
            strcpy(plugin_struct->name, "Atari ST interleaved picture");
            break;

        case PICTURE_GET_CALC_MAX_SIZE:
            plugin_struct->size = ((plugin_struct->bytes * plugin_struct->bitplanes) + plugin_struct->stride) * plugin_struct->height;
            break;

        case PICTURE_GEN_PICTURE:
            source_data = plugin_struct->entry;
            planar_mem = malloc(plugin_struct->bytes * plugin_struct->bitplanes + 2);
            if(planar_mem)
            {
                for(j = 0; j < plugin_struct->height; j++)
                {
                    memset(planar_mem, 0, plugin_struct->bytes * plugin_struct->bitplanes + 2);
                    for(k = 0; k < plugin_struct->bitplanes; k++)
                    {
                        for(l = 0; l < plugin_struct->bytes; l += 2)
                        {
                            planar_mem[(k * plugin_struct->bytes) + l] = source_data[(l * plugin_struct->bitplanes) + (k * 2)];
                            planar_mem[(k * plugin_struct->bytes) + (l + 1)] = source_data[(l * plugin_struct->bitplanes) + (k * 2) + 1];
                        }
                    }
                    if(fwrite(planar_mem, 1, plugin_struct->bytes * plugin_struct->bitplanes, plugin_struct->output_file) != (plugin_struct->bytes * plugin_struct->bitplanes))
                    {
                        plugin_struct->error = PLUGIN_ERROR_WRITE;
                        break;
                    }
                    source_data += plugin_struct->stride + (plugin_struct->bytes * plugin_struct->bitplanes);
                }
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
