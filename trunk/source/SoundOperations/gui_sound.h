#ifndef GUI_SOUND_H_
#define GUI_SOUND_H_

#include <gccore.h>

//!Sound conversion and playback. A wrapper for other sound libraries - ASND, libmad, ltremor, etc
class GuiSound
{
	public:
		//!Constructor
		//!\param sound Pointer to the sound data
		//!\param filesize Length of sound data
		GuiSound(const u8 * sound, int filesize, bool allocated = false);
		//!Destructor
		~GuiSound();
		//!Load a file and replace the old one
		bool Load(const u8 * sound, int filesize, bool allocated = true);
		//!Start sound playback
		void Play();
		//!Stop sound playback
		void Stop();
		//!Pause sound playback
		void Pause();
		//!Resume sound playback
		void Resume();
		//!Checks if the sound is currently playing
		//!\return true if sound is playing, false otherwise
		bool IsPlaying();
		//!Set sound volume
		//!\param v Sound volume (0-100)
		void SetVolume(int v);
		//!Set the sound to loop playback (only applies to OGG & MP3)
		//!\param l Loop (true to loop)
		void SetLoop(u8 l);
	protected:
		//!Determine what filetype the sound is
		//!\param sound Pointer to the sound data
		int GetType();

		u8 * sound; //!< Pointer to the sound data
		int type; //!< Sound format type (SOUND_PCM or SOUND_OGG)
		s32 length; //!< Length of sound data
		s32 voice; //!< Currently assigned ASND voice channel
		u8 format; //!< Sound format (default 16 Bit Stereo)
		u32 frequency; //!< Sound frequency (default 48kHz)
		s32 volume; //!< Sound volume (0-100)
		u8 loop; //!< Loop sound playback
		bool allocated; //!< Is the file allocated or not
};

#endif
