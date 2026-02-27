// =================================================================
#ifndef _PLUGIN_H_
#define _PLUGIN_H_

// =================================================================

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

#ifdef __WINDOWS__

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

// =================================================================
#define DLL_API __declspec(dllexport)
#define CALL __cdecl

#else
    
#define DLL_API
#define CALL 

#endif

// =================================================================
#define PLUGIN_GET_NAME 0
#define PALETTE_GET_COLOR_BYTES 1
#define PALETTE_GET_COLOR_VALUE 2
#define PICTURE_GET_CALC_MAX_SIZE 3
#define PICTURE_GEN_PICTURE 4

#define PLUGIN_NO_ERROR 0
#define PLUGIN_ERROR_WRITE 1
#define PLUGIN_ERROR_BOUNDS 2
#define PLUGIN_MEMORY 3

typedef struct
{
	unsigned char Red;          /* Red color component (0-255) */
	unsigned char Green;        /* Green color component (0-255) */
	unsigned char Blue;         /* Blue color component (0-255) */
} COLORMAPENTRY;

typedef struct
{
    int command;
    unsigned char *entry;
    COLORMAPENTRY result;
    char *name;
    FILE *output_file;
    int bytes;
    int width;
    int height;
    int bitplanes;
    int size;
    int color_index;
    int error;
} PLUGIN_COMMAND;

// =================================================================
DLL_API int CALL process(PLUGIN_COMMAND *cmd_struct);

#ifdef __cplusplus
}
#endif

#endif
