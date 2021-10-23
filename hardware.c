//*******************************************************************************************************
//*******************************************************************************************************
//
//      Name:       Hardware.C
//      Purpose:    Hardware Interface Layer (modified for SDL2)
//      Author:     Paul Robson
//      Date:       1st January 2014
//
//      Modified:   6th May 2017 by Raph Koster
//
//*******************************************************************************************************
//*******************************************************************************************************

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "hardware.h"
#include "hwinterface.h"
#include "debugsc11.h"
#include <c:/MINGW/MINGW_DEV_LIB/INCLUDE/SDL2/SDL.h>

#include "font.h"                                                                       // 5 x 7 font data.

#define SOUND                                                                           // Sound enabled in this SDL interface

static BOOL isSoundOn = FALSE;                                                          // Sound status.
int  pitch = 440;                                                                       // Current sound pitch
static int cyclePos;
static SDL_Window *window;                                                              // Window
static SDL_Renderer* renderer;                                                          // Accelerated Renderer.

static float WIDTH = 800;                                                                // window size
static float HEIGHT = 600;                                                                // can be passed in as args

static int widthDisplay = 415;
static int heightDisplay = 415;

#define KEYCOUNT    (36)                                                                // 0-9 A-Z

static BOOL isKeyPressed[KEYCOUNT];                                                     // 36 key check values.
static int rotaryControl = 0;                                                           // Rotary Control position (0-100)
static int lastRotaryTimeCheck = 0;                                                     // Time of last check

// artwork
static SDL_Surface *overlay;                                                            // Overlay bitmap
static SDL_Texture *overlayTexture;                                                     // as a texture
static SDL_Surface *snap;                                                               // Snap bitmap
static SDL_Texture *snapTexture;                                                        // as a texture
static SDL_Surface *knob;                                                               // Knob bitmap
static SDL_Texture *knobTexture;                                                        // as a texture
static SDL_Surface *keyHints;                                                           // Key hints bitmap
static SDL_Texture *keyHintsTexture;                                                    // as a texture

static void audioCallback(void *_beeper, Uint8 *_stream, int _length);

//*******************************************************************************************************
//                          Initialise the Interface Layer
//*******************************************************************************************************

void IF_Initialise(int w,int h,int f)
{
    char buffer[64];
    if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO)<0)                                     // Initialise SDL
        exit(fprintf(stderr, "Unable to init SDL: %s\n", SDL_GetError() ));
    atexit(IF_Terminate);                                                               // Call terminate on the way out.
    sprintf(buffer,"Microvision Emulator (TMS1100) : Build %d",BUILD);
    if (w != 0 && h != 0) {                                                             // Set display size if nonzero
        WIDTH = w; HEIGHT = h;
    }

    // match the aspect ratio Paul set up, but maximize real estate
    heightDisplay = HEIGHT / 143*100;
    widthDisplay = heightDisplay * 1.014;

    // always open in windowed; something in SDL breaks rendering if you toggle fullscreen starting from fullscreen
    window = SDL_CreateWindow(buffer, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WIDTH, HEIGHT, 0);

    renderer = SDL_CreateRenderer(window, -1, 0);
    SDL_SetRelativeMouseMode(TRUE);                                                     // Grab mouse events.
    overlay = NULL;                                                                     // No overlay.
    snap = NULL;                                                                        // No snap.
    knob = NULL;                                                                        // No knob.

    // toggle to fullscreen now if it was set in the CLI args
    if (f == 1) IF_ToggleFullscreen();

    #ifdef SOUND
    SDL_AudioSpec desiredSpec;                                                          // Create an SDL Audio Specification.
    desiredSpec.freq = 44100;
    desiredSpec.format = AUDIO_S16SYS;
    desiredSpec.channels = 1;
    desiredSpec.samples = 2048;
    desiredSpec.callback = audioCallback;
    SDL_AudioSpec obtainedSpec;                                                         // Request the specification.
    SDL_OpenAudio(&desiredSpec, &obtainedSpec);
    isSoundOn = FALSE;
    IF_SetSound(FALSE);                                                                 // Sound off.
    #endif
}

//*******************************************************************************************************
//                      Render the interface layer - in debug mode, or not
//*******************************************************************************************************

