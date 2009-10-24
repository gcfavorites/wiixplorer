/*************************************************
*                                                *
*  EasyBMP Cross-Platform Windows Bitmap Library * 
*                                                *
*  Author: Paul Macklin                          *
*   email: macklin01@users.sourceforge.net       *
* support: http://easybmp.sourceforge.net        *
*                                                *
*          file: EasyBMP.cpp                     * 
*    date added: 03-31-2006                      *
* date modified: 12-01-2006                      *
*       version: 1.06                            *
*                                                *
*   License: BSD (revised/modified)              *
* Copyright: 2005-6 by the EasyBMP Project       * 
*                                                *
* description: Actual source file                *
*                                                *
* This file is modified by r-win for compiling   *
* and working on the Wii, for WiiXplorer.        *
*                                                *
*************************************************/

#include <gccore.h>
#include <gctypes.h>
#include <stdio.h>

#include "EasyBMP.h"

#include "Prompts/PromptWindows.h"

/* These functions are defined in EasyBMP_DataStructures.h */

int IntPow( int base, int exponent )
{
	int i;
	int output = 1;
	for( i=0 ; i < exponent ; i++ )
	{ output *= base; }
	return output;
}

BMFH::BMFH()
{
	bfType = 19778;
	bfReserved1 = 0;
	bfReserved2 = 0;
}

void BMFH::SwitchEndianess( void )
{
	bfType = FlipWORD( bfType );
	bfSize = FlipDWORD( bfSize );
	bfReserved1 = FlipWORD( bfReserved1 );
	bfReserved2 = FlipWORD( bfReserved2 );
	bfOffBits = FlipDWORD( bfOffBits );
	return;
}

BMIH::BMIH()
{
	biPlanes = 1;
	biCompression = 0;
	biXPelsPerMeter = DefaultXPelsPerMeter;  
	biYPelsPerMeter = DefaultYPelsPerMeter;
	biClrUsed = 0;
	biClrImportant = 0;
}

void BMIH::SwitchEndianess( void )
{
	biSize = FlipDWORD( biSize );
	biWidth = FlipDWORD( biWidth );
	biHeight = FlipDWORD( biHeight );
	biPlanes = FlipWORD( biPlanes );
	biBitCount = FlipWORD( biBitCount );
	biCompression = FlipDWORD( biCompression );
	biSizeImage = FlipDWORD( biSizeImage );
	biXPelsPerMeter = FlipDWORD( biXPelsPerMeter );
	biYPelsPerMeter = FlipDWORD( biYPelsPerMeter );
	biClrUsed = FlipDWORD( biClrUsed );
	biClrImportant = FlipDWORD( biClrImportant );
	return;
}

/* These functions are defined in EasyBMP_BMP.h */

RGBApixel BMP::GetPixel( int i, int j ) const
{
	if( i >= Width )
	{ i = Width-1; }
	if( i < 0 )
	{ i = 0; }
	if( j >= Height )
	{ j = Height-1; }
	if( j < 0 )
	{ j = 0; }
	return Pixels[i][j];
}

bool BMP::SetPixel( int i, int j, RGBApixel NewPixel )
{
	Pixels[i][j] = NewPixel;
	return true;
}


bool BMP::SetColor( int ColorNumber , RGBApixel NewColor )
{
	if( BitDepth != 1 && BitDepth != 4 && BitDepth != 8 )
	{
		return false;
	}
	if( !Colors )
	{
		return false; 
	}
	if( ColorNumber >= TellNumberOfColors() )
	{
		return false;
	}
	Colors[ColorNumber] = NewColor;
	return true;
}

// RGBApixel BMP::GetColor( int ColorNumber ) const
RGBApixel BMP::GetColor( int ColorNumber )
{ 
	RGBApixel Output;
	Output.Red   = 255;
	Output.Green = 255;
	Output.Blue  = 255;
	Output.Alpha = 0;

	if( BitDepth != 1 && BitDepth != 4 && BitDepth != 8 )
	{
		return Output;
	}
	if( !Colors )
	{
		return Output; 
	}
	if( ColorNumber >= TellNumberOfColors() )
	{
		return Output;
	}
	Output = Colors[ColorNumber];
	return Output;
}

BMP::BMP()
{
	Width = 1;
	Height = 1;
	BitDepth = 24;
	Pixels = new RGBApixel* [Width];
	Pixels[0] = new RGBApixel [Height];
	Colors = NULL;

	XPelsPerMeter = 0;
	YPelsPerMeter = 0;

	MetaData1 = NULL;
	SizeOfMetaData1 = 0;
	MetaData2 = NULL;
	SizeOfMetaData2 = 0;
}

// BMP::BMP( const BMP& Input )
BMP::BMP( BMP& Input )
{
	// first, make the image empty.

	Width = 1;
	Height = 1;
	BitDepth = 24;
	Pixels = new RGBApixel* [Width];
	Pixels[0] = new RGBApixel [Height];
	Colors = NULL; 
	XPelsPerMeter = 0;
	YPelsPerMeter = 0;

	MetaData1 = NULL;
	SizeOfMetaData1 = 0;
	MetaData2 = NULL;
	SizeOfMetaData2 = 0;

	// now, set the correct bit depth

	SetBitDepth( Input.TellBitDepth() );

	// set the correct pixel size 

	SetSize( Input.TellWidth() , Input.TellHeight() );

	// set the DPI information from Input

	SetDPI( Input.TellHorizontalDPI() , Input.TellVerticalDPI() );

	// if there is a color table, get all the colors

	if( BitDepth == 1 || BitDepth == 4 ||  
		BitDepth == 8 )
	{
		for( int k=0 ; k < TellNumberOfColors() ; k++ )
		{
			SetColor( k, Input.GetColor( k )); 
		}
	}

	// get all the pixels 

	for( int j=0; j < Height ; j++ )
	{
		for( int i=0; i < Width ; i++ )
		{
			Pixels[i][j] = *Input(i,j);
			//   Pixels[i][j] = Input.GetPixel(i,j); // *Input(i,j);
		}
	}
}

