// =================================================================
#include "plugin.h"
#define __PALETTE_PLUGIN__
#include "plugin_startup.c"

// =================================================================
COLORMAPENTRY colors_block[256];

// =================================================================
void process(PLUGIN_COMMAND *plugin_struct)
{
    int i;

    plugin_struct->error = PLUGIN_NO_ERROR;
    switch(plugin_struct->command)
    {
        case PLUGIN_GET_NAME:
            strcpy(plugin_struct->name, "Alpha:8 Red:8 Green:8 Blue:8");
            break;

        case PALETTE_GET_COLOR_BYTES:
            plugin_struct->color_size = 4;
            break;

        case PALETTE_GET_COLOR_VALUE:
            for(i = 0; i < plugin_struct->colors; i++)
            {
                // Skip the alpha channel
                colors_block[i].Red = plugin_struct->entry[(i * 4) + 1];
                colors_block[i].Green = plugin_struct->entry[(i * 4) + 2];
                colors_block[i].Blue = plugin_struct->entry[(i * 4) + 3];
            }
            if(fwrite(colors_block, 1, sizeof(COLORMAPENTRY) * plugin_struct->colors, plugin_struct->output_file) != 
                     (sizeof(COLORMAPENTRY) * plugin_struct->colors))
            {
                plugin_struct->error = PLUGIN_ERROR_WRITE;
            }
            break;
    }
    return;
}