BOOL IF_Render(BOOL debugMode)
{
    int key;
    SDL_Event event;
    BOOL quit = FALSE;
    while(SDL_PollEvent(&event))                                                        // Empty the event queue.
    {
        if (event.type == SDL_MOUSEMOTION) {
            rotaryControl = rotaryControl + event.motion.xrel / 2;
            if (rotaryControl < 0) rotaryControl = 0;
            if (rotaryControl > 100) rotaryControl = 100;
        }

        if (event.type == SDL_KEYDOWN) {
            key = event.key.keysym.sym;
            if (key == SDLK_RETURN)
                                IF_ToggleFullscreen();
        }

        if (event.type == SDL_KEYUP || event.type == SDL_KEYDOWN)                       // Is it a key event

        {
            key = event.key.keysym.sym;                                                 // This is the SDL Key Code
            if (key == SDLK_ESCAPE)                                                     // Esc key ends program.
                                quit = TRUE;

            if (key >= SDLK_0 && key <= SDLK_9) {
                ASSERT(RANGE(key-SDLK_0,0,9));
                isKeyPressed[key-SDLK_0] = (event.type == SDL_KEYDOWN);
            }
            if (key >= SDLK_a && key <= SDLK_z) {
                ASSERT(RANGE(key-SDLK_a+10,10,35));
                isKeyPressed[key-SDLK_a+10] = (event.type == SDL_KEYDOWN);
            }

        } // end switch
    } // end of message processing

    SDL_SetRenderDrawColor(renderer, 0, 0, 64, 255);
    SDL_RenderClear(renderer);
    if (debugMode) DB11_Draw();
    IF_RenderScreen(debugMode);
    SDL_RenderPresent(renderer);
    return quit;
}

//*******************************************************************************************************
//                                Toggle fullscreen
//*******************************************************************************************************

void IF_ToggleFullscreen(SDL_Window* Window) {
    Uint32 FullscreenFlag = SDL_WINDOW_FULLSCREEN;
    int IsFullscreen = SDL_GetWindowFlags(window) & FullscreenFlag;
    SDL_SetWindowFullscreen(window, IsFullscreen ? 0 : FullscreenFlag);
    SDL_FlushEvent(SDL_KEYDOWN);
}
//*******************************************************************************************************
//                                Load overlay if there is one.
//*******************************************************************************************************

void IF_LoadOverlay(char *binFile) {
    char buffer[128];
    if (strlen(binFile) < 5) return;

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");    // smooth image scaling

    // Must be something.bin at least !
    strcpy(buffer,binFile);strcpy(buffer+strlen(buffer)-3,"bmp");                   // Create a .bmp file name
    overlay = SDL_LoadBMP(buffer);                                                  // Try to load it.
    if (overlay != NULL)
        overlayTexture = SDL_CreateTextureFromSurface(renderer,overlay);

    // now do the same for the snap
    strcpy(buffer,binFile);strcpy(buffer+strlen(buffer)-4,"_snap");strcpy(buffer+strlen(buffer),".bmp");
    snap = SDL_LoadBMP(buffer);                                                     // Same for the snap
    if (snap != NULL)
        snapTexture = SDL_CreateTextureFromSurface(renderer,snap);

    // overlay of key hints for the buttons. Am lazy and made it same size as the snaps
    // that way we can just directly overdraw it
    // someday maybe add a key to toggle it?
    keyHints = SDL_LoadBMP("keyhints.bmp");
    if (keyHints != NULL)
        keyHintsTexture = SDL_CreateTextureFromSurface(renderer, keyHints);

    // and lastly, the knob art
    knob = SDL_LoadBMP("knob.bmp");
    if (knob != NULL)
        knobTexture = SDL_CreateTextureFromSurface(renderer, knob);
}

//*******************************************************************************************************
//                                          Render Screen
//*******************************************************************************************************

