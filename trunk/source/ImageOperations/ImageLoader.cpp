#include <unistd.h>
#include "Controls/MainWindow.h"
#include "ImageViewer.h"
#include "sys.h"

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
