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
    char name[260];
    FILE *output_file;
    int bytes;
    int width;
    int height;
    int bitplanes;
    int size;
    int color_size;
    int colors;
    int stride;
    int error;
} PLUGIN_COMMAND;

#ifndef __AMIGA__

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

// =================================================================
#define DLL_API __declspec(dllexport)
#define CALL __cdecl

#else

#include <proto/exec.h>
#include <inline/macros.h>

#define DLL_API
#define CALL 

#define GetPicStruct() \
      LP0(0x1e, PLUGIN_COMMAND *, GetPicStruct ,\
      , Raw2IffBase)

#define GetPalStruct() \
      LP0(0x24, PLUGIN_COMMAND *, GetPalStruct ,\
      , Raw2IffBase)

#endif

// =================================================================
DLL_API void CALL process(PLUGIN_COMMAND *);

#ifdef __cplusplus
}
#endif

#endif