void IF_RenderScreen(BOOL debugMode)
{
    SDL_Rect rMain;
    rMain.w = WIDTH/4;rMain.h = rMain.w;
    rMain.x = WIDTH-rMain.w-8;rMain.y = 8;
    if (!debugMode) {
        // assumes landscape monitor...
        rMain.w = widthDisplay;
        rMain.h = heightDisplay;
        //int extra = WIDTH - (4 / 3 * HEIGHT);                               // total ugly hack for widescreen monitors.
        rMain.x = WIDTH - (widthDisplay * 121 / 100);
        rMain.y = HEIGHT / 2 - rMain.h / 2;
    }
    SDL_SetRenderDrawColor(renderer,160,160,160,255);
    SDL_RenderFillRect(renderer,&rMain);
    SDL_SetRenderDrawColor(renderer,48,48,48,255);
    for (int r = 0;r < 16;r++) {
        SDL_Rect rPixel;
        rPixel.w = rMain.w/16-1;
        rPixel.h = rMain.h/16-1;
        rPixel.x = rMain.x + rMain.w/2-(rPixel.w+1)*8;
        rPixel.y = rMain.y + (rPixel.h+1) * r + rMain.h/2 - (rPixel.h+1)*8;
        int pattern = HWIReadLCDRow(r);
        for (int c = 0;c < 16;c++) {
            if (pattern & 0x8000) SDL_RenderFillRect(renderer,&rPixel);
            else {
                if (debugMode)
                    SDL_RenderDrawRect(renderer,&rPixel);
            }
            rPixel.x += rPixel.w+1;
            pattern = (pattern << 1) & 0xFFFF;
        }
    }

    int playfieldAndOverlayWidth = 0;
    if (!debugMode && overlay != NULL) {                                        // Draw the overlay in non-debug mode
        SDL_Rect rcDisplay;
        rcDisplay.w = rMain.w * 141 / 100;
        rcDisplay.h = rMain.h * 143 / 100;
        rcDisplay.x = rMain.x + rMain.w / 2 - rcDisplay.w/2;
        rcDisplay.y = rMain.y + rMain.h / 2 - rcDisplay.h/2;
        playfieldAndOverlayWidth = rcDisplay.x;             //
        SDL_RenderCopy(renderer,overlayTexture,NULL,&rcDisplay);
    }

    if (!debugMode && snap != NULL && keyHints != NULL) {                       // Draw the snap in non-debug mode
        SDL_Rect rcDisplay;
        rcDisplay.h = rMain.h * 143 / 100;
        rcDisplay.w = snap->w * rcDisplay.h / snap->h;
        rcDisplay.x = 0;
        if (rcDisplay.w > playfieldAndOverlayWidth ) {      // scale the snap down so it doesn't overlap the overlay
            rcDisplay.h = rcDisplay.h * playfieldAndOverlayWidth/rcDisplay.w;
            rcDisplay.w = playfieldAndOverlayWidth;
        } else {
            rcDisplay.x =(playfieldAndOverlayWidth - rcDisplay.w)/2;
        }

        rcDisplay.y = rMain.y + rMain.h / 2 - rcDisplay.h/2;
        SDL_RenderCopy(renderer,snapTexture,NULL,&rcDisplay);
        SDL_RenderCopy(renderer,keyHintsTexture,NULL,&rcDisplay);               // overlay keymapping hints on the snap
    }

    // old paddle renderer
    rMain.h = rMain.h / 16;
    rMain.y = rMain.y + rMain.h * 17;
    if (!debugMode) {
        rMain.y = HEIGHT - rMain.h - 32;
        rMain.h = rMain.h / 2;
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);              // alphablend if not in debug
    }
    SDL_SetRenderDrawColor(renderer,192,192,192,128);
    SDL_RenderFillRect(renderer,&rMain);

     // new paddle knob
    if (!debugMode && knob != NULL) {                    // Draw the knob in non-debug mode
        SDL_Rect rcDisplay;
        int knobSize = HEIGHT/8;
        rcDisplay.w = knobSize;
        rcDisplay.h = rcDisplay.w;
        rcDisplay.x = rMain.x + rMain.w/2 - knobSize/2;
        rcDisplay.y = HEIGHT - knobSize - 2;
        SDL_RenderCopyEx(renderer,knobTexture,NULL,&rcDisplay, rotaryControl*2.7, NULL, SDL_FLIP_NONE);
    }

    rMain.x = rMain.x + rMain.w * rotaryControl / 100;
    rMain.w = 6;rMain.x -= rMain.w/2;rMain.y += 1;rMain.h -= 2;
    SDL_SetRenderDrawColor(renderer,0,0,0,128);
    SDL_RenderFillRect(renderer,&rMain);
}

//*******************************************************************************************************
//                      Write Character to screen square (x,y) - 32 x 24
//*******************************************************************************************************

