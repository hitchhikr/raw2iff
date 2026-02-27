// =================================================================
#include "plugin.h"
#include "plugin_startup.c"

// =================================================================
int process(PLUGIN_COMMAND *cmd_struct)
{
    int color_4_bit;
    
    switch(cmd_struct->command)
    {
        case PLUGIN_GET_NAME:
            cmd_struct->name = "Red:4 Green:4 Blue:4";
            break;
        
        case PALETTE_GET_COLOR_BYTES:
            return 2;

        case PALETTE_GET_COLOR_VALUE:
            color_4_bit = (cmd_struct->entry[0] << 8) | cmd_struct->entry[1];
            cmd_struct->result.Red = (color_4_bit & 0xf00) >> 4;
            cmd_struct->result.Green = (color_4_bit & 0xf0);
            cmd_struct->result.Blue = (color_4_bit & 0xf) << 4;
            break;
    }
    return 0;
}
