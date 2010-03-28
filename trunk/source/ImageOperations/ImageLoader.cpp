#include <unistd.h>
#include "Controls/MainWindow.h"
#include "ImageViewer.h"
#include "ImageConverterGUI.hpp"
#include "sys.h"

void ImageConverterLoader(const char *filepath)
{
    ImageConverterGui * ImageConv = new ImageConverterGui(filepath);
    ImageConv->SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);

    MainWindow::Instance()->Append(ImageConv);

    while(ImageConv->MainUpdate() < 0)
    {
	    usleep(100);

        if(shutdown)
            Sys_Shutdown();
        else if(reset)
            Sys_Reboot();
    }
    delete ImageConv;
    ImageConv = NULL;

    MainWindow::Instance()->ResumeGui();
}

void ImageLoader(const char *filepath)
{
    ImageViewer * ImageVwr = new ImageViewer(filepath);

    MainWindow::Instance()->Append(ImageVwr);

    while(ImageVwr->MainUpdate() < 0)
    {
	    VIDEO_WaitVSync();

        if(shutdown)
            Sys_Shutdown();
        else if(reset)
            Sys_Reboot();
    }
    delete ImageVwr;
    ImageVwr = NULL;

    MainWindow::Instance()->ResumeGui();
}