BMP::~BMP()
{
	int i;
	for(i=0;i<Width;i++)
	{ delete [] Pixels[i]; }
	delete [] Pixels;
	if( Colors )
	{ delete [] Colors; }

	if( MetaData1 )
	{ delete [] MetaData1; }
	if( MetaData2 )
	{ delete [] MetaData2; }
} 

RGBApixel* BMP::operator()(int i, int j)
{
	if( i >= Width )
	{ i = Width-1; }
	if( i < 0 )
	{ i = 0; }
	if( j >= Height )
	{ j = Height-1; }
	if( j < 0 )
	{ j = 0; }
	return &(Pixels[i][j]);
}

// int BMP::TellBitDepth( void ) const
int BMP::TellBitDepth( void )
{ return BitDepth; }

// int BMP::TellHeight( void ) const
int BMP::TellHeight( void )
{ return Height; }

// int BMP::TellWidth( void ) const
int BMP::TellWidth( void )
{ return Width; }

// int BMP::TellNumberOfColors( void ) const
int BMP::TellNumberOfColors( void )
{
	int output = IntPow( 2, BitDepth );
	if( BitDepth == 32 )
	{ output = IntPow( 2, 24 ); }
	return output;
}

bool BMP::SetBitDepth( int NewDepth )
{
	if( NewDepth != 1 && NewDepth != 4 && 
		NewDepth != 8 && NewDepth != 16 && 
		NewDepth != 24 && NewDepth != 32 )
	{
		return false;
	}

	BitDepth = NewDepth;
	if( Colors )
	{ delete [] Colors; }
	int NumberOfColors = IntPow( 2, BitDepth );
	if( BitDepth == 1 || BitDepth == 4 || BitDepth == 8 )
	{ Colors = new RGBApixel [NumberOfColors]; }
	else
	{ Colors = NULL; } 
	if( BitDepth == 1 || BitDepth == 4 || BitDepth == 8 )
	{ CreateStandardColorTable(); }

	return true;
}

bool BMP::SetSize(int NewWidth , int NewHeight )
{
	if( NewWidth <= 0 || NewHeight <= 0 )
	{
		return false;
	}

	int i,j; 

	for(i=0;i<Width;i++)
	{ delete [] Pixels[i]; }
	delete [] Pixels;

	Width = NewWidth;
	Height = NewHeight;
	Pixels = new RGBApixel* [ Width ]; 

	for(i=0; i<Width; i++)
	{ Pixels[i] = new RGBApixel [ Height ]; }

	for( i=0 ; i < Width ; i++)
	{
		for( j=0 ; j < Height ; j++ )
		{
			Pixels[i][j].Red = 255; 
			Pixels[i][j].Green = 255; 
			Pixels[i][j].Blue = 255; 
			Pixels[i][j].Alpha = 0;    
		}
	}

	return true; 
}

