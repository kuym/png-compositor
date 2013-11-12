#define _ _ /*

#####  Execute this file to build it:  #####
#
#  $ chmod +x png-composite.cpp  #(if necessary)
#  $ ./png-composite.cpp
#

g++ -I libpng-1.6.6 -lz -o png-composite libpng-1.6.6/.libs/libpng16.a png-composite.cpp
if [ $? -eq 0 ]
then
	otool -L png-composite
fi

exit
*/

#include <stdio.h>
#include <stdlib.h>
#include "png.h"

void abort(char const* message, char const* arg = 0)
{
	if(arg)
		printf(message, arg);
	else
		printf(message);
	exit(0);
}

class PNG
{
public:
	PNG(void):
		width(0),
		height(0),
		rowBytes(0),
		colorType(0),
		bitDepth(0),
		numPasses(0),
		rowPointers(0)
	{
		;
	}

	~PNG(void)
	{
		if(rowPointers != 0)
		{
			for(int y = 0; y < height; y++)
				delete[] rowPointers[y];
			delete[] rowPointers;
		}
	}

	bool	read(char const* fileName)
	{
		unsigned char header[8];    // 8 is the maximum size that can be checked

		// open file and test for it being a png
		FILE* fp = fopen(fileName, "rb");
		if(!fp)
			abort("File %s could not be opened for reading", fileName);
		fread(header, 1, 8, fp);
		if(png_sig_cmp(header, 0, 8))
			abort("File %s is not recognized as a PNG file", fileName);

		// initialize stuff
		png_structp pngPtr = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);

		if(!pngPtr)
			abort("[png_create_read_struct failed");

		png_infop infoPtr = png_create_info_struct(pngPtr);
		if(!infoPtr)
			abort("png_create_info_struct failed");

		//if(setjmp(png_jmpbuf(pngPtr)))
		//	abort("[read] Error during init_io");

		png_init_io(pngPtr, fp);
		png_set_sig_bytes(pngPtr, 8);

		png_read_info(pngPtr, infoPtr);

		width = png_get_image_width(pngPtr, infoPtr);
		height = png_get_image_height(pngPtr, infoPtr);
		colorType = png_get_color_type(pngPtr, infoPtr);
		bitDepth = png_get_bit_depth(pngPtr, infoPtr);

		numPasses = png_set_interlace_handling(pngPtr);
		png_read_update_info(pngPtr, infoPtr);

		// read file
		//if(setjmp(png_jmpbuf(pngPtr)))
		//	abort("[read] Error during read_image");

		rowPointers = new unsigned char*[height];
		rowBytes = png_get_rowbytes(pngPtr, infoPtr);
		for(int y = 0; y < height; y++)
			rowPointers[y] = new unsigned char[rowBytes];

		png_read_image(pngPtr, rowPointers);

		fclose(fp);

		return(true);
	}

	void write(char const* fileName)
	{
		// create file
		FILE *fp = fopen(fileName, "wb");
		if (!fp)
			abort("[write] File %s could not be opened for writing", fileName);

		// initialize stuff
		png_structp pngPtr = png_create_write_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);

		if(!pngPtr)
			abort("[write] png_create_write_struct failed");

		png_infop infoPtr = png_create_info_struct(pngPtr);
		if(!infoPtr)
			abort("[write] png_create_info_struct failed");

		//if(setjmp(png_jmpbuf(pngPtr)))
		//	abort("[write] Error during init_io");

		png_init_io(pngPtr, fp);

		// write header
		//if(setjmp(png_jmpbuf(pngPtr)))
		//	abort("[write] Error during writing header");

		png_set_IHDR(	pngPtr, infoPtr, width, height,
						bitDepth, colorType, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE,
						PNG_FILTER_TYPE_BASE
					);

		png_write_info(pngPtr, infoPtr);

		// write bytes
		//if(setjmp(png_jmpbuf(pngPtr)))
		//	abort("[write] Error during writing bytes");

		png_write_image(pngPtr, rowPointers);

		// end write
		//if(setjmp(png_jmpbuf(pngPtr)))
		//	abort("[write] Error during end of write");

		png_write_end(pngPtr, NULL);
		
		fclose(fp);
	}

	bool composite(PNG* overlay)
	{
		// assert that the images are the one particular format that this tool is designed to process
		if(		(bitDepth != 8) || (overlay->bitDepth != 8) ||
				(colorType != PNG_COLOR_TYPE_RGB) || (overlay->colorType != PNG_COLOR_TYPE_RGB_ALPHA) ||
				(width != overlay->width) || (height != overlay->height)
			)
			return(false);

		for(int y = 0; y < height; y++)
		{
			for(int x = 0; x < width; x++)
			{
				unsigned char* bPixel = rowPointers[y] + (x * 3);
				unsigned char* tPixel = overlay->rowPointers[y] + (x << 2);

				int alpha = tPixel[3];
				for(int channel = 0; channel < 3; channel++)
					bPixel[channel] = ((255 - alpha) * (int)bPixel[channel] + (alpha * (int)tPixel[channel])) / 255;
			}
		}

		return(true);
	}

	bool	dump(char const* fileName)
	{
		FILE* out = fopen(fileName, "wb");

		for(int y = 0; y < height; y++)
			fwrite(rowPointers[y], 1, rowBytes, out);

		fclose(out);

		return(true);
	}

private:
	int				width;
	int				height;
	int				rowBytes;
	unsigned char	colorType;
	unsigned char	bitDepth;
	int				numPasses;
	unsigned char**	rowPointers;
};

int main(int argc, char const* const* argv)
{
	if(argc != 4)
		abort("Usage: tool <input-bottom.png> <input-top.png> <output.png>");

	PNG pngBottom, pngTop;

	pngBottom.read(argv[1]);
	pngTop.read(argv[2]);

	if(pngBottom.composite(&pngTop))
		pngBottom.write(argv[3]);
	else
		printf("Could not composite, images aren't in the supported formats (same dimensions, bottom is 24bpp, top is 32bpp.\n");

	return(0);
}