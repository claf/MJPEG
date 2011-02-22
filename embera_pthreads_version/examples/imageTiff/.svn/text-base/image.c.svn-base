/*
 * Open Systems Laboratory
 * http://www.lam-mpi.org/tutorials/
 * Indiana University
 *
 * MPI Tutorial
 * Lab : Image processing -- sum of squares
 *
 * Mail questions regarding tutorial material to lam at lam dash mpi dot org
 *
 * This code mainly cobbled together from the Parallel Image
 * Processing Toolkit (PIPT).  As such, parts of it probably don't
 * make sense in this tutorial context.  Don't read this code.  Just
 * use the "loadtiff()" function to read in the irish.tif file.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tiffio.h>
#include <tiff.h>

const int CORRECT = 0;
const int ERROR = -1;

typedef char byte;
typedef unsigned char PIXEL;

typedef struct {
  int     fmt;
  uint32  h;                    /* height */
  uint32  w;                    /* width */
  uint32  p;                    /* #planes, e.g. 3 for RGB, 1 for Grayscale */
  PIXEL   ***data;              /* raster */
                                /* data[0][0] points to the beginning of a */
                                /* contiguous raster of h*w pixels; */
                                /* data[k][0] points to the beginning of */
                                /* the k-th plane (or color); */
                                /* data[k][i] points to the first pixel in */
                                /* the i-th row of the k-th plane. */
} IMAGE;

/**************************************************************

dumpTiff()

**************************************************************/

