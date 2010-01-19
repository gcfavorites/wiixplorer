#include "Controls/MainWindow.h"
#include "ImageViewer.h"

void ImageLoader(const char *filepath)
{
    ImageViewer * ImageVwr = new ImageViewer(filepath);

    MainWindow::Instance()->Append(ImageVwr);

    while(ImageVwr->MainUpdate() < 0)
    {
	    VIDEO_WaitVSync();
    }
    delete ImageVwr;
    ImageVwr = NULL;

    MainWindow::Instance()->ResumeGui();
}