bool BMP::WriteToFile( const char* FileName )
{
	FILE* fp = fopen( FileName, "wb" );
	if( fp == NULL )
	{
		fclose( fp );
		return false;
	}

	// some preliminaries

	double dBytesPerPixel = ( (double) BitDepth ) / 8.0;
	double dBytesPerRow = dBytesPerPixel * (Width+0.0);
	dBytesPerRow = ceil(dBytesPerRow);

	int BytePaddingPerRow = 4 - ( (int) (dBytesPerRow) )% 4;
	if( BytePaddingPerRow == 4 )
	{ BytePaddingPerRow = 0; } 

	double dActualBytesPerRow = dBytesPerRow + BytePaddingPerRow;

	double dTotalPixelBytes = Height * dActualBytesPerRow;

	double dPaletteSize = 0;
	if( BitDepth == 1 || BitDepth == 4 || BitDepth == 8 )
	{ dPaletteSize = IntPow(2,BitDepth)*4.0; }

	// leave some room for 16-bit masks 
	if( BitDepth == 16 )
	{ dPaletteSize = 3*4; }

	double dTotalFileSize = 14 + 40 + dPaletteSize + dTotalPixelBytes;

	// write the file header 

	BMFH bmfh;
	bmfh.bfSize = (ebmpDWORD) dTotalFileSize; 
	bmfh.bfReserved1 = 0; 
	bmfh.bfReserved2 = 0; 
	bmfh.bfOffBits = (ebmpDWORD) (14+40+dPaletteSize);  

	if( IsBigEndian() )
	{ bmfh.SwitchEndianess(); }

	fwrite( (char*) &(bmfh.bfType) , sizeof(ebmpWORD) , 1 , fp );
	fwrite( (char*) &(bmfh.bfSize) , sizeof(ebmpDWORD) , 1 , fp );
	fwrite( (char*) &(bmfh.bfReserved1) , sizeof(ebmpWORD) , 1 , fp );
	fwrite( (char*) &(bmfh.bfReserved2) , sizeof(ebmpWORD) , 1 , fp );
	fwrite( (char*) &(bmfh.bfOffBits) , sizeof(ebmpDWORD) , 1 , fp );

	// write the info header 

	BMIH bmih;
	bmih.biSize = 40;
	bmih.biWidth = Width;
	bmih.biHeight = Height;
	bmih.biPlanes = 1;
	bmih.biBitCount = BitDepth;
	bmih.biCompression = 0;
	bmih.biSizeImage = (ebmpDWORD) dTotalPixelBytes;
	if( XPelsPerMeter )
	{ bmih.biXPelsPerMeter = XPelsPerMeter; }
	else
	{ bmih.biXPelsPerMeter = DefaultXPelsPerMeter; }
	if( YPelsPerMeter )
	{ bmih.biYPelsPerMeter = YPelsPerMeter; }
	else
	{ bmih.biYPelsPerMeter = DefaultYPelsPerMeter; }

	bmih.biClrUsed = 0;
	bmih.biClrImportant = 0;

	// indicates that we'll be using bit fields for 16-bit files
	if( BitDepth == 16 )
	{ bmih.biCompression = 3; }

	if( IsBigEndian() )
	{ bmih.SwitchEndianess(); }

	fwrite( (char*) &(bmih.biSize) , sizeof(ebmpDWORD) , 1 , fp );
	fwrite( (char*) &(bmih.biWidth) , sizeof(ebmpDWORD) , 1 , fp );
	fwrite( (char*) &(bmih.biHeight) , sizeof(ebmpDWORD) , 1 , fp );
	fwrite( (char*) &(bmih.biPlanes) , sizeof(ebmpWORD) , 1 , fp );
	fwrite( (char*) &(bmih.biBitCount) , sizeof(ebmpWORD) , 1 , fp );
	fwrite( (char*) &(bmih.biCompression) , sizeof(ebmpDWORD) , 1 , fp );
	fwrite( (char*) &(bmih.biSizeImage) , sizeof(ebmpDWORD) , 1 , fp );
	fwrite( (char*) &(bmih.biXPelsPerMeter) , sizeof(ebmpDWORD) , 1 , fp );
	fwrite( (char*) &(bmih.biYPelsPerMeter) , sizeof(ebmpDWORD) , 1 , fp ); 
	fwrite( (char*) &(bmih.biClrUsed) , sizeof(ebmpDWORD) , 1 , fp);
	fwrite( (char*) &(bmih.biClrImportant) , sizeof(ebmpDWORD) , 1 , fp);

	// write the palette 
	if( BitDepth == 1 || BitDepth == 4 || BitDepth == 8 )
	{
		int NumberOfColors = IntPow(2,BitDepth);

		// if there is no palette, create one 
		if( !Colors )
		{
			if( !Colors )
			{ Colors = new RGBApixel [NumberOfColors]; }
			CreateStandardColorTable(); 
		}

		int n;
		for( n=0 ; n < NumberOfColors ; n++ )
		{ fwrite( (char*) &(Colors[n]) , 4 , 1 , fp ); }
	}

	// write the pixels 
	int i,j;
	if( BitDepth != 16 )
	{  
		ebmpBYTE* Buffer;
		int BufferSize = (int) ( (Width*BitDepth)/8.0 );
		while( 8*BufferSize < Width*BitDepth )
		{ BufferSize++; }
		while( BufferSize % 4 )
		{ BufferSize++; }

		Buffer = new ebmpBYTE [BufferSize];
		for( j=0 ; j < BufferSize; j++ )
		{ Buffer[j] = 0; }

		j=Height-1;

		while( j > -1 )
		{
			bool Success = false;
			if( BitDepth == 32 )
			{ Success = Write32bitRow( Buffer, BufferSize, j ); }
			if( BitDepth == 24 )
			{ Success = Write24bitRow( Buffer, BufferSize, j ); }
			if( BitDepth == 8  )
			{ Success = Write8bitRow( Buffer, BufferSize, j ); }
			if( BitDepth == 4  )
			{ Success = Write4bitRow( Buffer, BufferSize, j ); }
			if( BitDepth == 1  )
			{ Success = Write1bitRow( Buffer, BufferSize, j ); }
			if( Success )
			{
				int BytesWritten = (int) fwrite( (char*) Buffer, 1, BufferSize, fp );
				if( BytesWritten != BufferSize )
				{ Success = false; }
			}
			if( !Success )
			{
				j = -1; 
			}
			j--; 
		}

		delete [] Buffer;
	}

	if( BitDepth == 16 )
	{
		// write the bit masks

		ebmpWORD BlueMask = 31;    // bits 12-16
		ebmpWORD GreenMask = 2016; // bits 6-11
		ebmpWORD RedMask = 63488;  // bits 1-5
		ebmpWORD ZeroWORD;

		if( IsBigEndian() )
		{ RedMask = FlipWORD( RedMask ); }
		fwrite( (char*) &RedMask , 2 , 1 , fp );
		fwrite( (char*) &ZeroWORD , 2 , 1 , fp );

		if( IsBigEndian() )
		{ GreenMask = FlipWORD( GreenMask ); }
		fwrite( (char*) &GreenMask , 2 , 1 , fp );
		fwrite( (char*) &ZeroWORD , 2 , 1 , fp );

		if( IsBigEndian() )
		{ BlueMask = FlipWORD( BlueMask ); }
		fwrite( (char*) &BlueMask , 2 , 1 , fp );
		fwrite( (char*) &ZeroWORD , 2 , 1 , fp );

		int DataBytes = Width*2;
		int PaddingBytes = ( 4 - DataBytes % 4 ) % 4;

		// write the actual pixels

		for( j=Height-1 ; j >= 0 ; j-- )
		{
			// write all row pixel data
			i=0;
			int WriteNumber = 0;
			while( WriteNumber < DataBytes )
			{
				ebmpWORD TempWORD;

				ebmpWORD RedWORD = (ebmpWORD) ((Pixels[i][j]).Red / 8);
				ebmpWORD GreenWORD = (ebmpWORD) ((Pixels[i][j]).Green / 4);
				ebmpWORD BlueWORD = (ebmpWORD) ((Pixels[i][j]).Blue / 8);

				TempWORD = (RedWORD<<11) + (GreenWORD<<5) + BlueWORD;
				if( IsBigEndian() )
				{ TempWORD = FlipWORD( TempWORD ); }

				fwrite( (char*) &TempWORD , 2, 1, fp);
				WriteNumber += 2;
				i++;
			}
			// write any necessary row padding
			WriteNumber = 0;
			while( WriteNumber < PaddingBytes )
			{
				ebmpBYTE TempBYTE;
				fwrite( (char*) &TempBYTE , 1, 1, fp);
				WriteNumber++;
			}
		}

	}

	fclose(fp);
	return true;
}