void IF_Write(int x,int y,char ch,int colour)
{
    int xCSize = WIDTH / 32;                                                            // Work out character box size.
    int yCSize = HEIGHT / 23;
    SDL_Rect rc;
    rc.x = xCSize * x;rc.y = yCSize * y;                                                // Erase character background.
    rc.w = xCSize;rc.h = yCSize;
    SDL_SetRenderDrawColor(renderer,0,0,64,255);
    SDL_RenderFillRect(renderer,&rc);
    if (ch <= ' ' || ch > 127) return;                                                  // Don't render control and space.
    unsigned char *byteData = fontdata + (int)((ch - ' ') * 5);                         // point to the font data
    int xp,yp,pixel;
    rc.w = xCSize * 16 / 100;                                                           // Work out pixel sizes
    rc.h = yCSize * 12 / 100;
    SDL_SetRenderDrawColor(renderer,(colour & 1) ? 255:0,(colour & 2) ? 255:0,(colour & 4) ? 255:0,255);
    for (xp = 0;xp < 5;xp++)                                                            // Font data is stored vertically
    {
        rc.x = xp * rc.w + x * xCSize;                                                  // Horizontal value
        pixel = *byteData++;                                                            // Pixel data for vertical line.
        for (yp = 0;yp < 7;yp++)                                                        // Work through pixels.
        {
            if (pixel & (1 << yp))                                                      // Bit 0 is the top pixel, if set.
            {
                rc.y = yp * rc.h + y * yCSize;                                          // Vertical value
                SDL_RenderFillRect(renderer,&rc);                                       // Draw Cell.
            }
        }
    }
}

//*******************************************************************************************************
//                              Terminate the interface layer
//*******************************************************************************************************

void IF_Terminate(void)
{
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

//*******************************************************************************************************
//                              Check to see if a key is pressed
//*******************************************************************************************************

BOOL IF_KeyPressed(char ch)
{
    if (!isalnum(ch)) return FALSE;
    if (isdigit(ch)) return isKeyPressed[ch - '0'];
    return isKeyPressed[toupper(ch)-'A'+10];
}

//*******************************************************************************************************
//                                    Control sound.
//*******************************************************************************************************

void IF_SetSound(BOOL isOn)
{
    if (isSoundOn == isOn) return;                                                      // No status change.
    isSoundOn = isOn;                                                                   // Update status
    #ifdef SOUND
    SDL_PauseAudio(isOn == 0);                                                          // If sound built in, turn on/off.
    #endif
}

void IF_SetPitch(int frequency)
{
    pitch = frequency;
}

//*******************************************************************************************************
//                              Audio Callback Function
//*******************************************************************************************************

static void audioCallback(void *_beeper, Uint8 *_stream, int _length)
{
    Sint16 *stream = (Sint16*) _stream;                                                 // Pointer to audio data
    int length = _length / 2;                                                           // Length of audio data
    int i;
    for (i = 0;i < length;i++)                                                          // Fill buffer with data
    {
        stream[i] = (cyclePos > 22050 ? -32767:32767);                                  // Square Wave
        cyclePos = (cyclePos + pitch) % 44100;
    }
}

//*******************************************************************************************************
//                  Get Tick Timer - needs about a 20Hz minimum granularity.
//*******************************************************************************************************

int IF_GetTime(void)
{
    return SDL_GetTicks();
}

//*******************************************************************************************************
//                                 Process Rotary Control moves every frame
//*******************************************************************************************************

void IF_ProcessRotaryControl() {
    int currentTime = IF_GetTime();                                                     // Current ms time
    int elapsed = currentTime - lastRotaryTimeCheck;                                    // Time since last check.
    lastRotaryTimeCheck = currentTime;                                                  // Update last check.
    if (elapsed > 100) elapsed = 100;                                                   // If paused for some reason.

    elapsed = elapsed / 10;
    if (IF_KeyPressed('O')) rotaryControl -= elapsed;
    if (IF_KeyPressed('P')) rotaryControl += elapsed;

    if (rotaryControl > 100) rotaryControl = 100;                                       // Force into range
    if (rotaryControl < 0) rotaryControl = 0;
}

//*******************************************************************************************************
//                                          Synchronise system.
//*******************************************************************************************************

static int lastSync = 0;

void XHWISynchronise(int milliseconds) {
    int syncTime = lastSync + milliseconds;                                             // Synchronise to this time.
    while (IF_GetTime() < syncTime) {}                                                  // Wait for time out.
    lastSync = IF_GetTime();                                                            // Remember time of last sync.
}

//*******************************************************************************************************
//                                          Test status of a key
//*******************************************************************************************************

static char * _keys[] = { "1QAZ","2WSX","3EDC" };

BYTE8 XHWIReadKeypad(int col,int row) {
    return IF_KeyPressed(_keys[col][row]);
}

//*******************************************************************************************************
//                                          Set Sound Pitch, 0 = Off
//*******************************************************************************************************

void XHWISetPitch(int freq) {
    if (freq == 0)
        IF_SetSound(FALSE);
    else {
        IF_SetPitch(freq);
        IF_SetSound(TRUE);
    }
}

//*******************************************************************************************************
//                              Get Rotary Control Value (-1 if not supported)
//*******************************************************************************************************

int XHWIGetRotaryControl(void) {
    return rotaryControl;
}

