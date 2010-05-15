#ifndef PDFVIEWER_HPP_
#define PDFVIEWER_HPP_

#include "ImageOperations/ImageViewer.h"
#include "TextOperations/mupdf/pdftool.h"

class PDFViewer : public ImageViewer
{
    public:
        PDFViewer(const char * filepath, const char * password = NULL);
        ~PDFViewer();
        void OpenFile(const char * filename, const char * password = NULL);
        void CloseFile();
        bool LoadPage(int pagenum);
        bool NextPage();
        bool PreviousPage();
        void SetImageSize(float scale);
        int MainUpdate();
        //! Virtual overloads which are not needed
        bool NextImage(bool silent = false) { return NextPage(); };
        bool PreviousImage(bool silent = false) { return PreviousPage(); };
    protected:
        int PreparePage(int pagenum);
        int PageToRGBA8(int pagenum);
        //! Virtual overloads which are not needed
        bool LoadImage(int index, bool silent = false) { return LoadPage(index); };
        bool LoadImageList(const char * filepath) { return true; };

        u8 * OutputImage;
        fz_glyphcache *drawcache;
        pdf_page *drawpage;
        float drawzoom;
        int currentPage;
        int PageCount;
        int drawrotate;
        int imagewidth;
        int imageheight;
};

#endif
