#ifndef WII_MOVIE_H_
#define WII_MOVIE_H_

#include "libwiigui/gui.h"
#include "Tools/BufferCircle.hpp"
#include "gcvid.h"

using namespace std;

class WiiMovie : public GuiElement
{
    public:
        WiiMovie(const char * filepath);
        ~WiiMovie();
        bool Play();
        void Stop();
        void SetVolume(int vol);
        void SetFullscreen();
        void SetFrameSize(int w, int h);
        void SetAspectRatio(float Aspect);
        void Draw();
        void Update(GuiTrigger * t);
    protected:
        void OnExitClick(GuiElement *sender, int pointer, POINT p);
		static void * UpdateThread(void *arg);
        void ReadNextFrame();
        void LoadNextFrame();
        void FrameLoadLoop();

        u8 * ThreadStack;
		lwp_t ReadThread;
		mutex_t mutex;

        VideoFile * Video;
        VideoFrame VideoF;
        BufferCircle SoundBuffer;
        float fps;
        Timer PlayTime;
        u32 VideoFrameCount;
        vector<u8 *> Frames;
		bool Playing;
		bool ExitRequested;
		int maxSoundSize;
		int SndChannels;
		int SndFrequence;
		int volume;

		GuiImage * background;
		GuiButton * exitBtn;
		GuiTrigger * trigB;
};

#endif
