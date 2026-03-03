// =================================================================
// raw2iff v2.2
// Written by Franck 'hitchhikr' Charlet.
// =================================================================

// =================================================================
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <strings.h>

#ifndef __AMIGA__

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "plugins/plugin.h"

typedef int (__cdecl *MYPROC)(PLUGIN_COMMAND *); 

#define SEPARATOR "\\"
#define PLUGIN_FILENAME "%s.dll"
#define UNLOAD_PLUGIN(x) if(x) FreeLibrary(x); x = 0;
#define GET_PLUGIN_FUNC(x, y) y = (MYPROC) GetProcAddress(x, "process"); if(!y) { \
                              fprintf(stderr, "\nError: can't obtain plugin entry point."); \
                              exit(EXIT_FAILURE); }
#define LOAD_PLUGIN(w, x, y, z) x = LoadLibrary(w); if(!x) { fprintf(stderr, z); exit(EXIT_FAILURE); } \
                                GET_PLUGIN_FUNC(x, y)
#define PLUGIN_FUNC(x, y) x(y);

#else

#include <dos/dos.h>
#include <exec/libraries.h>
#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/alib.h>

#include "plugins/plugin.h"

typedef int (*MYPROC)(int __asm("d0"), PLUGIN_COMMAND * __asm("a0"));

#define SEPARATOR "/"
#define PLUGIN_FILENAME "%s.am"
#define UNLOAD_PLUGIN(x) if(x) UnLoadSeg(x); x = 0;
#define GET_PLUGIN_FUNC(x, y) y = (MYPROC) ((x << 2) + 4);
#define LOAD_PLUGIN(w, x, y, z) x = LoadSeg(w); if(!x) { fprintf(stderr, z); exit(EXIT_FAILURE); } \
                                GET_PLUGIN_FUNC(x, y)
#define PLUGIN_FUNC(x, y) asm("movem.l d0-a6,-(a7)"); x(0, 0); asm("movem.l (a7)+,d0-a6");

#endif

// =================================================================
typedef struct
{
	char ChunkId[4];            /* Chunk Identifier "BMHD" */
	unsigned int Size;          /* Size of chunk data in bytes */
	/* Chunk data starts here */
	unsigned short Width;       /* Width of image in pixels */
	unsigned short Height;      /* Height of image in pixels */
	unsigned short Left;        /* X coordinate of image */
	unsigned short Top;         /* Y coordinate of image */
	unsigned char Bitplanes;    /* Number of bitplanes */
	unsigned char Masking;      /* Type of masking used */
	unsigned char Compress;     /* Compression method use on image data */
	unsigned char Padding;      /* Alignment padding (always 0) */
	unsigned short Transparency;/* Transparent background color */
	unsigned char XAspectRatio; /* Horizontal pixel size */
	unsigned char YAspectRatio; /* Vertical pixel size */
	unsigned short PageWidth;   /* Horizontal resolution of display device */
	unsigned short PageHeight;  /* Vertical resolution of display device */
} BMHD;

typedef struct
{
	char  ChunkId[4];           /* Chunk Identifier "CMAP" */
	unsigned int Size;          /* Size of chunk data in bytes */
} CMAP;

typedef struct
{
	char  ChunkId[4];           /* Chunk Identifier "CAMG" */
	unsigned int Size;          /* Size of chunk data in bytes */
	unsigned int Data;          /* Amiga specific flags */
} CAMG;

typedef struct
{
	char  ChunkId[4];           /* Chunk Identifier "BODY" */
	unsigned int Size;          /* Size of chunk data in bytes */
} BODY;

typedef struct 
{
    char ChunkId[4];            /* "FORM" */
    unsigned int Size;          /* FORM size (size of file minus 8) */
    /* Start of FORM chunk's data */
    char TypeID[4];             /* "ILBM" */
    BMHD Bmhd;                  /* Bitmap header data */
    CMAP Cmap;                  /* Color map data */
} ILBM;

typedef struct
{
    int colors;
    int bitplanes;
} BITPLANES;

