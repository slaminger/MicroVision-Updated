This is the source for compiling new versions of the emulator.

To build the windows version, set up Codeblocks and SDL2 as described here (except SDL is currently v2.0.1):

http://lazyfoo.net/tutorials/SDL/01_hello_SDL/windows/codeblocks/index.php



And the included CPB (code::blocks project) in /MicrovisionEmu should work if you copy the source files (.c .h) from the Mac source.



Be sure to edit the path to your install of SDL2; it's an include in hardware.c.

I haven't messed with anything else that is in this folder, really.
