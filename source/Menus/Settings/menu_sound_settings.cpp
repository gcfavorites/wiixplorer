/****************************************************************************
 * Copyright (C) 2010
 * by Dimok
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any
 * damages arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any
 * purpose, including commercial applications, and to alter it and
 * redistribute it freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you
 * must not claim that you wrote the original software. If you use
 * this software in a product, an acknowledgment in the product
 * documentation would be appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and
 * must not be misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source
 * distribution.
 *
 * for WiiXplorer 2010
 ***************************************************************************/
#include "SettingsMenu.h"
#include "Controls/MainWindow.h"
#include "Controls/Taskbar.h"
#include "Prompts/PromptWindows.h"
#include "SoundOperations/MusicPlayer.h"

int MenuSoundSettings()
{
	int menu = MENU_NONE;
	int ret;
	int i = 0;
	char entered[150];
	bool firstRun = true;

	OptionList options;
	options.SetName(i++, tr("Music Volume"));
	options.SetName(i++, tr("Music Loop Mode"));
	options.SetName(i++, tr("Load Music to Memory"));
	options.SetName(i++, tr("Soundblocks"));
	options.SetName(i++, tr("Soundblock Size"));

	SettingsMenu * Menu = new SettingsMenu(tr("Sound Settings"), &options, MENU_SETTINGS);

	MainWindow::Instance()->Append(Menu);

	while(menu == MENU_NONE)
	{
	    usleep(THREAD_SLEEP);

		if(Menu->GetMenu() != MENU_NONE)
		{
			menu = Menu->GetMenu();
		}
        else if(Taskbar::Instance()->GetMenu() != MENU_NONE)
        {
			menu = Taskbar::Instance()->GetMenu();
        }

		ret = Menu->GetClickedOption();

		switch (ret)
		{
            case 0:
				Settings.MusicVolume += 10;
				if(Settings.MusicVolume > 100)
                    Settings.MusicVolume = 0;
                MusicPlayer::Instance()->SetVolume(Settings.MusicVolume);
				break;
            case 1:
                Settings.BGMLoopMode++;
                if(Settings.BGMLoopMode >= MAX_LOOP_MODES)
                    Settings.BGMLoopMode = 0;
                MusicPlayer::Instance()->SetLoop(Settings.BGMLoopMode);
				break;
            case 2:
                Settings.LoadMusicToMem = (Settings.LoadMusicToMem+1) % 2;
				break;
            case 3:
                snprintf(entered, sizeof(entered), "%i", Settings.SoundblockCount);
                if(Settings.LoadMusicToMem != 1 && OnScreenKeyboard(entered, 149))
                {
					Settings.SoundblockCount = atoi(entered);
					if(Settings.SoundblockCount < 3)
                        Settings.SoundblockCount = 3;
					WindowPrompt(tr("Warning:"), tr("The effect will take with next music load. It might break music playback."), tr("OK"));
					if(Settings.SoundblockSize*Settings.SoundblockCount > 512*1024)
                        WindowPrompt(tr("WARNING:"), tr("The buffer size is really high. If the app doesn't start after this delete your config files."), tr("OK"));
                }
				break;
            case 4:
                snprintf(entered, sizeof(entered), "%i", Settings.SoundblockSize);
                if(Settings.LoadMusicToMem != 1 && OnScreenKeyboard(entered, 149))
                {
					Settings.SoundblockSize = atoi(entered);
					WindowPrompt(tr("Warning:"), tr("The effect will take with next music load. It might break music playback."), tr("OK"));
					if(Settings.SoundblockSize*Settings.SoundblockCount > 512*1024)
                        WindowPrompt(tr("WARNING:"), tr("The buffer size is really high. If the app doesn't start after this delete your config files."), tr("OK"));
                }
				break;
		}

        if(firstRun || ret >= 0)
        {
            i = 0;
            firstRun = false;

            if(Settings.MusicVolume > 0)
                options.SetValue(i++, "%i", Settings.MusicVolume);
            else
                options.SetValue(i++, tr("OFF"));

            if(Settings.BGMLoopMode == ONCE) options.SetValue(i++,tr("Play Once"));
            else if (Settings.BGMLoopMode == LOOP) options.SetValue(i++,tr("Loop"));
            else if (Settings.BGMLoopMode == RANDOM_MUSIC) options.SetValue(i++,tr("Random"));
            else if (Settings.BGMLoopMode == PLAYLIST_LOOP) options.SetValue(i++,tr("Loop Playlist"));

            if(Settings.LoadMusicToMem == 1) options.SetValue(i++, tr("ON"));
            else options.SetValue(i++, tr("OFF"));

            if(Settings.LoadMusicToMem == 1) options.SetValue(i++, tr("Memory Buffer"));
            else options.SetValue(i++, "%i (%0.1f KB)", Settings.SoundblockCount, Settings.SoundblockCount*Settings.SoundblockSize/1024.0f);

            if(Settings.LoadMusicToMem == 1) options.SetValue(i++, tr("Memory Buffer"));
            else options.SetValue(i++, "%i Bytes", Settings.SoundblockSize);
        }
	}

    delete Menu;

    Settings.Save();

	return menu;
}
