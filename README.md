raw2iff is a plugins driven tool to convert RAW pictures to IFF ILBM pictures format (mainly used on Amiga).

Windows and Amiga executables/plugins are provided.

Usage: raw2iff \<-p\<n\>\> \<-a\<n\>\> [-f] [-b\<n\>] [-e\<palette file\>[,<offset>]] \<width\> \<height\> \<colors\> \<input\> [output]

       -p<n>  : source picture plugin number to use
                3 are plugins available:

                0: Planar picture
                1: Interleaved planar picture
                2: 8 bit chunky picture

       -a<n>  : source palette plugin number to use
                3 are plugins available:

                0: Alpha:8 Red:8 Green:8 Blue:8
                1: Red:4 Green:4 Blue:4
                2: Red:8 Green:8 Blue:8

       -f     : palette is located in front of source picture data (after the data by default)
       -b<n>  : enforce the number of bitplanes
       -e     : palette is in a specified external file at an optional bytes offset
       width  : width of the source picture
       height : height of the source picture
       colors : 2 4 8 16 32 64 128 or 256
       input  : raw source file
       output : iff destination file

Example: raw2iff -p0 -a1 -ePAL,908 320 512 64 INPUT
         convert a 320x512 64 colors INPUT file using picture plugin 0 with palette plugin 1 located at offset 908 in PAL

=============

v2.0:

- Now plugins driven.
- Fixed non-8 bit aligned widths.
- Number of bitplanes can now be specified.
- Now generates HAM6/HAM8/Halfbrite pictures if the number of colors and bitplanes correspond:
  - HAM6: 16 colors / 6 bitplanes.
  - HAM8: 64 colors / 8 bitplanes.
  - Halfbrite: 32 colors / 6 bitplanes.

=============

v1.1:

- Added support for chunky pictures.
