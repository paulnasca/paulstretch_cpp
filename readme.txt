PaulStretch
Copyright (C) 2006-2011 Nasca Octavian Paul, Tg. Mures, Romania

Released under GNU General Public License v.2 license

This is an experimental program for extreme stretching the audio.
Requirements:
    - audiofile library
    - libvorbis
    - fltk library
    - portaudio library
    - libmad (for mp3 input)
    - mxml library (for saving/loading parameters)
    - not required, but you can use the FFTW library


This algorithm/program is suitable only for extreme stretching the audio. 

Tips:
	You can change the default output device with "PA_RECOMMENDED_OUTPUT_DEVICE" environment variable (used by PortAudio).
	eg: set PA_RECOMMENDED_OUTPUT_DEVICE=1  #where 1 represents the index of the device; you can try other values for other devices

History:
    20060527(0.0.1)
	  - First release

    20060530(0.0.2)
	  - Ogg Vorbis output support
	  - Added a wxWidgets graphical user interface

    20060812(1.000)
	  - Removed the wxWidgets GUI and added a FLTK GUI (because FLTK GUI is smaller)
	  - Added real-time processing/player
	  - Added input support for Ogg Vorbis files
	  - Improved the stretch algorithm and now the amount of stretch is unlimited (and on big stretch amounts, you don't need additional memory)
	  - Added "Freeze" button to the player
	  - It is possible to render to file only a selected part of the sound
	  - Other improvements    

    20060905(1.024)
	  - Added MP3 support for input
	  - Added bypass mode (if you click play with the right mouse button)
	  - Improved the precision of the position slider (now it shows really what's currenly playing)
	  - Added the possibility to set the stretch amount by entering the numeric value
	  - Added pause mode and volume control
	  - Added post-processing of the spectrum(pitch/frequency shift, octave mixer, compress,filter,harmonics)
	  - Command line parameter for input filename 
	  - The file can be dragged from the explorer to the file text to open it

    20090424(2.0)
	  - Added free envelopes, which allows the user to freely edit some parameters
	  - Added stretch multiplier (with free envelope) which make the stretching variable
	  - Added arbitrary frequency filter
	  - Added a frequency spreader effect, which increase the bandwith of each harmonic
	  - Added a frequency shifter which produces binaural beats (the beats frequencies are variable)
	  - Added 32 bit WAV rendering
	  - Other improvements and bugfixes
	
    20110210(2.1)
	  - Added loading/saving parameters
	  - Added Linux Jack support (thanks to Robin Gareus for the patch)
	  - Added "Symmetric" mode of Binaural Beats
	  - Support for longer stretches - for the really patient ones - up to one quintillion times  ( 10^18 x ) ;-)
	  - Fixed a bug which produced infinite loop at the end of some mp3 files (at playing or render)
	  - Fixed a bug in the mp3 reader
	  - other minor additions

    20110211(2.1-0)
          - Increased the precision of a paremeter for extreme long stretches
    
    20110303(2.2)
          - Improved the stretching algorithm, adding the onset detection
          - Shorten algorithm improvements 
          - Added an option to preserve the tonal part or noise part
	  - Ignored the commandline parameters starting with "-" (usefull for macosx)

    20110305(2.2-0)
          - gzip bugfix which prevents loading of the .psx files on Windows 
          - bugfix on freeze function on onsets

    20110305(2.2-1)
	  - removed the noise on starting/seeking with the player
          - bugfix on freeze function 

    20110306(2.2-2)
	  - buffer error on render

Enjoy! :)
Paul

zynaddsubfx_AT_yahoo com


    
