raw2iff is a tool to convert RAW PLANAR or CHUNKY pictures to IFF ILBM pictures format (mainly used on Amiga).

Windows and Amiga executables are provided.

Usage: raw2iff [-i] [-c] [-8] [-a] [-f] [-e\<palette file\>[,\<offset\>]] \<width\> \<height\> \<colors\> \<input\> [output]

       -i     : source picture data are interleaved
       -c     : source picture data are chunky
       -8     : source palette components are 8 bit (default is 4 bit)
       -a     : source 8 bit palette have an alpha component (ARGB)
       -f     : palette is located in front of source picture data
       -e     : palette is in a specified external file at an optional bytes offset
       width  : width of the source picture
       height : height of the source picture
       colors : 2 4 8 16 32 64 128 or 256
       input  : raw source file
       output : iff destination file

Example:<br>
raw2iff -8 -ePAL,908 320 512 64 INPUT<br>
convert a 320x512 64 colors INPUT planar picture with RGB 8 bit palette located at offset 908 in PAL

v1.1:

- Added support for chunky pictures.
