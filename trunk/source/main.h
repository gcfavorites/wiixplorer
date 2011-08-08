/** \mainpage WiiXplorer
 *
 * \section sec_intro Introduction
 *
 * WiiXplorer is a multi-device file explorer for the Wii made with a customized libwiigui as base for the GUI.
 * It has several additional features to execute various of filetypes like on an actual browser/explorer.
 * <p>
 * It was created and is developed by Dimok. Some features of the project were contributed/developed by R-win and Dude.
 * <p>
 * The graphical design and the required artwork for that were/are made by NeoRame.
 * <p>
 * WiiXplorer is written mainly in C++ and makes use of sevaral open-source C/C++ libraries.
 * <p>
 * \section sec_special_thanks Special thanks:
 * Dj Skual, kavid and all translators<br>
 * Tantric for his tool libwiigui<br>
 * Armin Tamzarian for FreeTypeGX<br>
 * The libogc/devkitPro Team<br>
 *
 * \section sec_license License
 *
 * The WiiXplorer source code is distributed under the GNU General Public License v3.
 *
 * \section sec_contact Contact
 *
 * If you have any suggestions, questions, or comments regarding the source code or the application feel free to e-mail me at dimok321@hotmail.com.
 */
#ifndef _MAIN_H_
#define _MAIN_H_

#include "Settings.h"

#define WiiControls		 Settings.Controls.WiiControls
#define ClassicControls	 Settings.Controls.ClassicControls
#define GCControls		  Settings.Controls.GCControls

#endif