bool BMP::ReadFromFile( const char* FileName )
{ 
	FILE* fp = fopen( FileName, "rb" );
	if( fp == NULL )
	{
		SetBitDepth(1);
		SetSize(1,1);
		return false;
	}

	struct BMPFile bmpFile;
	memset(&bmpFile, 0, sizeof(BMPFile));
	bmpFile.fp = fp;

	bool retval = Read(&bmpFile);
	fclose(fp);
	return retval;
}

bool BMP::ReadFromMem( const unsigned char* img, int imgSize )
{
	struct BMPFile bmpFile;
	memset(&bmpFile, 0, sizeof(BMPFile));
	bmpFile.imgData = img;
	bmpFile.imgSize = imgSize;

	return Read(&bmpFile);
}

bool BMP::Read(struct BMPFile *bmpFile)
{
	// read the file header 

	BMFH bmfh;
	bool NotCorrupted = true;
	
	NotCorrupted &= SafeFread( (char*) &(bmfh.bfType) , sizeof(ebmpWORD), 1, bmpFile);

	bool IsBitmap = false;

	if( IsBigEndian() && bmfh.bfType == 16973 )
	{ IsBitmap = true; }
	if( !IsBigEndian() && bmfh.bfType == 19778 )
	{ IsBitmap = true; }

	if( !IsBitmap ) 
	{
		return false;
	}

	NotCorrupted &= SafeFread( (char*) &(bmfh.bfSize) , sizeof(ebmpDWORD) , 1, bmpFile); 
	NotCorrupted &= SafeFread( (char*) &(bmfh.bfReserved1) , sizeof(ebmpWORD) , 1, bmpFile);
	NotCorrupted &= SafeFread( (char*) &(bmfh.bfReserved2) , sizeof(ebmpWORD) , 1, bmpFile);
	NotCorrupted &= SafeFread( (char*) &(bmfh.bfOffBits) , sizeof(ebmpDWORD) , 1 , bmpFile);

	if( IsBigEndian() ) 
	{ bmfh.SwitchEndianess(); }

	// read the info header

	BMIH bmih; 

	NotCorrupted &= SafeFread( (char*) &(bmih.biSize) , sizeof(ebmpDWORD) , 1 , bmpFile);
	NotCorrupted &= SafeFread( (char*) &(bmih.biWidth) , sizeof(ebmpDWORD) , 1 , bmpFile); 
	NotCorrupted &= SafeFread( (char*) &(bmih.biHeight) , sizeof(ebmpDWORD) , 1 , bmpFile);
	NotCorrupted &= SafeFread( (char*) &(bmih.biPlanes) , sizeof(ebmpWORD) , 1, bmpFile); 
	NotCorrupted &= SafeFread( (char*) &(bmih.biBitCount) , sizeof(ebmpWORD) , 1, bmpFile);

	NotCorrupted &= SafeFread( (char*) &(bmih.biCompression) , sizeof(ebmpDWORD) , 1 , bmpFile);
	NotCorrupted &= SafeFread( (char*) &(bmih.biSizeImage) , sizeof(ebmpDWORD) , 1 , bmpFile);
	NotCorrupted &= SafeFread( (char*) &(bmih.biXPelsPerMeter) , sizeof(ebmpDWORD) , 1 , bmpFile);
	NotCorrupted &= SafeFread( (char*) &(bmih.biYPelsPerMeter) , sizeof(ebmpDWORD) , 1 , bmpFile);
	NotCorrupted &= SafeFread( (char*) &(bmih.biClrUsed) , sizeof(ebmpDWORD) , 1 , bmpFile);
	NotCorrupted &= SafeFread( (char*) &(bmih.biClrImportant) , sizeof(ebmpDWORD) , 1 , bmpFile);

	if( IsBigEndian() ) 
	{ bmih.SwitchEndianess(); }

	// a safety catch: if any of the header information didn't read properly, abort
	// future idea: check to see if at least most is self-consistent

	if( !NotCorrupted )
	{
		SetSize(1,1);
		SetBitDepth(1);
		return false;
	} 

	XPelsPerMeter = bmih.biXPelsPerMeter;
	YPelsPerMeter = bmih.biYPelsPerMeter;

	// if bmih.biCompression 1 or 2, then the file is RLE compressed

	if( bmih.biCompression == 1 || bmih.biCompression == 2 )
	{
		SetSize(1,1);
		SetBitDepth(1);
		return false; 
	}

	// if bmih.biCompression > 3, then something strange is going on 
	// it's probably an OS2 bitmap file.

	if( bmih.biCompression > 3 )
	{
		SetSize(1,1);
		SetBitDepth(1);
		return false; 
	}

	if( bmih.biCompression == 3 && bmih.biBitCount != 16 )
	{
		SetSize(1,1);
		SetBitDepth(1);
		return false; 
	}

	// set the bit depth

	int TempBitDepth = (int) bmih.biBitCount;
	if(    TempBitDepth != 1  && TempBitDepth != 4 
		&& TempBitDepth != 8  && TempBitDepth != 16
		&& TempBitDepth != 24 && TempBitDepth != 32 )
	{
		SetSize(1,1);
		SetBitDepth(1);
		return false;
	}
	SetBitDepth( (int) bmih.biBitCount ); 
	
	// set the size

	if( (int) bmih.biWidth <= 0 || (int) bmih.biHeight <= 0 ) 
	{
		SetSize(1,1);
		SetBitDepth(1);
		return false;
	} 
	SetSize( (int) bmih.biWidth , (int) bmih.biHeight );

	// some preliminaries

	double dBytesPerPixel = ( (double) BitDepth ) / 8.0;
	double dBytesPerRow = dBytesPerPixel * (Width+0.0);
	dBytesPerRow = ceil(dBytesPerRow);

	int BytePaddingPerRow = 4 - ( (int) (dBytesPerRow) )% 4;
	if( BytePaddingPerRow == 4 )
	{ BytePaddingPerRow = 0; }  

	// if < 16 bits, read the palette

	if( BitDepth < 16 )
	{
		// determine the number of colors specified in the 
		// color table

		int NumberOfColorsToRead = ((int) bmfh.bfOffBits - 54 )/4;  
		if( NumberOfColorsToRead > IntPow(2,BitDepth) )
		{ NumberOfColorsToRead = IntPow(2,BitDepth); }

		int n;
		for( n=0; n < NumberOfColorsToRead ; n++ )
		{
			SafeFread( (char*) &(Colors[n]) , 4 , 1 , bmpFile);     
		}
		for( n=NumberOfColorsToRead ; n < TellNumberOfColors() ; n++ )
		{
			RGBApixel WHITE; 
			WHITE.Red = 255;
			WHITE.Green = 255;
			WHITE.Blue = 255;
			WHITE.Alpha = 0;
			SetColor( n , WHITE );
		}
	}

	// skip blank data if bfOffBits so indicates

	int BytesToSkip = bmfh.bfOffBits - 54;;
	if( BitDepth < 16 )
	{ BytesToSkip -= 4*IntPow(2,BitDepth); }
	if( BitDepth == 16 && bmih.biCompression == 3 )
	{ BytesToSkip -= 3*4; }
	if( BytesToSkip < 0 )
	{ BytesToSkip = 0; }
	if( BytesToSkip > 0 && BitDepth != 16 )
	{
		ebmpBYTE* TempSkipBYTE;
		TempSkipBYTE = new ebmpBYTE [BytesToSkip];
		SafeFread( (char*) TempSkipBYTE , BytesToSkip , 1 , bmpFile);   
		delete [] TempSkipBYTE;
	} 

	// This code reads 1, 4, 8, 24, and 32-bpp files 
	// with a more-efficient buffered technique.

	int i,j;
	if( BitDepth != 16 )
	{
		int BufferSize = (int) ( (Width*BitDepth) / 8.0 );
		while( 8*BufferSize < Width*BitDepth )
		{ BufferSize++; }
		while( BufferSize % 4 )
		{ BufferSize++; }
		ebmpBYTE* Buffer;
		Buffer = new ebmpBYTE [BufferSize];
		j= Height-1;
		while( j > -1 )
		{
			//   int BytesRead = (int) fread( (char*) Buffer, 1, BufferSize, bmpFile->fp );
			if(!SafeFread( (char*) Buffer, 1, BufferSize, bmpFile))
			{
				j = -1; 
			}
			else
			{
				bool Success = false;
				if( BitDepth == 1  )
				{ Success = Read1bitRow(  Buffer, BufferSize, j ); }
				if( BitDepth == 4  )
				{ Success = Read4bitRow(  Buffer, BufferSize, j ); }
				if( BitDepth == 8  )
				{ Success = Read8bitRow(  Buffer, BufferSize, j ); }
				if( BitDepth == 24 )
				{ Success = Read24bitRow( Buffer, BufferSize, j ); }
				if( BitDepth == 32 )
				{ Success = Read32bitRow( Buffer, BufferSize, j ); }
				if( !Success )
				{
					j = -1;
				}
			}   
			j--;
		}
		delete [] Buffer; 
	}

	if( BitDepth == 16 )
	{
		int DataBytes = Width*2;
		int PaddingBytes = ( 4 - DataBytes % 4 ) % 4;

		// set the default mask

		ebmpWORD BlueMask = 31; // bits 12-16
		ebmpWORD GreenMask = 992; // bits 7-11
		ebmpWORD RedMask = 31744; // bits 2-6

		// read the bit fields, if necessary, to 
		// override the default 5-5-5 mask

		if( bmih.biCompression != 0 )
		{
			// read the three bit masks

			ebmpWORD TempMaskWORD;
			//   ebmpWORD ZeroWORD;

			SafeFread( (char*) &RedMask , 2 , 1 , bmpFile );
			if( IsBigEndian() )
			{ RedMask = FlipWORD(RedMask); }
			SafeFread( (char*) &TempMaskWORD , 2, 1, bmpFile );

			SafeFread( (char*) &GreenMask , 2 , 1 , bmpFile );
			if( IsBigEndian() )
			{ GreenMask = FlipWORD(GreenMask); }
			SafeFread( (char*) &TempMaskWORD , 2, 1, bmpFile );

			SafeFread( (char*) &BlueMask , 2 , 1 , bmpFile );
			if( IsBigEndian() )
			{ BlueMask = FlipWORD(BlueMask); }
			SafeFread( (char*) &TempMaskWORD , 2, 1, bmpFile );
		}

		// read and skip any meta data

		if( BytesToSkip > 0 )
		{
			ebmpBYTE* TempSkipBYTE;
			TempSkipBYTE = new ebmpBYTE [BytesToSkip];
			SafeFread( (char*) TempSkipBYTE , BytesToSkip , 1 , bmpFile);
			delete [] TempSkipBYTE;   
		} 

		// determine the red, green and blue shifts

		int GreenShift = 0; 
		ebmpWORD TempShiftWORD = GreenMask;
		while( TempShiftWORD > 31 )
		{ TempShiftWORD = TempShiftWORD>>1; GreenShift++; }  
		int BlueShift = 0;
		TempShiftWORD = BlueMask;
		while( TempShiftWORD > 31 )
		{ TempShiftWORD = TempShiftWORD>>1; BlueShift++; }  
		int RedShift = 0;  
		TempShiftWORD = RedMask;
		while( TempShiftWORD > 31 )
		{ TempShiftWORD = TempShiftWORD>>1; RedShift++; }  

		// read the actual pixels

		for( j=Height-1 ; j >= 0 ; j-- )
		{
			i=0;
			int ReadNumber = 0;
			while( ReadNumber < DataBytes )
			{
				ebmpWORD TempWORD;
				SafeFread( (char*) &TempWORD , 2 , 1 , bmpFile );
				if( IsBigEndian() )
				{ TempWORD = FlipWORD(TempWORD); }
				ReadNumber += 2;

				ebmpWORD Red = RedMask & TempWORD;
				ebmpWORD Green = GreenMask & TempWORD;
				ebmpWORD Blue = BlueMask & TempWORD;

				ebmpBYTE BlueBYTE = (ebmpBYTE) 8*(Blue>>BlueShift);
				ebmpBYTE GreenBYTE = (ebmpBYTE) 8*(Green>>GreenShift);
				ebmpBYTE RedBYTE = (ebmpBYTE) 8*(Red>>RedShift);

				(Pixels[i][j]).Red = RedBYTE;
				(Pixels[i][j]).Green = GreenBYTE;
				(Pixels[i][j]).Blue = BlueBYTE;

				i++;
			}
			ReadNumber = 0;
			while( ReadNumber < PaddingBytes )
			{
				ebmpBYTE TempBYTE;
				SafeFread( (char*) &TempBYTE , 1, 1, bmpFile);
				ReadNumber++;
			}
		}

	}

	return true;
}

