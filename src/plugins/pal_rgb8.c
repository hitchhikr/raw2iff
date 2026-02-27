// =================================================================
#include "plugin.h"
#include "plugin_startup.c"

// =================================================================
int process(PLUGIN_COMMAND *cmd_struct)
{
    switch(cmd_struct->command)
    {
        case PLUGIN_GET_NAME:
            cmd_struct->name = "Red:8 Green:8 Blue:8";
            break;

        case PALETTE_GET_COLOR_BYTES:
            return 3;

        case PALETTE_GET_COLOR_VALUE:
            cmd_struct->result.Red = cmd_struct->entry[0];
            cmd_struct->result.Green = cmd_struct->entry[1];
            cmd_struct->result.Blue = cmd_struct->entry[2];
            break;
    }
    return 0;
}
