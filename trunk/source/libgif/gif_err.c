/*****************************************************************************
 *   "Gif-Lib" - Yet another gif library.                     
 *                                         
 * Written by:  Gershon Elber            IBM PC Ver 0.1,    Jun. 1989    
 *****************************************************************************
 * Handle error reporting for the GIF library.                     
 *****************************************************************************
 * History:                                     
 * 17 Jun 89 - Version 1.0 by Gershon Elber.                     
 *
 * Some small changes were made by r-win, for the use of this library
 * with WiiXplorer
 *
 ****************************************************************************/

#include <stdio.h>
#include "gif_lib.h"

int _GifError = 0;

/*****************************************************************************
 * Return the last GIF error (0 if none) and reset the error.             
 ****************************************************************************/
int
GifLastError(void) {
    int i = _GifError;

    _GifError = 0;

    return i;
}
#ifndef _GBA_NO_FILEIO

/*****************************************************************************
 * Return the last GIF error.
 ****************************************************************************/
char *
GetGifError(void) {
    char *Err;

    switch (_GifError) {
      case D_GIF_ERR_OPEN_FAILED:
        Err = "Failed to open given file";
        break;
      case D_GIF_ERR_READ_FAILED:
        Err = "Failed to Read from given file";
        break;
      case D_GIF_ERR_NOT_GIF_FILE:
        Err = "Given file is NOT GIF file";
        break;
      case D_GIF_ERR_NO_SCRN_DSCR:
        Err = "No Screen Descriptor detected";
        break;
      case D_GIF_ERR_NO_IMAG_DSCR:
        Err = "No Image Descriptor detected";
        break;
      case D_GIF_ERR_NO_COLOR_MAP:
        Err = "Neither Global Nor Local color map";
        break;
      case D_GIF_ERR_WRONG_RECORD:
        Err = "Wrong record type detected";
        break;
      case D_GIF_ERR_DATA_TOO_BIG:
        Err = "#Pixels bigger than Width * Height";
        break;
      case D_GIF_ERR_NOT_ENOUGH_MEM:
        Err = "Fail to allocate required memory";
        break;
      case D_GIF_ERR_CLOSE_FAILED:
        Err = "Failed to close given file";
        break;
      case D_GIF_ERR_NOT_READABLE:
        Err = "Given file was not opened for read";
        break;
      case D_GIF_ERR_IMAGE_DEFECT:
        Err = "Image is defective, decoding aborted";
        break;
      case D_GIF_ERR_EOF_TOO_SOON:
        Err = "Image EOF detected, before image complete";
        break;
      default:
        Err = NULL;
        break;
    }
	return Err;
}
/*****************************************************************************
 * Print the last GIF error to stderr.                         
 ****************************************************************************/
void
PrintGifError(void) {
    char *Err = GetGifError();

    if (Err != NULL)
        fprintf(stderr, "\nGIF-LIB error: %s.\n", Err);
    else
        fprintf(stderr, "\nGIF-LIB undefined error %d.\n", _GifError);
}
#endif /* _GBA_NO_FILEIO */
