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
            strcpy(plugin_struct->name, "Red:3 Green:3 Blue:3");
            break;

        case PALETTE_GET_COLOR_BYTES:
            plugin_struct->color_size = 2;
            break;

        case PALETTE_GET_COLOR_VALUE:
            for(i = 0; i < plugin_struct->colors; i++)
            {
                color = (plugin_struct->entry[(i * 2)] << 8) | plugin_struct->entry[(i * 2) + 1];
                colors_block[i].Red = (unsigned char) ((((float) ((color & 0x700) >> 8)) * 255.0f) / 7.0f);
                colors_block[i].Green = (unsigned char) ((((float) ((color & 0x70) >> 4))  * 255.0f) / 7.0f);
                colors_block[i].Blue = (unsigned char) ((((float) (color & 0x7))  * 255.0f) / 7.0f);
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
