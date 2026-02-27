// =================================================================
#include "plugin.h"
#include "plugin_startup.c"

// =================================================================
int process(PLUGIN_COMMAND *cmd_struct)
{
    int color;
    
    switch(cmd_struct->command)
    {
        case PLUGIN_GET_NAME:
            cmd_struct->name = "Red:3 Green:3 Blue:3";
            break;
        
        case PALETTE_GET_COLOR_BYTES:
            return 2;

        case PALETTE_GET_COLOR_VALUE:
            color = (cmd_struct->entry[0] << 8) | cmd_struct->entry[1];
            cmd_struct->result.Red = (unsigned char) ((((float) ((color & 0x700) >> 8)) * 255.0f) / 7.0f);
            cmd_struct->result.Green = (unsigned char) ((((float) ((color & 0x70) >> 4))  * 255.0f) / 7.0f);
            cmd_struct->result.Blue = (unsigned char) ((((float) (color & 0x7))  * 255.0f) / 7.0f);
            break;
    }
    return 0;
}