// =================================================================
MYPROC pal_process_address;
MYPROC pic_process_address;
PLUGIN_COMMAND pal_plugin_struct;
PLUGIN_COMMAND pic_plugin_struct;

#ifndef __AMIGA__

HMODULE hpal_plugin = NULL;
HMODULE hpic_plugin = NULL;

#else

BPTR hpal_plugin = 0;
BPTR hpic_plugin = 0;

struct LibraryHeader
{
    struct Library lh_Library;
    UWORD lh_Pad1;
    BPTR lh_Segment;
};

struct LibraryHeader *Raw2IffBase;

BPTR LibExpunge(struct LibraryHeader *base __asm("a6"))
{
    BPTR rc;

    if(base->lh_Library.lib_OpenCnt > 0)
    {
        base->lh_Library.lib_Flags | LIBF_DELEXP;
        return 0;
    }
    Remove((struct Node *) base);
    rc = base->lh_Segment;
    FreeMem(((unsigned char *) base) - base->lh_Library.lib_NegSize, 
            (ULONG) (base->lh_Library.lib_NegSize + base->lh_Library.lib_PosSize)
           );
    return rc;
}

struct LibraryHeader *LibOpen(struct LibraryHeader *base __asm("a6"))
{
    base->lh_Library.lib_Flags & ~LIBF_DELEXP;
    base->lh_Library.lib_OpenCnt++;
    return base;
}

struct LibraryHeader *LibClose(struct LibraryHeader *base __asm("a6"))
{
    base->lh_Library.lib_OpenCnt--;
    if(base->lh_Library.lib_OpenCnt == 0 && base->lh_Library.lib_Flags & LIBF_DELEXP)
    {
        LibExpunge(base);
    }
    return NULL;
}

PLUGIN_COMMAND *GetPicStructLib()
{
    return &pic_plugin_struct;
}

PLUGIN_COMMAND *GetPalStructLib()
{
    return &pal_plugin_struct;
}

APTR lib_functions[] =
{
    (APTR) LibOpen,
    (APTR) LibClose,
    (APTR) LibExpunge,
    (APTR) NULL,
    (APTR) GetPicStructLib,
    (APTR) GetPalStructLib,
    (APTR) -1
};

#endif

unsigned char *src_mem = NULL;
unsigned char *external_mem = NULL;
FILE *output_file = NULL;
FILE *pal_index_file = NULL;
FILE *pic_index_file = NULL;
char pal_plugins_filenames[256][360];
char pal_plugins_filename[360];
char pic_plugins_filenames[256][360];
char pic_plugins_filename[360];

// =================================================================
#ifdef __BIG_ENDIAN__

unsigned int swap_dword(unsigned int value)
{
    return value;
}

unsigned short swap_word(unsigned short value)
{
    return value;
}

#else

unsigned int swap_dword(unsigned int value)
{
    return ((value & 0xff) << 24) |
           ((value & 0xff00) << 8) |
           ((value & 0xff0000) >> 8) |
           ((value & 0xff000000) >> 24); 
}

unsigned short swap_word(unsigned short value)
{
    return ((value & 0xff) << 8) |
           ((value & 0xff00) >> 8);
}

#endif

// =================================================================
unsigned char *load_input_file(char *filename, unsigned int *size)
{
    unsigned char *memory;
    FILE *input = fopen(filename, "rb");
    int exp_size;

    if(!input)
    {
        fprintf(stderr, "\nError: opening file '%s'.", filename);
        return(NULL);
    }
    printf("\nReading '%s'...", filename);
    fflush(stdout);
    // get the filesize
    fseek(input, 0, SEEK_END);
    *size = ftell(input);
    fseek(input, 0, SEEK_SET);
    memory = (unsigned char *) malloc(*size);
    if(!memory)
    {
        fprintf(stderr, "\nError: can't allocate memory.");
        fclose(input);
        return(NULL);
    }
    if(fread(memory, 1, *size, input) != (size_t) *size)
    {
        fprintf(stderr, "\nError: reading input file.");
        fclose(input);
        free(memory);
        return(NULL);
    }
    fclose(input);
    return(memory);
}