bool BMP::CreateStandardColorTable( void )
{
	if( BitDepth != 1 && BitDepth != 4 && BitDepth != 8 )
	{
		return false;
	}

	if( BitDepth == 1 )
	{
		int i;
		for( i=0 ; i < 2 ; i++ )
		{
			Colors[i].Red = i*255;
			Colors[i].Green = i*255;
			Colors[i].Blue = i*255;
			Colors[i].Alpha = 0;
		} 
		return true;
	} 

	if( BitDepth == 4 )
	{
		int i = 0;
		int j,k,ell;

		// simplify the code for the first 8 colors
		for( ell=0 ; ell < 2 ; ell++ )
		{
			for( k=0 ; k < 2 ; k++ )
			{
				for( j=0 ; j < 2 ; j++ )
				{
					Colors[i].Red = j*128; 
					Colors[i].Green = k*128;
					Colors[i].Blue = ell*128;
					i++;
				}
			}
		}

		// simplify the code for the last 8 colors
		for( ell=0 ; ell < 2 ; ell++ )
		{
			for( k=0 ; k < 2 ; k++ )
			{
				for( j=0 ; j < 2 ; j++ )
				{
					Colors[i].Red = j*255;
					Colors[i].Green = k*255; 
					Colors[i].Blue = ell*255;
					i++;
				}
			}
		}

		// overwrite the duplicate color
		i=8; 
		Colors[i].Red = 192;
		Colors[i].Green = 192;
		Colors[i].Blue = 192;

		for( i=0 ; i < 16 ; i++ )
		{ Colors[i].Alpha = 0; }
		return true;
	}

	if( BitDepth == 8 )
	{
		int i=0;
		int j,k,ell;

		// do an easy loop, which works for all but colors 
		// 0 to 9 and 246 to 255
		for( ell=0 ; ell < 4 ; ell++ ) 
		{
			for( k=0 ; k < 8 ; k++ )
			{
				for( j=0; j < 8 ; j++ )
				{
					Colors[i].Red = j*32; 
					Colors[i].Green = k*32;
					Colors[i].Blue = ell*64;
					Colors[i].Alpha = 0;
					i++;
				}
			}
		} 

		// now redo the first 8 colors  
		i=0;
		for( ell=0 ; ell < 2 ; ell++ ) 
		{
			for( k=0 ; k < 2 ; k++ )
			{
				for( j=0; j < 2 ; j++ )
				{
					Colors[i].Red = j*128;
					Colors[i].Green = k*128;
					Colors[i].Blue = ell*128;
					i++;
				}
			}
		} 

		// overwrite colors 7, 8, 9
		i=7;
		Colors[i].Red = 192;
		Colors[i].Green = 192;
		Colors[i].Blue = 192;
		i++; // 8
		Colors[i].Red = 192;
		Colors[i].Green = 220;
		Colors[i].Blue = 192;
		i++; // 9
		Colors[i].Red = 166;
		Colors[i].Green = 202;
		Colors[i].Blue = 240;

		// overwrite colors 246 to 255 
		i=246;
		Colors[i].Red = 255;
		Colors[i].Green = 251;
		Colors[i].Blue = 240;
		i++; // 247
		Colors[i].Red = 160;
		Colors[i].Green = 160;
		Colors[i].Blue = 164;
		i++; // 248
		Colors[i].Red = 128;
		Colors[i].Green = 128;
		Colors[i].Blue = 128;
		i++; // 249
		Colors[i].Red = 255;
		Colors[i].Green = 0;
		Colors[i].Blue = 0;
		i++; // 250
		Colors[i].Red = 0;
		Colors[i].Green = 255;
		Colors[i].Blue = 0;
		i++; // 251
		Colors[i].Red = 255;
		Colors[i].Green = 255;
		Colors[i].Blue = 0;
		i++; // 252
		Colors[i].Red = 0;
		Colors[i].Green = 0;
		Colors[i].Blue = 255;
		i++; // 253
		Colors[i].Red = 255;
		Colors[i].Green = 0;
		Colors[i].Blue = 255;
		i++; // 254
		Colors[i].Red = 0;
		Colors[i].Green = 255;
		Colors[i].Blue = 255;
		i++; // 255
		Colors[i].Red = 255;
		Colors[i].Green = 255;
		Colors[i].Blue = 255;

		return true;
	}
	return true;
}

