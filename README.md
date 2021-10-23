# MicroVision-Updated
This is Paul Robinsons Microvision Emulator with the changes from  Rkoster added 


RetroArena/ARES/RetroPie How To:
*** How to build and install Microvision 
 

RetroPie installed on top of Raspbian
Bit of background before we start. The Milton-Bradley Microvision was released in 1979. It was the first handheld to use interchangeable cartridges. It had a 16x16 square pixel screen, an analog knob at the bottom, and a membrane-based keypad so that when cartridges were swapped out, different buttons, up to 3(across) by 4 (down) would be available to the user. Unfortunately, it was not successful, and the build quality combined with the age of the units means working consoles are rare. More information can be found on wikipedia as well as this really nice fansite. I have also included a picture below.
http://www.handheldmuseum.com/MB/uVUS.htm
📷
MVEM and some versions of MESS/MAME emulate this system. MVEM is superior in my opinion. I did not create MVEM. All credit there goes to Paul Robson who coded the emulator and a few homebrew games for the Microvision around 2014 and released the source code and binaries for his final build (Build 14) free for anyone to use. All I did was work out how to get it installed in RetroPie.
Unfortunately, I cannot give a link to the emulator itself. All binary and source code distributions I have been able to find contain at least some copyrighted games as *.bin files. I am looking into creating a GitHub repository without any of the copyrighted material.
EDIT: Here is a GitHub repository link. It only contains the emulator source code freely released by Paul Robson and his own freely released homebrew games. Other games are copyrighted to Milton-Bradley and cannot be legally released
 slaminger/Paul-Robson-s-Microvision-Emulation: Repository for Microvision Emulator (Build 14) produced by Paul Robson and freely released (github.com) 