// =================================================================
void write_to_output(void *buffer, size_t size)
{
    if(fwrite(buffer, 1, size, output_file) != size)
    {
        fprintf(stderr, "\nError: writing to file.");
        exit(EXIT_FAILURE);
    }
}

// =================================================================
void free_resource()
{

#ifdef __AMIGA__

    printf("\n");

    if(Raw2IffBase) CloseLibrary((struct Library *) Raw2IffBase);
    Raw2IffBase = NULL;

#endif

    if(output_file) fclose(output_file);
    output_file = NULL;
    if(src_mem) free(src_mem);
    src_mem = NULL;
    if(external_mem) free(external_mem);
    external_mem = NULL;
    if(pal_index_file) fclose(pal_index_file);
    pal_index_file = NULL;
    if(pic_index_file) fclose(pic_index_file);
    pic_index_file = NULL;
}

// =================================================================
int main(int argc, char *argv[])
{
	unsigned int size;
	unsigned char *bitmap_mem;
	unsigned char *colors_mem;
	unsigned int external_pal_size;
    char output_name[512];
    char external_pal_name[512];
    int ret_value = 0;
    int arg_pos = 1;
    int width;
    int height;
    int colors;
    int bytes;
    int color_size;
    int stride = 0;
    int pic_offset = 0;
    int pal_in_front = 0;
    int extern_pal = 0;
    int pal_offset = 0;
    ILBM iff_header;
    CAMG iff_camg;
    BODY iff_body;
	COLORMAPENTRY colors_entry;
    int i;
    int j;
    int use_camg = 0;
    int pal_plugins_nbr;
    int selected_pal_plugin = -1;
    int pic_plugins_nbr;
    int selected_pic_plugin = -1;
    int wrong_colors;
    int bitplanes = -1;
    BITPLANES allowed_colors[8] =
    {
        { 2, 1 },
        { 4, 2 },
        { 8, 3 },
        { 16, 4 },
        { 32, 5 },
        { 64, 6 },
        { 128, 7 },
        { 256, 8 }
    };
    
    atexit(free_resource);

    // Load the palette plugins index
    memset(pal_plugins_filenames, 0, sizeof(pal_plugins_filenames));
    
    FILE *pal_index_file = fopen("plugins" SEPARATOR "pal_plugins.txt", "r");

    if(!pal_index_file)
    {
        fprintf(stderr, "\nError: can't open 'plugins" SEPARATOR "pal_plugins.txt'");
        exit(EXIT_FAILURE);
    }

    pal_plugins_nbr = 0;
    for(i = 0; i < 256; i++)
    {
        if(fscanf(pal_index_file, "%s", pal_plugins_filename) == EOF) break;

        if(!strlen(pal_plugins_filename)) break;
        sprintf(pal_plugins_filenames[i], "plugins" SEPARATOR PLUGIN_FILENAME, pal_plugins_filename);
        pal_plugins_nbr++;
    }

    // Load the picture plugins index
    memset(pic_plugins_filenames, 0, sizeof(pic_plugins_filenames));
    
    FILE *pic_index_file = fopen("plugins" SEPARATOR "pic_plugins.txt", "r");

    if(!pic_index_file)
    {
        fprintf(stderr, "\nError: can't open 'plugins" SEPARATOR "pic_plugins.txt'");
        exit(EXIT_FAILURE);
    }

    pic_plugins_nbr = 0;
    for(i = 0; i < 256; i++)
    {
        if(fscanf(pic_index_file, "%s", pic_plugins_filename) == EOF) break;

        if(!strlen(pic_plugins_filename)) break;
        sprintf(pic_plugins_filenames[i], "plugins" SEPARATOR PLUGIN_FILENAME, pic_plugins_filename);
        pic_plugins_nbr++;
    }

#ifdef __AMIGA__

    Raw2IffBase = (struct LibraryHeader *) MakeLibrary(lib_functions, NULL, NULL, sizeof(struct LibraryHeader), 0);
    Raw2IffBase->lh_Library.lib_Node.ln_Type = NT_LIBRARY;
    Raw2IffBase->lh_Library.lib_Node.ln_Name = (char *) "raw2iff.library";
    Raw2IffBase->lh_Library.lib_Flags |= LIBF_CHANGED | LIBF_SUMUSED;
    Raw2IffBase->lh_Library.lib_Version = 1;
    Raw2IffBase->lh_Library.lib_Revision = 0;
    Raw2IffBase->lh_Library.lib_IdString = (char *) "plugins lib";
    Raw2IffBase->lh_Library.lib_OpenCnt = 0;
    AddLibrary((struct Library *) Raw2IffBase);

#endif

    printf("raw2iff v2.2\n");
    printf("Written by Franck 'hitchhikr' Charlet.\n");
    if(argc < 5)
    {
        printf("Usage: raw2iff <-p<n>> <-a<n>> [-f] [-b<n>] [-s<offset>] [-o<offset>] [-e<palette file>[,<offset>]] <width> <height> <colors> <input> [output]\n\n");
        printf("       -p     : source picture plugin number to use\n");
        printf("                %d are plugins available:\n\n", pic_plugins_nbr);

        pic_plugin_struct.command = PLUGIN_GET_NAME;

        for(i = 0; i < pic_plugins_nbr; i++)
        {
            LOAD_PLUGIN(pic_plugins_filenames[i], hpic_plugin, pic_process_address, "\nError: can't load picture plugin.");
            PLUGIN_FUNC(pic_process_address, &pic_plugin_struct)
            printf("                  %d: %s\n", i, pic_plugin_struct.name);
            UNLOAD_PLUGIN(hpic_plugin)
        }
        printf("\n");

        printf("       -a     : source palette plugin number to use\n");
        printf("                %d are plugins available:\n\n", pal_plugins_nbr);

        pal_plugin_struct.command = PLUGIN_GET_NAME;

        for(i = 0; i < pal_plugins_nbr; i++)
        {
            LOAD_PLUGIN(pal_plugins_filenames[i], hpal_plugin, pal_process_address, "\nError: can't load palette plugin.");
            PLUGIN_FUNC(pal_process_address, &pal_plugin_struct)
            printf("                  %d: %s\n", i, pal_plugin_struct.name);
            UNLOAD_PLUGIN(hpal_plugin)
        }
        printf("\n");

        printf("       -f     : palette is located in front of source picture data (after the data by default)\n");
        printf("       -b     : enforce the number of bitplanes\n");
        printf("       -o     : bytes offset in source picture\n");
        printf("       -s     : bytes added after each line processed in source picture\n");
        printf("       -e     : palette is in a specified external file at an optional bytes offset\n");
        printf("       width  : width of the source picture\n");
        printf("       height : height of the source picture\n");
        printf("       colors : 2 4 8 16 32 64 128 or 256\n");
        printf("       input  : raw source file\n");
        printf("       output : iff destination file\n\n");
        printf("Example: raw2iff -p0 -a1 -ePAL,908 320 512 64 INPUT\n");
        printf("         convert a 320x512 64 colors INPUT file using picture plugin 0 with palette plugin 1 located at offset 908 in PAL\n");
        exit(EXIT_FAILURE);
    }
    memset(external_pal_name, 0, sizeof(external_pal_name));

    while(argv[arg_pos][0] == '-')
    {
        i = 1;
        while(argv[arg_pos][i])
        {
            if(argv[arg_pos][i] == 'f' ||
               argv[arg_pos][i] == 'F')
            {
                pal_in_front = 1;
            }
            if(argv[arg_pos][i] == 'p' ||
               argv[arg_pos][i] == 'P')
            {
                i++;
                if(argv[arg_pos][i] == 0)
                {
                    fprintf(stderr, "\nError: missing argument.");
                    exit(EXIT_FAILURE);
                }
                // get plugin index
                selected_pic_plugin = atoi(&argv[arg_pos][i]);
                break;
            }
            if(argv[arg_pos][i] == 'o' ||
               argv[arg_pos][i] == 'O')
            {
                i++;
                if(argv[arg_pos][i] == 0)
                {
                    fprintf(stderr, "\nError: missing argument.");
                    exit(EXIT_FAILURE);
                }
                pic_offset = atoi(&argv[arg_pos][i]);
                break;
            }
            if(argv[arg_pos][i] == 's' ||
               argv[arg_pos][i] == 'S')
            {
                i++;
                if(argv[arg_pos][i] == 0)
                {
                    fprintf(stderr, "\nError: missing argument.");
                    exit(EXIT_FAILURE);
                }
                stride = atoi(&argv[arg_pos][i]);
                break;
            }
            if(argv[arg_pos][i] == 'a' ||
               argv[arg_pos][i] == 'A')
            {
                i++;
                if(argv[arg_pos][i] == 0)
                {
                    fprintf(stderr, "\nError: missing argument.");
                    exit(EXIT_FAILURE);
                }
                // get plugin index
                selected_pal_plugin = atoi(&argv[arg_pos][i]);
                break;
            }
            if(argv[arg_pos][i] == 'b' ||
               argv[arg_pos][i] == 'B')
            {
                i++;
                if(argv[arg_pos][i] == 0)
                {
                    fprintf(stderr, "\nError: missing argument.");
                    exit(EXIT_FAILURE);
                }
                bitplanes = atoi(&argv[arg_pos][i]);
                if(bitplanes <= 0 || bitplanes > 8)
                {
                    fprintf(stderr, "\nError: wrong number of bitplanes (1 to 8).");
                    exit(EXIT_FAILURE);
                }
                break;
            }
            if(argv[arg_pos][i] == 'e' ||
               argv[arg_pos][i] == 'E')
            {
                extern_pal = 1;
                i++;
                j = 0;
                while(argv[arg_pos][i] && argv[arg_pos][i] != ',')
                {
                    external_pal_name[j] = argv[arg_pos][i];
                    j++;
                    i++;
                }
                external_pal_name[j] = 0;
                if(argv[arg_pos][i] == ',')
                {
                    i++;
                    // get offset
                    pal_offset = atoi(&argv[arg_pos][i]);
                }
                break;
            }
            i++;
        }
        arg_pos++;
    }
    if(selected_pal_plugin < 0 || selected_pal_plugin >= pal_plugins_nbr)
    {
        fprintf(stderr, "\nError: wrong palette plugin number.");
        exit(EXIT_FAILURE);
    }
    if(selected_pic_plugin < 0 || selected_pic_plugin >= pic_plugins_nbr)
    {
        fprintf(stderr, "\nError: wrong picture plugin number.");
        exit(EXIT_FAILURE);
    }
    if(stride < 0)
    {
        fprintf(stderr, "\nError: wrong stride offset.");
        exit(EXIT_FAILURE);
    }
    if(pic_offset < 0)
    {
        fprintf(stderr, "\nError: wrong picture offset.");
        exit(EXIT_FAILURE);
    }
    if(pal_offset < 0)
    {
        fprintf(stderr, "\nError: wrong palette offset.");
        exit(EXIT_FAILURE);
    }
    
    width = atoi(argv[arg_pos]);
    if(width <= 0)
    {
        fprintf(stderr, "\nError: wrong width.");
        exit(EXIT_FAILURE);
    }

    arg_pos++;
    height = atoi(argv[arg_pos]);
    if(height <= 0)
    {
        fprintf(stderr, "\nError: wrong height.");
        exit(EXIT_FAILURE);
    }

    arg_pos++;
    colors = atoi(argv[arg_pos]);
    wrong_colors = 0;
    for(i = 0; i < (sizeof(allowed_colors) / sizeof(BITPLANES)); i++)
    {
        if(allowed_colors[i].colors == colors)
        {
            if(bitplanes == -1)
            {
                bitplanes = allowed_colors[i].bitplanes;
            }
            wrong_colors = 1;
            break;
        }
    }
    if(!wrong_colors)
    {
        fprintf(stderr, "\nError: wrong number of colors.");
        exit(EXIT_FAILURE);
    }

    pal_plugin_struct.command = PALETTE_GET_COLOR_BYTES;
    LOAD_PLUGIN(pal_plugins_filenames[selected_pal_plugin], hpal_plugin, pal_process_address, "\nError: can't load palette plugin.");
    PLUGIN_FUNC(pal_process_address, &pal_plugin_struct)
    UNLOAD_PLUGIN(hpal_plugin)

    color_size = pal_plugin_struct.color_size;

    pal_plugin_struct.command = PLUGIN_GET_NAME;
    LOAD_PLUGIN(pal_plugins_filenames[selected_pal_plugin], hpal_plugin, pal_process_address, "\nError: can't load palette plugin.");
    PLUGIN_FUNC(pal_process_address, &pal_plugin_struct)
    UNLOAD_PLUGIN(hpal_plugin)

    printf("\nUsing palette plugin: %s\n", pal_plugin_struct.name);

    pic_plugin_struct.command = PLUGIN_GET_NAME;
    LOAD_PLUGIN(pic_plugins_filenames[selected_pic_plugin], hpic_plugin, pic_process_address, "\nError: can't load picture plugin.");
    PLUGIN_FUNC(pic_process_address, &pic_plugin_struct)
    UNLOAD_PLUGIN(hpic_plugin)
    printf("Using picture plugin: %s\n", pic_plugin_struct.name);

    arg_pos++;

    // correct width
    bytes = width / 8;
    if(width % 8)
    {
        bytes = (width + 8) / 8;
        width = bytes * 8;
    }

	if(src_mem = load_input_file(argv[arg_pos], &size))
	{
        arg_pos++;

        pic_plugin_struct.bytes = bytes;
        pic_plugin_struct.width = width;
        pic_plugin_struct.height = height;
        pic_plugin_struct.bitplanes = bitplanes;
        pic_plugin_struct.stride = stride;
        pic_plugin_struct.command = PICTURE_GET_CALC_MAX_SIZE;
        LOAD_PLUGIN(pic_plugins_filenames[selected_pic_plugin], hpic_plugin, pic_process_address, "\nError: can't load picture plugin.");
        PLUGIN_FUNC(pic_process_address, &pic_plugin_struct)
        UNLOAD_PLUGIN(hpic_plugin)

        if(size < (pic_plugin_struct.size + pic_offset + (extern_pal ? 0: (colors * color_size))))
        {
            fprintf(stderr, "\nError: wrong file size.");
            exit(EXIT_FAILURE);
        }
        if(extern_pal)
        {
            if(!(external_mem = load_input_file(external_pal_name, &external_pal_size)))
            {
                exit(EXIT_FAILURE);
            }
            if((pal_offset + (colors * color_size)) > external_pal_size)
            {
                fprintf(stderr, "\nError: palette file is too small.");
                exit(EXIT_FAILURE);
            }
        }

        if(argc <= arg_pos)
        {
            // user didn't supply an output name
            strcpy(output_name, argv[arg_pos - 1]);
            strcat(output_name, ".iff");
        }
        else
        {
            strcpy(output_name, argv[arg_pos]);
        }
        printf("\nWriting '%s'...", output_name);
        fflush(stdout);
        output_file = fopen(output_name, "wb");
        if(output_file)
        {
            // Optional CAMG chunk
            memcpy(iff_camg.ChunkId, "CAMG", 4);
            iff_camg.Size = swap_dword(4);
            use_camg = 0;
            // HAM6
            if(bitplanes == 6 && colors == 16)
            {
                use_camg = 1;
                iff_camg.Data = swap_dword(0x11800);
            }
            // HAM8
            else if(bitplanes == 8 && colors == 64)
            {
                use_camg = 1;
                iff_camg.Data = swap_dword(0x11800);
            }
            // Halfbrite
            else if(bitplanes == 6 && colors == 32)
            {
                use_camg = 1;
                iff_camg.Data = swap_dword(0x11080);
            }

            memset(&iff_header, 0, sizeof(iff_header));
            memcpy(iff_header.ChunkId, "FORM", 4);
            iff_header.Size = swap_dword(sizeof(iff_header) +
                                         (sizeof(COLORMAPENTRY) * colors) +
                                         sizeof(iff_body) +
                                         (bytes * height * bitplanes) +
                                         (use_camg ? sizeof(iff_camg) : 0) -
                                         8);
            memcpy(iff_header.TypeID, "ILBM", 4);
            memcpy(iff_header.Bmhd.ChunkId, "BMHD", 4);
            iff_header.Bmhd.Size = swap_dword(sizeof(iff_header.Bmhd) - 8);
            iff_header.Bmhd.Width = swap_word(width);
            iff_header.Bmhd.Height = swap_word(height);
            iff_header.Bmhd.Left = swap_word(0);
            iff_header.Bmhd.Top = swap_word(0);
            iff_header.Bmhd.Bitplanes = bitplanes;
            iff_header.Bmhd.Masking = 0;
            iff_header.Bmhd.Compress = 0;
            iff_header.Bmhd.Padding = 0;
            iff_header.Bmhd.Transparency = 0;
            iff_header.Bmhd.XAspectRatio = 1;
            iff_header.Bmhd.YAspectRatio = 1;
            iff_header.Bmhd.PageWidth = swap_word(width);
            iff_header.Bmhd.PageHeight = swap_word(height);
            
            // Convert the palette
            memcpy(iff_header.Cmap.ChunkId, "CMAP", 4);
            iff_header.Cmap.Size = swap_dword((sizeof(COLORMAPENTRY) * colors));
            write_to_output(&iff_header, sizeof(iff_header));
            if(extern_pal)
            {
                colors_mem = external_mem + pal_offset;
            }
            else
            {
                if(pal_in_front)
                {
                    colors_mem = src_mem;
                }
                else
                {
                    colors_mem = src_mem + (bytes * height * bitplanes);
                }
            }
            pal_plugin_struct.command = PALETTE_GET_COLOR_VALUE;
            pal_plugin_struct.entry = colors_mem;
            pal_plugin_struct.colors = colors;
            pal_plugin_struct.output_file = output_file;
            LOAD_PLUGIN(pal_plugins_filenames[selected_pal_plugin], hpal_plugin, pal_process_address, "\nError: can't load palette plugin.");
            PLUGIN_FUNC(pal_process_address, &pal_plugin_struct)
            UNLOAD_PLUGIN(hpal_plugin)
            fflush(output_file);

            if(use_camg)
            {
                write_to_output(&iff_camg, sizeof(iff_camg));
            }

            // Convert the picture
            memcpy(iff_body.ChunkId, "BODY", 4);
            iff_body.Size = swap_dword((bytes * height * bitplanes));
            write_to_output(&iff_body, sizeof(iff_body));

            bitmap_mem = src_mem + pic_offset;
            if(pal_in_front && !extern_pal)
            {
                bitmap_mem += (colors * color_size);
            }
            pic_plugin_struct.command = PICTURE_GEN_PICTURE;
            pic_plugin_struct.output_file = output_file;
            pic_plugin_struct.entry = bitmap_mem;
            LOAD_PLUGIN(pic_plugins_filenames[selected_pic_plugin], hpic_plugin, pic_process_address, "\nError: can't load picture plugin.");
            PLUGIN_FUNC(pic_process_address, &pic_plugin_struct)
            UNLOAD_PLUGIN(hpic_plugin)
            switch(pic_plugin_struct.error)
            {
                case PLUGIN_ERROR_WRITE:
                    fprintf(stderr, "\nError: writing to file.");
                    exit(EXIT_FAILURE);
                    break;
                case PLUGIN_ERROR_BOUNDS:
                    fprintf(stderr, "\nError: color index out of bounds in picture.");
                    exit(EXIT_FAILURE);
                    break;
                case PLUGIN_MEMORY:
                    fprintf(stderr, "\nError: not enough memory.");
                    exit(EXIT_FAILURE);
                    break;
            }
        }
        else
        {
            fprintf(stderr, "\nError: can't open '%s' for writing.", output_name);
            exit(EXIT_FAILURE);
        }
	}
    else
    {
        exit(EXIT_FAILURE);
    }
    exit(EXIT_SUCCESS);
}
