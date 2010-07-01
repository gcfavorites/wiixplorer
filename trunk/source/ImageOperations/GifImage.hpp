#ifndef GIFIMAGE_HPP_
#define GIFIMAGE_HPP_

#include <vector>
#include <gctypes.h>
#include "Tools/tools.h"

typedef struct
{
    u8 * image;
    int width;
    int height;
    int offsetx;
    int offsety;
    int Delay;
    int Disposal;
    bool Transparent;
} GifFrame;

class GifImage
{
    public:
        GifImage(const u8 * img, int imgSize);
        ~GifImage();
        void LoadImage(const u8 * img, int imgSize);
        u8 * GetFrameImage(int pos);
        int GetWidth() { return MainWidth; };
        int GetHeight() { return MainHeight; };
        int GetFrameCount() { return Frames.size(); };
        void Draw(int x, int y, int z, int degrees, float scaleX,
                  float scaleY, int alpha, int minwidth, int maxwidth,
                  int minheight, int maxheight);
    protected:
        int MainWidth;
        int MainHeight;
        int last;
        int currentFrame;
        float lastTimer;
        Timer DelayTimer;
        std::vector<GifFrame> RedrawQueue;
        std::vector<GifFrame> Frames;
};

#endif
