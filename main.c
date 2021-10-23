//*******************************************************************************************************
//*******************************************************************************************************
//
//      Name:       Main.C
//      Purpose:    Main program.
//      Author:     Paul Robson
//      Date:       1st January 2014
//
//      Modified:   6th May 2017 by Raph Koster
//
//*******************************************************************************************************
//*******************************************************************************************************

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "general.h"
#include "hardware.h"
#include "core11.h"
#include "system.h"

//*******************************************************************************************************
//                                              Main Program
//*******************************************************************************************************

int main(int argc,char *argv[])
{
    BOOL quit = FALSE;
    int w = 0,h = 0,f = 0;                                                                    // Quick hack to scale screen size.
    if (argc == 3) {
        char c;
        char c2;
        sscanf(argv[2],"%d%c%d%c%d",&w,&c,&h,&c2,&f);
        argc--;
    }

    IF_Initialise(w,h,f);                                                                 // Initialise the hardware
    IF_SetSound(FALSE);
    if (argc != 2)
        exit(fprintf(stderr,"Microvision Emulator : Build %d : mvem <binary file> [<width>x<height>x<full screen 1 or 0>]\n",BUILD));
    char *file = argv[1];
    BOOL bDebug = file[0] == '@';                                                       // @ dumps to rom.h
    SYS_SetMode(bDebug);                                                                // Set start up mode.
    if (bDebug) file++;                                                                 // Dump at character.
    C11_LoadAndReorganise(file);                                                        // Load ROM Image
    IF_LoadOverlay(argv[1]);                                                            // Load overlay if it exists.
    int checksum = C11_ROMCheckSum();                                                   // Get the ROM checksum
    //printf("%x\n",checksum);
    C11_DetectHardware();                                                               // Detect the control.
    C11_Reset();                                                                        // Reset the TMS1100 CPU.
    while (!quit)                                                                       // Keep running till finished.
    {
        IF_ProcessRotaryControl();
        quit = IF_Render(SYS_Process());
    }
    IF_Terminate();
    return 0;
}

/*

    Fixes:

    8-1-14     1    Released.
    8-1-14     2    Fixed bug where sound not turned off if < 2 transitions to speaker line in a frame.
    9-1-14     3    Uprated speed to 500Khz Clock. Fixed overflow bug in sound frequency.
    11-1-14    4    Fixed to load LFSR-ordered binaries. Fixed LDX and BIT operand problem.
    13-1-14    5    Fixed Keypad wiring. Added checksum to detect cartridge variations.
    14-1-14    6    Added Rotary Control support.
    16-1-14    7    Fixed so handles column/row write, with adjustable rendering and latency row/colum switching.
    18-1-14    8    Added ASSERT() checking to attempt to fix wierd Windows problem.
    23-1-14    9    Removed switchable rendering, it doesn't work. Added Bowling Checksum.
    23-1-14    10   Added Pixel-based Latency stuff.
    24-1-14    11   Added Pinball Checksum (uses Rotary control). Added Mouse Control of Paddle. Changed non-debug display. Added overlays.
                    Added Pinball-compatible rotary control.
    18-3-14    13   Added checksums for all known cartridges.
                    Fixed Sea Duel opening display (no updating of latches in a frame)
                    Memory assumed clear on start by Sea Duel
                    Latency not reduced when no transfers from address latches to output latches in a frame.
                    Added variable speed, primarily for Sea Duel
                    Added paddle controls for those carts known to use them.
                    Now starts in run mode, @ switches to debugger on start up.
                    Added option to change screen size
                    Set up Paddle variables for paddle games.
                    TODO: Mouse support in Windows does not work.
    7-5-17     15   Allow passing in of screen res and full screen from the command line. Format is <width>x<height>x<fullscreen> where fullscreen = 1 for full and 0 for windowed.
                    The Enter/Return key now toggles fullscreen.
                    Emulator now chooses optimal size for playfield plus overlay based on the screen size.
                    Default screen to 800x600 windowed.
                    Add 1 pixel gap between pixels on LCD display.
                    Lighten the LCD pixel color so it is a closer match to actual LCD black.
                    Added overlays for all games. These are based on the American editions of the carts except for Super Blockbuster. Replaced the one by Paul Robson for Pinball.
                    Load snaps of carts and display on the side at full screen height.
                    Overlay key mappings on top of the snaps.
                    Created new snaps for the two homebrew games by Paul Robson.
                    Added linear filtering to textures for smoother images.
                    Slide playfield over so that both always fit.
                    There is now a knob that represents the range of motion for the paddle, rather than just the bar.
                    Made the existing bar-shaped render of knob position have alpha transparency, so it would blend into the overlays.
                    NOTE: Mouse support in Windows seems to be fine. Perhaps fixed in Build 14?

*/