int dumpTiff(char *fileName, unsigned char *pixels, int *w, int *h) {
	TIFF *tfp;
	uint32 pixelsPerRaster;
	int i;
	
	pixelsPerRaster = *w * *h;
	
	tfp = TIFFOpen(fileName, "w");
	if (! tfp) {
		fprintf(stderr, "dumpTiff(): Can't write to file %s.\n", fileName);
		return ERROR;
	}
	
	TIFFSetField(tfp, TIFFTAG_IMAGEWIDTH,  *w);
	TIFFSetField(tfp, TIFFTAG_IMAGELENGTH, *h);
	TIFFSetField(tfp, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
	TIFFSetField(tfp, TIFFTAG_BITSPERSAMPLE, 8);
	TIFFSetField(tfp, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
	
	/* Assuming COLOR_RGB */
	
	TIFFSetField(tfp, TIFFTAG_PLANARCONFIG, PLANARCONFIG_SEPARATE);
	TIFFSetField(tfp, TIFFTAG_SAMPLESPERPIXEL, 3);
	TIFFSetField(tfp, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);
	
	for (i = 0; i < 3; i++) {
		if (TIFFWriteEncodedStrip(tfp, i, &pixels[i * (*w)], 
					pixelsPerRaster) < 0) {
		fprintf(stderr, "dumpTiff(): Can't write encoded strip.\n");
		TIFFClose(tfp);
		return ERROR;
		}
	}
	
	TIFFClose(tfp);
	return CORRECT;
}

/* IMAGE I/O Formats */

#ifndef UNKNOWN_FORMAT
#define UNKNOWN_FORMAT   (  0)
#endif

#define BINARY           ( 11)          /* 2^1 shades of gray, 0 = black */
#define GRAYSCALE_4      ( 14)          /* 2^4 shades of gray, 0 = black */
#define GRAYSCALE_8      ( 18)          /* 2^8 shades of gray, 0 = black */
#define COLOR_PALETTE_4  ( 24)          /*  4 bit color palette */
#define COLOR_PALETTE_8  ( 28)		/*  8 bit color palette */
#define COLOR_RGB_24     (124)		/*  8 bit red,  8 bit green, ... */
#define COLOR_RGB_48     (148)	        /* 16 bit red, 16 bit green, ... */

static int stopOnError = 0;


/**************************************************************

tiffToIOFormat()

**************************************************************/

/* tiffToIOFormat converts the essential tiff format parameters into the
 * corresponding I/O Format Code.
 */

static int tiffToIOFormat(uint16 bps, uint16 pm, uint16 spp) {

	int fmt;

	switch(pm) {
	case PHOTOMETRIC_MINISBLACK:
	case PHOTOMETRIC_MINISWHITE:
	switch(bps) {
		case 1:
		fmt = BINARY;
		break;
	case 4:
		fmt = GRAYSCALE_4;
		break;
	case 8:
		fmt = GRAYSCALE_8;
		break;
	default:
	fprintf(stderr, 
		"tiffToIOFormat():  Can't handle a grayscale with %d bits/sample\n",
		bps);
	return UNKNOWN_FORMAT;
	}
	break;
	case PHOTOMETRIC_PALETTE:
	switch(bps) {
	case 4:
	fmt = COLOR_PALETTE_4;
	break;
	case 8:
	fmt = COLOR_PALETTE_8;
	break;
	default:
	fprintf(stderr, 
		"tiffToIOFormat():  Can't handle a color palette with %d bits/sample\n",
		bps);
	return UNKNOWN_FORMAT;
	}
	break;
	case PHOTOMETRIC_RGB:
	switch((short) spp) {
	case 4:
	fprintf(stderr, "tiffToIOFormat(): Caution: Alpha channel ignored.\n");
	break;
		case 3:
		break;
	default:
	fprintf(stderr, 
		"tiffToIOFormat(): Can't handle %d samples/pixel RGB image\n", spp);
	return UNKNOWN_FORMAT;
	}
	
	switch(bps) {
	case 8:
	fmt = COLOR_RGB_24;
	break;
	case 16:
	fmt = COLOR_RGB_48;
	break;
	default:
	fprintf(stderr, 
		"tiffToIOFormat():  Can't handle a %d bits/pixel RGB image\n", 3*bps);
	return UNKNOWN_FORMAT;
	}
	break;
	default:
	fprintf(stderr, 
		"tiffToIOFormat(): Can't handle a tiff photometric == %d\n", pm);
	return UNKNOWN_FORMAT;
	}
	return fmt;
}


/**************************************************************

loadContigStripData()

**************************************************************/

static int 
loadContigStripData(TIFF *tfp, uint32 w, uint32 h, PIXEL ***raster,
		    uint32 samplesToLoad, uint32 samplesPerPixel)
{
  byte *strip;
  uint32 rowsPerStrip;
  uint32 topRow;
  uint32 nRowsToConvert;
  uint32 dstPixel = 0;
  uint32 srcPixel;
  uint32 sample;
  uint32 scanLineSize = TIFFScanlineSize(tfp);
  
  strip = (byte *) malloc(TIFFStripSize(tfp));
  if (! strip) {
    fprintf(stderr, "loadContigStripData(): Can't malloc strip[%lu]\n",
        TIFFStripSize(tfp));
    return ERROR;
  }

  TIFFGetFieldDefaulted(tfp, TIFFTAG_ROWSPERSTRIP, &rowsPerStrip);

  /* N.B. dstPixel is initialized ot 0 above */
  for(topRow = 0; topRow < h; topRow += rowsPerStrip) {
    nRowsToConvert = (topRow + rowsPerStrip > h ? h - topRow : rowsPerStrip);

    if (TIFFReadEncodedStrip(tfp, TIFFComputeStrip(tfp, topRow, 0),
	 strip, nRowsToConvert*scanLineSize) < 0 && stopOnError) {
      free(strip);
      return ERROR;
    }
	
    for(srcPixel = 0; srcPixel < nRowsToConvert*w*samplesPerPixel;
	srcPixel += samplesPerPixel, dstPixel++)
      for(sample = 0; sample < samplesToLoad; sample++)
	raster[sample][0][dstPixel] = strip[srcPixel + sample];
  }

  free(strip);
  return CORRECT;
}


/**************************************************************

loadSeparateStripData()

**************************************************************/

static int 
loadSeparateStripData(TIFF *tfp, uint32 w, uint32 h, PIXEL ***raster,
		      uint32 samplesToLoad, uint32 samplesPerPixel)
{
  byte   *strip[3];
  uint32 rowsPerStrip;
  uint32 topRow;
  uint32 nRowsToConvert;
  uint32 dstPixel = 0;
  uint32 srcPixel;
  uint32 sample;
  uint32 i;
  uint32 scanLineSize = TIFFScanlineSize(tfp);

  for(sample = 0; sample < samplesToLoad; sample++) {
    strip[sample] = (byte *) malloc(TIFFStripSize(tfp));
    if (! strip[sample]) {
      for(i = 0; i < sample; i++) free(strip[i]);
      fprintf(stderr, 
          "loadSeparateStripData(): Can't malloc strip[%lu]\n", sample);
      return ERROR;
    }
  }

  TIFFGetFieldDefaulted(tfp, TIFFTAG_ROWSPERSTRIP, &rowsPerStrip);

  /* N.B. dstPixel is initialized to 0 above */
  for(topRow = 0; topRow < h; topRow += rowsPerStrip) {
    nRowsToConvert = (topRow + rowsPerStrip > h ? h - topRow : rowsPerStrip);
    for(sample = 0; sample < samplesToLoad; sample++) {
      if (TIFFReadEncodedStrip(tfp, TIFFComputeStrip(tfp, topRow, sample),
          strip[sample], nRowsToConvert*scanLineSize) < 0
              && stopOnError) {
	for(i = 0; i < samplesToLoad; i++) free(strip[i]);
	return ERROR;
      }
    }

    for(srcPixel = 0; srcPixel < nRowsToConvert*w; srcPixel++, dstPixel++) {
      for(sample = 0; sample < samplesToLoad; sample++) 
	raster[sample][0][dstPixel]   = strip[sample][srcPixel];
    }
  }

  for(sample = 0; sample < samplesToLoad; sample++) free(strip[sample]);
  return CORRECT;
}


/**************************************************************

loadContigTileData()

**************************************************************/

static int 
loadContigTileData(TIFF *tfp, uint32 w, uint32 h, PIXEL ***raster,
		   uint32 samplesToLoad, uint32 samplesPerPixel)
{
  byte *tile;
  uint32 rowsPerTile;
  uint32 colsPerTile;
  uint32 topRow;
  uint32 leftCol;

  uint32 nRowsToConvert;
  uint32 nColsToConvert;

  uint32 tileRowOffset;
  uint32 sample;
  uint32 i, j;

  tile = (byte *) malloc(TIFFTileSize(tfp));
  if (! tile) {
    fprintf(stderr, "loadContigTileData(): Can't malloc tile\n");
    return ERROR;
  }

  TIFFGetFieldDefaulted(tfp, TIFFTAG_TILELENGTH, &rowsPerTile);
  TIFFGetFieldDefaulted(tfp, TIFFTAG_TILEWIDTH,  &colsPerTile);

  for(topRow = 0; topRow < h; topRow += rowsPerTile) {
    nRowsToConvert = (topRow + rowsPerTile > h ? h - topRow : rowsPerTile);
    for(leftCol = 0; leftCol < w; leftCol += colsPerTile) {
      nColsToConvert = 
          (leftCol + colsPerTile > w ? w - leftCol : colsPerTile);
      if (TIFFReadTile(tfp, tile, leftCol, topRow, 0, 0) < 0
          && stopOnError) {
        free(tile);
        return ERROR;
      } 
	
      tileRowOffset = 0;
      for(i = 0; i < nRowsToConvert; i++) {
        for(j = 0; j < nColsToConvert; j++) {
          for(sample = 0; sample < samplesToLoad; sample++) {
            raster[sample][topRow + i][leftCol + j] =
              tile[(tileRowOffset + j)*samplesPerPixel + sample];
          }
        }
        tileRowOffset   += colsPerTile;
      }
    } 
  }

  free(tile);
  return CORRECT;
}


/**************************************************************

loadSeparateTileData()

**************************************************************/

static int 
loadSeparateTileData(TIFF *tfp, uint32 w, uint32 h, PIXEL ***raster, 
		     uint32 samplesToLoad, uint32 samplesPerPixel)
{
  byte *tile[3];
  uint32 rowsPerTile;
  uint32 colsPerTile;
  uint32 topRow;
  uint32 leftCol;

  uint32 nRowsToConvert;
  uint32 nColsToConvert;

  uint32 tileRowOffset;
  uint32 sample;
  uint32 i, j;

  for(sample = 0; sample < samplesToLoad; sample++) {
    tile[sample] = (byte *) malloc(TIFFTileSize(tfp));
    if (! tile[sample]) {
      fprintf(stderr, 
          "loadContigTileData(): Can't malloc tile[%lu]\n", sample);
      for(i = 0; i < sample; i++) free(tile[i]);
      return ERROR;
    }
  }
  TIFFGetFieldDefaulted(tfp, TIFFTAG_TILELENGTH, &rowsPerTile);
  TIFFGetFieldDefaulted(tfp, TIFFTAG_TILEWIDTH,  &colsPerTile);

  for(topRow = 0; topRow < h; topRow += rowsPerTile) {
    nRowsToConvert = (topRow + rowsPerTile > h ? h - topRow : rowsPerTile);
    for(leftCol = 0; leftCol < w; leftCol += colsPerTile) {
      nColsToConvert = (leftCol + colsPerTile > w ? w - leftCol : colsPerTile);
      for(sample = 0; sample < samplesToLoad; sample++) {
        if (TIFFReadTile(tfp, tile[sample], leftCol, topRow, 0, sample) < 0
            && stopOnError) {
	  for(i = 0; i < samplesToLoad; i++) free(tile[i]);
	    return ERROR;
	} 
      }

      tileRowOffset = 0;
      for(i = 0; i < nRowsToConvert; i++) {
	for(j = 0; j < nColsToConvert; j++) {
	  for(sample = 0; sample < samplesToLoad; sample++) {
            raster[sample][topRow + i][leftCol + j] =
	    tile[sample][tileRowOffset + j];
	  }
	}
	tileRowOffset   += colsPerTile;
      }
    }
  }

  for(sample = 0; sample < samplesToLoad; sample++) free(tile[sample]);
  return CORRECT;
}


/**************************************************************

allocImagePlane()

**************************************************************/

PIXEL **
allocImagePlane(uint32 ulHeight, uint32 ulWidth)
{
  PIXEL **plane;
  uint32 ulImageRow;

  /* Allocate Image data */
  if(!(plane = malloc(ulHeight * sizeof(PIXEL *)))) {
    free(plane);
    return NULL;
  }

  if(!(plane[0] = calloc(ulHeight * ulWidth, sizeof(PIXEL)))) {
    free(plane);
    return NULL;
  }

  /* Make all pointers point to the correct spot in the image */
  for(ulImageRow = 0; ulImageRow < ulHeight; ulImageRow++) {
    plane[ulImageRow] = plane[0] + ulImageRow * ulWidth;
  }

  return(plane);
}


/**************************************************************

allocImageData()

**************************************************************/

PIXEL ***
allocImageData(uint32 ulPlanes, uint32 ulHeight, uint32 ulWidth)
{
  PIXEL ***data;
  uint32 ulPlaneNum;

  /* Allocation all memory needed */

  if(!(data = malloc(ulPlanes * sizeof(PIXEL **)))) {
    return NULL;
  }

  /* Instead, we now alloc one huge chunk of memory, and then */
  /* set the plane pointers appropriately */
  data[0] = allocImagePlane(ulHeight * ulPlanes, ulWidth);
  if (data[0] == NULL)
    return NULL;

  for(ulPlaneNum = 1; ulPlaneNum < ulPlanes; ulPlaneNum++)
    data[ulPlaneNum] = &data[0][ulPlaneNum * ulHeight];

  return(data);
}


/**************************************************************

freeImagePlane()

**************************************************************/

void 
freeImagePlane(PIXEL **plane)
{
  if (plane == NULL)
    return;

  if (plane[0])
    free(plane[0]);

  free(plane);
}


/**************************************************************

freeImageData()

**************************************************************/

void 
freeImageData(PIXEL ***data, uint32 ulPlanes)
{
  uint32 ulImagePlane;

  if (data == NULL)
    return;

  /* All the planes are now allocated together.  See allocImage.c */
  freeImagePlane(data[0]);

  free(data);
}


/**************************************************************

loadTiff()

**************************************************************/

int loadtiff(char *path, unsigned char *pixels, int *ih, int *iw) {
	int i;
	IMAGE image;
	
	if (loadTiffStatic(path, &image) == ERROR)
		return ERROR;
	
	/* Not efficient, but this is just a tutorial, and the focus is on
	MPI, not on loading TIFF images nicely...  Cheap shot on the
	memcpy, because we know it will be allocated contiguously. */
	
	*ih = image.h;
	*iw = image.w;
	memcpy(pixels, image.data[0][0], image.w * image.h);
	freeImageData(image.data, image.p);
	
	return CORRECT;
}

/*
 * Fortran wrappers 
 */

int loadtiff_(char *path, unsigned char *pixels, int *ih, int *iw, int len) {
	char filename[256];
	strncpy(filename, path, len);
	filename[len] = '\0';
	return loadtiff(filename, pixels, ih, iw);
}

int loadtiff__(char *path, unsigned char *pixels, int *ih, int *iw, int len) {
	char filename[256];
	strncpy(filename, path, len);
	filename[len] = '\0';
	return loadtiff(filename, pixels, ih, iw);
}

int LOADTIFF(char *path, unsigned char *pixels, int *ih, int *iw, int len) {
	char filename[256];
	strncpy(filename, path, len);
	filename[len] = '\0';
	return loadtiff(filename, pixels, ih, iw);
}

/**************************************************************

loadTiffStatic()

**************************************************************/

int loadTiffStatic(char *path, IMAGE *image) {

	TIFF *tfp;
	uint32 h;
	uint32 w;
	
	uint16 bps;
	uint16 pm;
	uint16 spp;
	uint16 orientation;
	uint16 planarConfig;
	uint32 sppDest;
	
	int    fmt;
	PIXEL ***data;
	
	tfp = TIFFOpen(path, "r");
	if (!tfp) 
		return ERROR;
	
	/* Get Format information from the TIFF file */
	
	if (!TIFFGetFieldDefaulted(tfp, TIFFTAG_ORIENTATION, &orientation)) {
		fprintf(stderr, "loadTiff(): Can't get Orientation tag!\n");
		return ERROR;
	}
	
	if (!TIFFGetField(tfp, TIFFTAG_IMAGELENGTH, &h)) {
		fprintf(stderr, "loadTiff(): Can't get tiff ImageLength!\n");
		return ERROR;
	}
	
	if (!TIFFGetFieldDefaulted(tfp, TIFFTAG_IMAGEWIDTH, &w)) {
		fprintf(stderr, "Can't get tiff ImageWidth!\n");
		return ERROR;
	} 
	
	if (!TIFFGetFieldDefaulted(tfp, TIFFTAG_BITSPERSAMPLE, &bps)) {
		fprintf(stderr, "loadTiff() --- Can't get tiff BitsPerSample!\n");
		return ERROR;
	}
	
	if (!TIFFGetField(tfp, TIFFTAG_PHOTOMETRIC, &pm)) {
		fprintf(stderr, "Can't get tiff Photometric!\n");
		return ERROR;
	} 
	
	if (!TIFFGetFieldDefaulted(tfp, TIFFTAG_SAMPLESPERPIXEL, &spp)) {
		fprintf(stderr, "loadTiff(): Can't get SamplesPerPixel tag!\n");
		return ERROR;
	}
	
	fmt = tiffToIOFormat(bps, pm, spp);
	
	/* Weed out all formats that are currently not supported, 
	and convert fmt field */
	
	switch(fmt) {
		case GRAYSCALE_8:		/* Supported Formats */
			sppDest= 1L;
			break;
		case COLOR_RGB_24:
			sppDest = 3L;
			break;
		case BINARY:			/* Unsupported Formats */
		case GRAYSCALE_4:
		case COLOR_PALETTE_4:
		case COLOR_PALETTE_8:
		case COLOR_RGB_48:
			fprintf(stderr, "loadTiff(): Format %d is not currently supported.\n", fmt);
			return ERROR;
		case UNKNOWN_FORMAT:
			fprintf(stderr, "loadTiff(): Illegal Tiff File Parameters !?\n");
			return ERROR;
		default:
			fprintf(stderr, "loadTiff(): This shouldn't happpen.\n");
			abort();
		break;
	}
	
	/* For the time being, only TOPLEFT orientation is fully supported */
	if (orientation != ORIENTATION_TOPLEFT) {
		fprintf(stderr, 
		"Caution: Assumming 0th row is at the top and 0th column is at left\n");
	}
	
	if (! TIFFGetFieldDefaulted(tfp, TIFFTAG_PLANARCONFIG, &planarConfig)) {
		fprintf(stderr, "loadTiff(): Can't get PlanarConfig tag!\n");
		return ERROR;
	}
	
	data = allocImageData(sppDest, h, w);

	if (!data) {
		fprintf(stderr, "loadTiff():  Can't calloc data!\n");
		fprintf(stderr, "  format = %d\n", fmt);
		fprintf(stderr, "  image height = %lu\n", h);
		fprintf(stderr, "  image width  = %lu\n", w);
		return ERROR;
	}
	
	if (planarConfig == PLANARCONFIG_SEPARATE) {
		if (TIFFIsTiled(tfp)) {
			if (loadSeparateTileData(tfp, w, h, data, sppDest, spp) == ERROR) {
				fprintf(stderr, 
				"loadTiff(): Could not parse separated tile data\n");
				freeImageData(data, sppDest);
				return ERROR;
			}	
		} 
		else {
			if (loadSeparateStripData(tfp, w, h, data, sppDest, spp) == ERROR) {
				fprintf(stderr, 
				"loadTiff(): Could not parse separated strip data\n");
				freeImageData(data, sppDest);
				return ERROR;
			} 
		}
	} 
	/*  planarConfig = PLANARCONFIG_CONTIG */
	else {
		if (TIFFIsTiled(tfp)) {
			if (loadContigTileData(tfp, w, h, data, sppDest, spp) == ERROR) {
				fprintf(stderr, 
				"loadTiff(): Could not parse contiguous tile data\n");
				freeImageData(data, sppDest);
				return ERROR;
			}	
		} 
		else {
			if (loadContigStripData(tfp, w, h, data, sppDest, spp) == ERROR) {
				fprintf(stderr, 
				"loadTiff(): Could not parse separated strip data\n");
				freeImageData(data, sppDest );
				return ERROR;
			}
		}
	}
	
	/* Convert to internal format */
	
	image->w    = w;
	image->h    = h;
	image->p    = sppDest;
	image->fmt  = fmt;
	image->data = data;
	
	/* Close the tiff file */
	
	TIFFClose(tfp);
	
	return CORRECT;
}
