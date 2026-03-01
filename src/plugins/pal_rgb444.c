// =================================================================
#include "plugin.h"
#define __PALETTE_PLUGIN__
#include "plugin_startup.c"

// =================================================================
COLORMAPENTRY colors_block[256];

// =================================================================
void process(PLUGIN_COMMAND *plugin_struct)
{
    int color;
    int i;
    
    plugin_struct->error = PLUGIN_NO_ERROR;    
    switch(plugin_struct->command)
    {
        case PLUGIN_GET_NAME:
            strcpy(plugin_struct->name, "Red:4 Green:4 Blue:4");
            break;
        
        case PALETTE_GET_COLOR_BYTES:
            plugin_struct->color_size = 2;
            break;

        case PALETTE_GET_COLOR_VALUE:
            for(i = 0; i < plugin_struct->colors; i++)
            {
                color = (plugin_struct->entry[(i * 2)] << 8) | plugin_struct->entry[(i * 2) + 1];
                colors_block[i].Red = (color & 0xf00) >> 4;
                colors_block[i].Green = (color & 0xf0);
                colors_block[i].Blue = (color & 0xf) << 4;
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
