// =================================================================
#include "plugin.h"
#include "plugin_startup.c"

// =================================================================
void process(PLUGIN_COMMAND *plugin_struct)
{
    int i;
    int j;
    unsigned char *source_data;
    
    plugin_struct->error = PLUGIN_NO_ERROR;
    switch(plugin_struct->command)
    {
        case PLUGIN_GET_NAME:
            strcpy(plugin_struct->name, "Interleaved planar picture");
            break;
        
        case PICTURE_GET_CALC_MAX_SIZE:
            plugin_struct->size = ((plugin_struct->bytes + plugin_struct->stride) * plugin_struct->height) * plugin_struct->bitplanes;
            break;

        case PICTURE_GEN_PICTURE:
            source_data = plugin_struct->entry;
            for(j = 0; j < plugin_struct->height; j++)
            {
                for(i = 0; i < plugin_struct->bitplanes; i++)
                {
                    if(fwrite(source_data, 1, plugin_struct->bytes, plugin_struct->output_file) != plugin_struct->bytes)
                    {
                        plugin_struct->error = PLUGIN_ERROR_WRITE;
                        return;
                    }
                    source_data += plugin_struct->bytes + plugin_struct->stride;
                }
            }
            break;
    }
    return;
}
