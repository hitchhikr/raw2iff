// =================================================================
// raw2iff v1.1
// Written by Franck 'hitchhikr' Charlet.
// =================================================================

// =================================================================
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <strings.h>

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
	unsigned char Red;          /* Red color component (0-255) */
	unsigned char Green;        /* Green color component (0-255) */
	unsigned char Blue;         /* Blue color component (0-255) */
} COLORMAPENTRY;

typedef struct
{
	char  ChunkId[4];           /* Chunk Identifier "CMAP" */
	unsigned int Size;          /* Size of chunk data in bytes */
} CMAP;

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
int main(int argc, char *argv[])
{
	unsigned int size;
	unsigned char *src_mem;
	unsigned char *bitmap_mem;
	unsigned char *planar_mem = NULL;
	unsigned char *colors_mem;
	unsigned char *external_mem = NULL;
	unsigned int external_pal_size;
	FILE *output_file;
    char output_name[512];
    char external_pal_name[512];
    int ret_value = 0;
    int arg_pos = 1;
    int width;
    int height;
    int colors;
    int bytes;
    int color_size;
    int planar_offset;
    int error_chunky;
    int src_interleaved = 0;
    int src_chunky = 0;
    int pal_8_bits = 0;
    int pal_alpha = 0;
    int pal_in_front = 0;
    int extern_pal = 0;
    int color_4_bit;
    int pal_offset = 0;
    ILBM iff_header;
    BODY iff_body;
	COLORMAPENTRY colors_entry;
    int i;
    int j;
    int wrong_colors;
    int bitplanes;
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

    printf("raw2iff v1.1\n");
    printf("Written by Franck 'hitchhikr' Charlet.\n");
    if(argc < 5)
    {
        printf("Usage: raw2iff [-i] [-c] [-8] [-a] [-f] [-e<palette file>[,<offset>]] <width> <height> <colors> <input> [output]\n\n");
        printf("       -i     : source picture data are interleaved\n");
        printf("       -c     : source picture data are chunky\n");
        printf("       -8     : source palette components are 8 bit (default is 4 bit)\n");
        printf("       -a     : source 8 bit palette have an alpha component (ARGB)\n");
        printf("       -f     : palette is located in front of source picture data\n");
        printf("       -e     : palette is in a specified external file at an optional bytes offset\n");
        printf("       width  : width of the source picture\n");
        printf("       height : height of the source picture\n");
        printf("       colors : 2 4 8 16 32 64 128 or 256\n");
        printf("       input  : raw source file\n");
        printf("       output : iff destination file\n\n");
        printf("Example: raw2iff -8 -ePAL,908 320 512 64 INPUT\n");
        printf("         convert a 320x512 64 colors INPUT picture with RGB 8 bit palette located at offset 908 from PAL\n");
        return ret_value;
    }
    memset(external_pal_name, 0, sizeof(external_pal_name));

    while(argv[arg_pos][0] == '-')
    {
        i = 1;
        while(argv[arg_pos][i])
        {
            if(argv[arg_pos][i] == 'i' ||
               argv[arg_pos][i] == 'I')
            {
                src_interleaved = 1;
            }
            if(argv[arg_pos][i] == 'c' ||
               argv[arg_pos][i] == 'C')
            {
                src_chunky = 1;
            }
            if(argv[arg_pos][i] == '8')
            {
                pal_8_bits = 1;
            }
            if(argv[arg_pos][i] == 'f' ||
               argv[arg_pos][i] == 'F')
            {
                pal_in_front = 1;
            }
            if(argv[arg_pos][i] == 'a' ||
               argv[arg_pos][i] == 'A')
            {
                pal_alpha = 1;
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
    if(pal_offset < 0)
    {
        fprintf(stderr, "\nError: wrong offset.");
        return 1;
    }
    
    width = atoi(argv[arg_pos]);
    if(width <= 0)
    {
        fprintf(stderr, "\nError: wrong width.");
        return 1;
    }
    arg_pos++;
    height = atoi(argv[arg_pos]);
    if(height <= 0)
    {
        fprintf(stderr, "\nError: wrong height.");
        return 1;
    }
    arg_pos++;
    colors = atoi(argv[arg_pos]);
    wrong_colors = 0;
    for(i = 0; i < (sizeof(allowed_colors) / sizeof(BITPLANES)); i++)
    {
        if(allowed_colors[i].colors == colors)
        {
            bitplanes = allowed_colors[i].bitplanes;
            wrong_colors = 1;
            break;
        }
    }

    if(!wrong_colors)
    {
        fprintf(stderr, "\nError: wrong number of colors.");
        return 1;
    }
    if(pal_8_bits)
    {
        color_size = 3 + pal_alpha;
    }
    else
    {
        color_size = 2;
    }
    arg_pos++;

    bytes = width / 8;

	if(src_mem = load_input_file(argv[arg_pos], &size))
	{
        arg_pos++;

        if(src_chunky)
        {
            if(size < ((width * height) + (extern_pal ? 0: (colors * color_size))))
            {
                free(src_mem);
                fprintf(stderr, "\nError: wrong file size.");
                return 1;
            }
        }
        else
        {
            if(size < ((bytes * height * bitplanes) + (extern_pal ? 0: (colors * color_size))))
            {
                free(src_mem);
                fprintf(stderr, "\nError: wrong file size.");
                return 1;
            }
        }
        if(extern_pal)
        {
            if(!(external_mem = load_input_file(external_pal_name, &external_pal_size)))
            {
                free(src_mem);
                return 1;
            }
            if((pal_offset + (colors * color_size)) > external_pal_size)
            {
                free(src_mem);
                fprintf(stderr, "\nError: palette file is too small.");
                return 1;
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
        output_file = fopen(output_name, "wb");
        if(output_file)
        {
            memset(&iff_header, 0, sizeof(iff_header));
            memcpy(iff_header.ChunkId, "FORM", 4);
            iff_header.Size = swap_dword(sizeof(iff_header) +
                                         (sizeof(COLORMAPENTRY) * colors) +
                                         sizeof(iff_body) +
                                         (bytes * height * bitplanes) -
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
            
            memcpy(iff_header.Cmap.ChunkId, "CMAP", 4);
            iff_header.Cmap.Size = swap_dword((sizeof(COLORMAPENTRY) * colors));
            fwrite(&iff_header, 1, sizeof(iff_header), output_file);

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

            for(i = 0; i < colors; i++)
            {
                if(pal_8_bits)
                {
                    colors_entry.Red = colors_mem[(i * color_size) + (1 - (1 - pal_alpha))];
                    colors_entry.Green = colors_mem[(i * color_size) + (2 - (1 - pal_alpha))];
                    colors_entry.Blue = colors_mem[(i * color_size) + (3 - (1 - pal_alpha))];
                }
                else
                {
                    color_4_bit = (colors_mem[(i * color_size)] << 8) | colors_mem[(i * color_size) + 1];
                    colors_entry.Red = (color_4_bit & 0xf00) >> 4;
                    colors_entry.Green = (color_4_bit & 0xf0);
                    colors_entry.Blue = (color_4_bit & 0xf) << 4;
                }
                fwrite(&colors_entry, 1, sizeof(COLORMAPENTRY), output_file);
            }
            
            memcpy(iff_body.ChunkId, "BODY", 4);
            iff_body.Size = swap_dword((bytes * height * bitplanes));
            fwrite(&iff_body, 1, sizeof(iff_body), output_file);

            bitmap_mem = src_mem;
            if(pal_in_front)
            {
                bitmap_mem = src_mem + (colors * color_size);
            }

            if(src_chunky)
            {
                planar_mem = malloc(bytes * bitplanes);
                for(j = 0; j < height; j++)
                {
                    memset(planar_mem, 0, bytes * bitplanes);
                    error_chunky = 0;
                    for(i = 0; i < width; i++)
                    {
                        // Convert a line to planar
                        planar_offset = bitmap_mem[i + (j * width)];
                        
                        if(planar_offset & 1)
                        {
                            planar_mem[(i / 8) + (0 * bytes)] |= 1 << (7 - (i % 8));
                        }
                        if(planar_offset & 2)
                        {
                            if(bitplanes < 2)
                            {
                                error_chunky = 1;
                                goto bailout;
                            }
                            planar_mem[(i / 8) + (1 * bytes)] |= 1 << (7 - (i % 8));
                        }
                        if(planar_offset & 4)
                        {
                            if(bitplanes < 3)
                            {
                                error_chunky = 1;
                                goto bailout;
                            }
                            planar_mem[(i / 8) + (2 * bytes)] |= 1 << (7 - (i % 8));
                        }
                        if(planar_offset & 8)
                        {
                            if(bitplanes < 4)
                            {
                                error_chunky = 1;
                                goto bailout;
                            }
                            planar_mem[(i / 8) + (3 * bytes)] |= 1 << (7 - (i % 8));
                        }
                        if(planar_offset & 16)
                        {
                            if(bitplanes < 5)
                            {
                                error_chunky = 1;
                                goto bailout;
                            }
                            planar_mem[(i / 8) + (4 * bytes)] |= 1 << (7 - (i % 8));
                        }
                        if(planar_offset & 32)
                        {
                            if(bitplanes < 6)
                            {
                                error_chunky = 1;
                                goto bailout;
                            }
                            planar_mem[(i / 8) + (5 * bytes)] |= 1 << (7 - (i % 8));
                        }
                        if(planar_offset & 64)
                        {
                            if(bitplanes < 7)
                            {
                                error_chunky = 1;
                                goto bailout;
                            }
                            planar_mem[(i / 8) + (6 * bytes)] |= 1 << (7 - (i % 8));
                        }
                        if(planar_offset & 128)
                        {
                            if(bitplanes < 8)
                            {
                                error_chunky = 1;
                                goto bailout;
                            }
                            planar_mem[(i / 8) + (7 * bytes)] |= 1 << (7 - (i % 8));
                        }
                    }
                    fwrite(planar_mem, 1, bytes * bitplanes, output_file);
                }
bailout:;
                if(error_chunky)
                {
                    fprintf(stderr, "\nError: color index out of bounds in chunky picture.");
                    ret_value = 1;
                }
            }
            else
            {
                if(src_interleaved)
                {
                    // write it as-is
                    fwrite(bitmap_mem, 1, (bytes * height * bitplanes), output_file);
                }
                else
                {
                    for(j = 0; j < height; j++)
                    {
                        for(i = 0; i < bitplanes; i++)
                        {
                            fwrite(&bitmap_mem[(i * (height * bytes)) + (j * bytes)], 1, bytes, output_file);
                        }
                    }
                }
            }
            fclose(output_file);
        }
        else
        {
            fprintf(stderr, "\nError: can't open '%s' for writing.", output_name);
            ret_value = 1;
        }
        if(extern_pal)
        {
            free(external_mem);
        }
        if(planar_mem) free(planar_mem);
		free(src_mem);
	}
    else
    {
        ret_value = 1;
    }
#ifdef __AMIGA__
    printf("\n");
#endif
	return ret_value;
}