bool SafeFread( char* buffer, int size, int number, struct BMPFile *bmpFile )
{
	if (bmpFile->fp) {
		int ItemsRead;
		if( feof(bmpFile->fp) )
		{ return false; }
		ItemsRead = (int) fread( buffer , size , number , bmpFile->fp );
		if( ItemsRead < number )
		{ return false; }
		return true;
	} else {
		int amount = number * size;
		
		bool retval = true;
		if (amount + bmpFile->currentPosition > bmpFile->imgSize) {
			retval = false;
			amount = bmpFile->imgSize - bmpFile->currentPosition;
		}

		void *start = (void *) (bmpFile->imgData + bmpFile->currentPosition);
		memcpy(buffer, start, amount);
		bmpFile->currentPosition += amount;
		return retval;
	}
}

void BMP::SetDPI( int HorizontalDPI, int VerticalDPI )
{
	XPelsPerMeter = (int) ( HorizontalDPI * 39.37007874015748 );
	YPelsPerMeter = (int) (   VerticalDPI * 39.37007874015748 );
}

// int BMP::TellVerticalDPI( void ) const
int BMP::TellVerticalDPI( void )
{
	if( !YPelsPerMeter )
	{ YPelsPerMeter = DefaultYPelsPerMeter; }
	return (int) ( YPelsPerMeter / (double) 39.37007874015748 ); 
}