EDIT: It has come to my attention that Raph Koster has released a Build 15 of MVEM based on Paul Robson's original source code. This new build can also be made to work with RetroPie without much difficulty. This features a game screen area with spaces between the pixels, overlays for controller and screen, and an explanation of the game controls. In my opinion, this is an improvement over build 14. I will leave the build 14 information available here, but I will also add sections to the instructions where build 15 installation differs from build 14. These sections are current marked with EDIT BLOCK RELEVANT TO BUILD 15 and END OF EDIT BLOCK to begin and end the sections respectively. Unfortunately, I do not have a download link at this time for the build 15 source, as all distributions that I know of contain the games that are still copyrighted by Milton-Bradley.
I can't really say anything else without breaking forum rules, so I'll move on. If you have found the source code, then we can begin at step 1.
The source code distribution that worked for me is commonly called "emulator-osx.tar.gz" which contains Build 14.
This has a CRC32 checksum (E873F9A9).
EDIT BLOCK RELEVANT TO BUILD 15:
Build 15 source is usually called mvem_dev.zip. My version has CRC32 checksum (0AB7BE73)
The .bmp files used for the overlays can be copied over from the Windows release of MVEM build 15. My version is called MVEM.zip and has the CRC32 checksum (B30EB4D0)
END OF EDIT BLOCK
There is another distribution marked as the complete project that actually only contains Build 12. The main difference that I can see is that Build 12 does not have the ability to change the screen size in the command-line input.
Part 1: Compilation
I was able to compile via GCC 4.9.2, available standard under Raspbian Jessie. No need to go to Raspbian Stretch to grab GCC5+ as with later MAME versions.
Aside from this, the only prerequisite appears to be SDL2, judging by the information in the Makefile.
Once you have the source code, you will need to choose a directory (i.e /home/pi/mvem and place everything there). Here is a list of source files from the version I have along with CRC32 checksums.
BUILD 14:
code11.h (A6A762F3)
core11.c (D1047849)
core11.h (628B3E4)
debugsc11.c (7BCC4DA9)
debugsc11.h (2113B5Ed)
font.h (9841B07E)
general.h (5463D313)
hardware.c (0113A4D3)
hardware.h (DA38D0AF)
hwinterface.c (7CE3AFD7)
hwinterface.h (E4FFDAF2)
main.c (6D004998)
mnemonics11.h (90685C70)
opcodes11.h (9A90C1B8)
rom.h (2E642F46)
system.c (CE941B7E)
system.h (C39D5E72)
EDIT BLOCK RELEVANT TO BUILD 15:
All source files have the same names. However, as Build 15 may be in active development, I cannot reliably give CRC32 checksums.
END OF EDIT BLOCK
BUILD 14:
You should also have this Makefile
CC      = gcc EXE	=  CFLAGS  = -g -O2 -DDEBUGGABLE -D_THREAD_SAFE -I/usr/local/include/SDL2 -I/usr/X11R6/include  -I/usr/X11/include -DHAVE_OPENGL -g LIBS	=  -lSDL2_test -L/usr/local/lib -lSDL2   TARGETS = mvem$(EXE) all: $(TARGETS) mvem$(EXE): main.c hardware.c core11.c debugsc11.c system.c hwinterface.c # debugsc80.c  core80.c $(CC) -o $@ $^ $(CFLAGS) $(LIBS) 
However, I needed to modify the Makefile to allow it to compile on my Pi
The CFLAGS line (should be line 4), needs to be modified as shown below.
CC      = gcc EXE	=  CFLAGS  = -std=c99 -g -O2 -DDEBUGGABLE -D_THREAD_SAFE -I/usr/local/include/SDL2 -I/usr/X11R6/include  -I/usr/X11/include -DHAVE_OPENGL -g LIBS	=  -lSDL2_test -L/usr/local/lib -lSDL2   TARGETS = mvem$(EXE) all: $(TARGETS) mvem$(EXE): main.c hardware.c core11.c debugsc11.c system.c hwinterface.c # debugsc80.c  core80.c $(CC) -o $@ $^ $(CFLAGS) $(LIBS) 
EDIT BLOCK RELEVANT TO BUILD 15:
Some versions of the Makefile for Build 15 need to be edited for successful Raspberry Pi compilation. Here is my working Makefile
CC      = gcc EXE	= CFLAGS  = -std=c99 -g -O2 -DDEBUGGABLE -D_THREAD_SAFE -I/usr/local/include/SDL2 -I/usr/X11R6/include  -I/usr/X11/include -DHAVE_OPENGL -g LIBS	= -lSDL2_test -L/usr/local/lib -lSDL2  TARGETS = mvem$(EXE) all: $(TARGETS) mvem$(EXE): main.c hardware.c core11.c debugsc11.c system.c hwinterface.c # debugsc80.c  core80.c $(CC) -o $@ $^ $(CFLAGS) $(LIBS) clean: $(RM) *.o *~ $(MAIN) 
Please ensure that the CFLAGS line contains the -std=c99 and that the LIBS line does not contain an erroneous "#".
Also, please examine this section at the beginning of my working hardware.c
#include <stdio.h> #include <stdlib.h> #include <ctype.h> #include "hardware.h" #include "hwinterface.h" #include "debugsc11.h" #include <SDL2/SDL.h> 
Some versions of hardware.c replace the #include <SDL2/SDL.h> with #include <c:/MINGW/MINGW_DEV_LIB/INCLUDE/SDL2/SDL.h>. This is a mistake and needs to be corrected to match my version before compilation.
END OF EDIT BLOCK
If you have an executable file labeled "mvem" remove it by typing "rm mvem" first. After that you should be able to type "make" to compile via GCC 4.9.2
Here is the output from my successful compilation. There is a warning, but it seems harmless.
gcc -o mvem main.c hardware.c core11.c debugsc11.c system.c hwinterface.c -std=c99 -g -O2 -DDEBUGGABLE -D_THREAD_SAFE -I/usr/local/include/SDL2 -I/usr/X11R6/include  -I/usr/X11/include -DHAVE_OPENGL -g -lSDL2_test -L/usr/local/lib -lSDL2 In file included from core11.c:37:0: core11.c: In function ‘tms1100reset’: code11.h:18:31: warning: iteration 128u invokes undefined behavior [-Waggressive-loop-optimizations]     dataMemory[i] = dataMemory[i] & 0xF;                                ^ code11.h:17:2: note: containing loop  for (i = 0;i <= 0x80;i++)   ^   
EDIT BLOCK RELEVANT TO BUILD 15
Here are the warnings I got from compiling Build 15. My version appears to work properly in spite of these warnings.
gcc -o mvem main.c hardware.c core11.c debugsc11.c system.c hwinterface.c -std=c99 -g -O2 -DDEBUGGABLE -D_THREAD_SAFE -I/usr/local/include/SDL2 -I/usr/X11R6/include  -I/usr/X11/include -DHAVE_OPENGL -g -lSDL2_test -L/usr/local/lib -lSDL2 hardware.c: In function ‘IF_Initialise’: hardware.c:85:17: warning: implicit declaration of function ‘IF_ToggleFullscree ’ [-Wimplicit-function-declaration]      if (f == 1) IF_ToggleFullscreen();                  ^~~~~~~~~~~~~~~~~~~ hardware.c: At top level: hardware.c:155:6: warning: conflicting types for ‘IF_ToggleFullscreen’  void IF_ToggleFullscreen(SDL_Window* Window) {       ^~~~~~~~~~~~~~~~~~~ hardware.c:85:17: note: previous implicit declaration of ‘IF_ToggleFullscreen’ was here      if (f == 1) IF_ToggleFullscreen();                  ^~~~~~~~~~~~~~~~~~~ In file included from core11.c:38:0: core11.c: In function ‘tms1100reset’: code11.h:18:31: warning: iteration 128 invokes undefined behavior [-Waggressive-loop-optimizations]      dataMemory[i] = dataMemory[i] & 0xF;                      ~~~~~~~~~~^~~ code11.h:17:2: note: within this loop for (i = 0;i <= 0x80;i++)   ^~~ 
END OF EDIT BLOCK
If compilation fails, then it is most likely that you are missing GCC or SDL2. As far as I know, those are the only two requirements here. My original installation was several months ago, so I may have forgotten something. However, I performed a fresh compilation when writing this tutorial. 4  

Part 2: Running MVEM
Once compilation is complete, you can use the following command format to run MVEM (Build 14). Build 12 users will need to remove the <screen_size> part, as that is not available in that build.
./mvem <rom_filename> <screen_size>./mvem "bomber.bin" 640x480 
At this point, a more advanced user might be done listening to me and simply follow the RetroPie tutorial for adding an external emulator. However, I will continue to describe my RetroPie configuration process and provide the checksums for all known game files that work with MVEM.
Part 3: Configuring RetroPie for MVEM
First, You will need to find your es_systems.cfg file
/etc/emulationstation/es_systems.cfg is the default
You will need to add the following code inside the <systemList></systemList> tags, but outside of any other tags. You will also need to change the PATH to match your directory. I highly recommend you store your games and emulator executable in the same directory.
<system> <name>microvision</name> <fullname>Milton-Bradley Microvision</fullname> <path>/home/pi/emulator-osx</path> <extension>.bin .BIN</extension> <command>/opt/retropie/supplementary/runcommand/runcommand.sh 0 _SYS_ microvision %ROM%</command> <platform>microvision</platform> <theme>microvision</theme> </system> 
Second, you will need to handle the /opt/retropie/configs changes.
Go to your /opt/retropie/configs directory and create a new folder name microvision
Inside this new folder, you need to create a retroarch.cfg and emulators.cfg file.
Here are the contents of my working files. You may need to change paths to match your own install directories.
emulators.cfg:
mvem = "/home/pi/emulator-osx/mvem  %ROM% 640x480" default = "mvem" 
retroarch.cfg:
# Settings made here will only override settings in the global retroarch.cfg if placed above the #include line input_remapping_directory = "/opt/retropie/configs/microvision/" #include "/opt/retropie/configs/all/retroarch.cfg" 
At this point, you should be able to restart emulation station and have a new section for the microvision with all of your *.bin files visible. 1  

Part 4: Controlling the Emulator
The controls are hardcoded:
Escape is used to exit.
The analog control can use either the mouse movement left and right or O and P.
The microvision control pad (3 across x 4 down) uses 123,qwe,asd,zxc for each of the respective rows. Overlays are not shown, so it may be tricky to work out how to play. You may need to look up images of the microvision keypads for these games.
Debugging mode is accessed with M. I don't really understand it, so I'm going to put Paul Robson's explanation here. Probably best to stay out of debugging mode though.
0-9A-F change displayed address
G run program until broken (with M)
K set breakpoint at current address
S single step
V step over
M return to monitor (if running)
Esc exits the debugger
Part 5: Notes on ROMS and MVEM configuration
I recommend sticking with the 640x480 game area at the maximum. It is large enough to be clearly visible to me, and with 16x16 square pixels on the screen, you don't have to worry about missing detail. The main problem with increasing the game area is that there is a separate row below the game area that shows the current position of the analog control. If the game area is made larger than 640x480, this row will overlap the game area. As the source code has been released, this may be fixable, but I don't know how to do it.
EDIT BLOCK RELEVANT TO BUILD 15
Build 15 users are able to select larger screen sizes. I currently use 1280x720x1 for my screen size value.
Raph Koster has confirmed that 1920x1080x1 should work for users of 1080p screens
END OF EDIT BLOCK
Here are the CRC32 checksums for my current collection (Build 14).
Alien Raiders(Milton Bradley)(1981):
alienraiders.bin (B1632712)
Baseball(Milton Bradley)(1980):
baseball.bin (40E3327A)
Block Buster(Milton Bradley)(1979):
block.bin (FD18EA6D)
Bomber(Milton Bradley)(2014):
bomber.bin (BADBD0F8)
Bowling(Milton Bradley)(1979):
bowling.bin (460AC30E)
Connect Four(Milton Bradley)(1979):
connect4.bin (6A4CF60B)
Cosmic Hunter(Milton Bradley)(1981):
cosmichunter.bin (D8BA1377)
Demo(Paul Robson)(2014):
demo.bin (54F1B288)
Invaders(Paul Robson)(2014):
invaders.bin (D520FF6E)
Mindbuster(Milton Bradley)(1979):
mind.bin (D5DC1F7D)
Star Trek Phaser Strike(Milton Bradley)(1979):
phaser.bin (C7768D04)
Pinball (Milton Bradley)(1980):
pinball.bin (906544EA)
Super Block Buster(Milton Bradley)(19xx):
sbb.bin (796660E3)
Sea Duel (Milton Bradley)(1980):
seaduel.bin (83093475)
Vegas Slots (Milton Bradley)(1979):
slots.bin (4500C1D9)
Speed(Paul Robson)(2014):
speed.bin (67A53F5E)
Test(Paul Robson)(2014):
test.bin (A64065F9)
Once you have these files, you can rename them as you wish. However, you may wish to leave pinball.bin as is. The pinball.bin uses a separate pinball.bmp for a background, which is included with the source code. If pinball.bin is renamed, this background will not show up when running the game. This background is not required to run the game, but the game logic makes much more sense with the background.
EDIT BLOCK RELEVANT TO BUILD 15:
The file naming scheme has changed in Build 15. I have not fully tested file renaming. I would recommend sticking with the names from Raph Koster's distribution, as the emulator appears to use these names to determine which .bmp images to load for the controller and screen overlays.
Also, there is an ongoing bug where certain .bmp images (controller, key hints, and knob) will not appear if you are loading MVEM after booting the Raspberry Pi straight to Emulation Station. This is fixable by exiting Emulation Station, running and exiting MVEM on any game and then reopening Emulation Station. This appears to be a problem with SDL-based external emulators and RetroPie/Emulation Station.
END OF EDIT BLOCK


More information:
https://www.raphkoster.com/2017/05/07/microvision-emulator-release/
