#ifndef WII_MOVIE_H_
#define WII_MOVIE_H_

#include "libwiigui/gui.h"
#include "gcvid.h"

using namespace std;

typedef struct _VFrame
{
    u8 * videobuffer;
    vector<s16> soundbuffer;
    int soundlength;
} VFrame;

class WiiMovie : public GuiElement
{
    public:
        WiiMovie(const char * filepath);
        ~WiiMovie();
        bool Play();
        void Stop();
        void SetVolume(int vol);
        void Draw();
        void Update(GuiTrigger * t);
    protected:
        void OnExitClick(GuiElement *sender, int pointer, POINT p);
        void InternalUpdate();
        void FillBuffer();
        void LoadNextFrame();
        u8 * ConvertToFlippedRGBA(const u8 * src, u32 width, u32 height);

        VideoFile * Video;
        u32 VideoFrameCount;
        vector<VFrame> Frames;
		bool Playing;
		bool ExitRequested;
		u32 FrameCountGlobal;
		int SndChannels;
		int SndFrequence;
		int volume;

		GuiImage * background;
		GuiButton * exitBtn;
		GuiTrigger * trigB;
};

#endif