// int BMP::TellHorizontalDPI( void ) const
int BMP::TellHorizontalDPI( void )
{
	if( !XPelsPerMeter )
	{ XPelsPerMeter = DefaultXPelsPerMeter; }
	return (int) ( XPelsPerMeter / (double) 39.37007874015748 );
}

bool BMP::Read32bitRow( ebmpBYTE* Buffer, int BufferSize, int Row )
{ 
 int i;
 if( Width*4 > BufferSize )
 { return false; }
 for( i=0 ; i < Width ; i++ )
 { memcpy( (char*) &(Pixels[i][Row]), (char*) Buffer+4*i, 4 ); }
 return true;
}

bool BMP::Read24bitRow( ebmpBYTE* Buffer, int BufferSize, int Row )
{ 
 int i;
 if( Width*3 > BufferSize )
 { return false; }
 for( i=0 ; i < Width ; i++ )
 { memcpy( (char*) &(Pixels[i][Row]), Buffer+3*i, 3 ); }
 return true;
}

bool BMP::Read8bitRow(  ebmpBYTE* Buffer, int BufferSize, int Row )
{
 int i;
 if( Width > BufferSize )
 { return false; }
 for( i=0 ; i < Width ; i++ )
 {
  int Index = Buffer[i];
  *( this->operator()(i,Row) )= GetColor(Index); 
 }
 return true;
}

