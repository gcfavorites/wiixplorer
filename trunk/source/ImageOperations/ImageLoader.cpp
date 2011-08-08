#include <unistd.h>
#include "Controls/Application.h"
#include "ImageViewer.h"
#include "ImageConverterGUI.hpp"
#include "sys.h"

void ImageConverterLoader(const char *filepath)
{
	ImageConverterGui * ImageConv = new ImageConverterGui(filepath);
	ImageConv->SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);

	Application::Instance()->Append(ImageConv);

	while(ImageConv->MainUpdate() < 0)
	{
		usleep(100);
	}
	delete ImageConv;
	ImageConv = NULL;
}

void ImageLoader(const char *filepath)
{
	ImageViewer * ImageVwr = new ImageViewer(filepath);

	Application::Instance()->Append(ImageVwr);

	while(ImageVwr->MainUpdate() < 0)
	{
		VIDEO_WaitVSync();
	}
	delete ImageVwr;
	ImageVwr = NULL;
}
