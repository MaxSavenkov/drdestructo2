ReadMe file for Return of Dr. Destructo v1.0.
-------------------------------------------------

1. About

The "Return Of Dr. Destructo" is a remake of an ZX Spectrum game, "Island of Dr. Destructo" by Bulldog Software/Mastertronics.
The goal of this project is to recreate all original levels with new, better graphics, but without introducing new levels or 
gameplay elements.

In each level, you have to destroy a ship, a castle or some other structure belonging to the evil Dr. Destructo, while evading or
shooting down various aircrafts he sents after you. There is a time limit: you must complete all 21 levels in space of one week in
game time (duration of a game day in real time can be set in Gameplay Settings menu), so you have to be aggressive and take risks!

Your plane has only one bomb, and your gun is no use against well-armored targets, so you can only complete your mission by
dropping enemy planes on your target. Collisions with some enemies are harmless, but most of them will crash your plane. Finding
which is which is a part of the game experience, just like it was in 80's :)

To sink level's target, you must destroy a specific percent of its pixels near waterline. Each falling plane destroys a small
circular chunk. Level completion indicator is the red bar in lower-right corner of the screen, where level's name is displayed.

The remake features many improvements, including Casual mode, where you can continue game from any level you have already
reached with the full number of days and lives. For finishing the game in one sitting in Hardcore Mode, you get a special
achievement.

You can download source code for this game at: http://zxstudio.org/projects/drdestructo/

-------------------------------------------------

2. Controls

The default control scheme is:

Right Arrow - Rotate clockwise
Left Arrow  - Rotate counter-clockwise
Up Arrow    - Speed boost
Q           - Fire gun
A           - Drop bomb

You can change controls in Options menu

-------------------------------------------------

3. Known bugs & limitations

* Only generic XInput gamepads are supported on Linux (all others may work, but also may have wrong mapping displayed int tutorial & control options)
* Gamepad support on MacOS X is very limited
* The game may only work on MacOS X 10.9+ and will simply crash on earlier versions

-------------------------------------------------

4. Contacts

You can download the latest version, report bugs and request features at http://zxstudio.org/projects/drdestructo/
You can also reach me via e-mail at max-savenkov AT tochka DOT ru

-------------------------------------------------

5. Future plans

Mobile versions (first, Android, later iOS).

-------------------------------------------------

6. Version history

Version 1.0 ( 12.04.15 ):
	* Removed Rapid Fire option: you can now either hold fire button, or tap it rapidly
	* Added level completion indicator
	* Fixed bad text placement in some cases
	* Fixed image corruption in briefing
	* Fixed a rare problem when multiple lives could be lost at once (a 1.0b2 regression)
	* Fixed level selection: clicking on the left side of button with mouse wasn't selecting previous level

Version 1.0b2 ( 09.03.15 ):
	* Added displaying of control schemes to tutorial
	* Added life loss effect (cracks) and information about then enemy the player have crashed into
	* Fixed clipping rectangle corruption when swtiching to/from fullscreen mode
	* Various other fixes

Version 1.0b1 ( 20.12.14 ):
	* Totally new graphics for everything
	* Gamepad support
	* Mouse support in all menus
	* Tutorial
	* Removed export options from Achievements menu
	* Optimized explosion code, it should no longer cause lags
    * Fixed some issues with screen mode switching
	* Brand new level selection menu for Casual mode
    * Fixed a serious bug in High Scores
	* Integrated Goodle Brekpad for crash reporting

Version 0.9 ( 01.06.12 ):
	* Casual mode (you can continue game from any level you have already reached)
	* After-level statistic table
	* Achievements
	* Short invincibility period after every respawn
	* Made non-turbo-fire mode slightly less fast
	* Fixed some problems with music, removed irrKlang dependency

Version 0.8 ( 08.04.12 ):	
	* Huge performance increases
	* Three new enemies: shuttle, spacejet, satellite
	* ALL game levels
	* Outro!
	* Optimized game loading (should not have any noticeable delay before launch and intro now)
	* New Lua-based AI scripts for easier editing
	* Options menu split into several sub-menus
	* More difficulty options (number of lives and length of day)	
	* Added option to turn VSync on/off (on by default)
	* Option to turn turbo fire mode on/off (on by default)
	* Special music for menu and briefing
	* Fanfare for level end
	* Level complete screen is now cleared faster (all objects move at once)	

Version 0.7 ( 25.02.12 ):	
	* Two new enemies (UFO, hyperjet)
	* 17 levels
	* Fixed graphic glitches
	* Sound & Music
	* Intro picture, also used for menu & briefing background

Version 0.6 ( 07.01.12 ):
	* Two new enemies (two-seater planes, bombers)
	* 13 levels
	* Story briefings between levels
	* Story messages during levels
	* High Score table
	* Second player weapon - bombs (one per life)
	* Fixed graphic glitches
	* Aiming Aid as option
	* Better spawning algorithm for enemies
	* Performance improvements
	* Uneven damages
	
Version 0.5 ( 27.11.11 ):
	* Two new enemies (skymines & helicopters)
	* Three new levels & first level reworked to match original game
	* Menu with support for changing screen resolution, mode and key bindings
	* Scaling of game screen for different resolutions
	* More lives for player
	* Time limit from original game added (presently, 7 days = 21 minutes of real time)
	* Added screens for last level completion and timeout

Version 0.4 ( 25.10.11 ):
	* FIRST PUBLIC RELEASE
	* Target damage and sinking
	* Lives loss & gameover screen
	* Level restart & game restart
	
Version 0.3 ( 11.09.11 ):
	* Player's gun fire
	* Enemies death
	* Player's death
	* Two new enemies
	
Version 0.2 ( 03.09.11 ):
	* Loading of prototypes
	* First two enemies
	* Beginning of AI
	* Object spawner (Level director)
	
Version 0.1 ( 11.08.11 ):
	* Player's plane
	* Background with moving astral bodies & day and night cycle
	
-------------------------------------------------

7. Credits

Programming, Design, Music & Sound:

* MaxEd (max-savenkov AT tochka DO ru)

New artwork:

* Oleg

Some sounds generated using as3sfxr by Tom Vian (http://www.superflashbros.net/as3sfxr/)

This game uses the following libraries:

Allegro 5.1.x (by Allegro Community, http://www.allegro.cc/)
Lua 5.1 (by Pontifical Catholic University of Rio de Janeiro in Brazil, http://www.lua.org)
Luabind 0.9.1 (by Rasterbar Software, http://www.rasterbar.com/products/luabind.html)
TinyXML 2.6.2 (by Lee Thomason, http://www.grinninglizard.com/tinyxml/)
Google Breakpad

Original game Programming, Design & Art:

* Eugene Messina
* David Lincoln-Howells

-------------------------------------------------

8. License

This game's code is licensed under MIT license, and art is CC-BY-SA. Please see LICENSE_CODE.txt and LICENSE_ART.txt for details.

-------------------------------------------------