bool BMP::Read4bitRow(  ebmpBYTE* Buffer, int BufferSize, int Row )
{
 int Shifts[2] = {4  ,0 };
 int Masks[2]  = {240,15};
 
 int i=0;
 int j;
 int k=0;
 if( Width > 2*BufferSize )
 { return false; }
 while( i < Width )
 {
  j=0;
  while( j < 2 && i < Width )
  {
   int Index = (int) ( (Buffer[k]&Masks[j]) >> Shifts[j]);
   *( this->operator()(i,Row) )= GetColor(Index); 
   i++; j++;   
  }
  k++;
 }
 return true;
}
bool BMP::Read1bitRow(  ebmpBYTE* Buffer, int BufferSize, int Row )
{
 int Shifts[8] = {7  ,6 ,5 ,4 ,3,2,1,0};
 int Masks[8]  = {128,64,32,16,8,4,2,1};
 
 int i=0;
 int j;
 int k=0;
 
 if( Width > 8*BufferSize )
 { return false; }
 while( i < Width )
 {
  j=0;
  while( j < 8 && i < Width )
  {
   int Index = (int) ( (Buffer[k]&Masks[j]) >> Shifts[j]);
   *( this->operator()(i,Row) )= GetColor(Index); 
   i++; j++;   
  }
  k++;
 }
 return true;
}

bool BMP::Write32bitRow( ebmpBYTE* Buffer, int BufferSize, int Row )
{ 
 int i;
 if( Width*4 > BufferSize )
 { return false; }
 for( i=0 ; i < Width ; i++ )
 { memcpy( (char*) Buffer+4*i, (char*) &(Pixels[i][Row]), 4 ); }
 return true;
}

bool BMP::Write24bitRow( ebmpBYTE* Buffer, int BufferSize, int Row )
{ 
 int i;
 if( Width*3 > BufferSize )
 { return false; }
 for( i=0 ; i < Width ; i++ )
 { memcpy( (char*) Buffer+3*i,  (char*) &(Pixels[i][Row]), 3 ); }
 return true;
}

bool BMP::Write8bitRow(  ebmpBYTE* Buffer, int BufferSize, int Row )
{
 int i;
 if( Width > BufferSize )
 { return false; }
 for( i=0 ; i < Width ; i++ )
 { Buffer[i] = FindClosestColor( Pixels[i][Row] ); }
 return true;
}

bool BMP::Write4bitRow(  ebmpBYTE* Buffer, int BufferSize, int Row )
{ 
 int PositionWeights[2]  = {16,1};
 
 int i=0;
 int j;
 int k=0;
 if( Width > 2*BufferSize )
 { return false; }
 while( i < Width )
 {
  j=0;
  int Index = 0;
  while( j < 2 && i < Width )
  {
   Index += ( PositionWeights[j]* (int) FindClosestColor( Pixels[i][Row] ) ); 
   i++; j++;   
  }
  Buffer[k] = (ebmpBYTE) Index;
  k++;
 }
 return true;
}

bool BMP::Write1bitRow(  ebmpBYTE* Buffer, int BufferSize, int Row )
{ 
 int PositionWeights[8]  = {128,64,32,16,8,4,2,1};
 
 int i=0;
 int j;
 int k=0;
 if( Width > 8*BufferSize )
 { return false; }
 while( i < Width )
 {
  j=0;
  int Index = 0;
  while( j < 8 && i < Width )
  {
   Index += ( PositionWeights[j]* (int) FindClosestColor( Pixels[i][Row] ) ); 
   i++; j++;   
  }
  Buffer[k] = (ebmpBYTE) Index;
  k++;
 }
 return true;
}

ebmpBYTE BMP::FindClosestColor( RGBApixel& input )
{
 int i=0;
 int NumberOfColors = TellNumberOfColors();
 ebmpBYTE BestI = 0;
 int BestMatch = 999999;
  
 while( i < NumberOfColors )
 {
  RGBApixel Attempt = GetColor( i );
  int TempMatch = IntSquare( (int) Attempt.Red - (int) input.Red )
                + IntSquare( (int) Attempt.Green - (int) input.Green )
                + IntSquare( (int) Attempt.Blue - (int) input.Blue );
  if( TempMatch < BestMatch )
  { BestI = (ebmpBYTE) i; BestMatch = TempMatch; }
  if( BestMatch < 1 )
  { i = NumberOfColors; }
  i++;
 }
 return BestI;
}

void BMP::DecodeTo4x4RGB8(unsigned char *dst)
{
    int vert, hor, col, row;
	unsigned char *p = (unsigned char *)dst;

	// Write to 4x4 blocks, so read from 4 lines at once
	// Read 4 pixels from each line, which is 16 pixels in a block
	// Every block contains 4 bytes for each colors, which is 64 bytes
	// The first 32 bytes will be alpha and red, the next 32 pixels will be green and blue
	
	// If the image width is not 4 by 4 exactly, 
	// add the amount of extra cells in transparent color
	for (vert = 0; vert < Height + (Height & 3); vert += 4) // height & 3 == height % 4, but a lot faster!
	{
		for (hor = 0; hor < Width + (Width & 3); hor += 4)
		{
			for (row = 0; row < 4; row++)
			{
				unsigned char *gb = (unsigned char *) (p + 32);
				for (col = 0; col < 4; col++)
				{
					if (vert + row >= Height || // Transparent cell
							hor + col >= Width)
					{
						*p++ = 0;
						*p++ = 255;
						*gb++ = 255;
						*gb++ = 255;
					}
					else
					{
						// Get the color of this pixel, which is at 
						// Line = vert + row, Col = hor + col
						RGBApixel pixel = Pixels[hor + col][vert + row];				
						*p++ = 255; //pixel.Alpha;
						*p++ = pixel.Red;
						*gb++ = pixel.Green;
						*gb++ = pixel.Blue;
					}
				}
			}
			p += 32;
		}
	}
}
