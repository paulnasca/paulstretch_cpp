outfile=paulstretch.exe

rm -f $outfile
wine /usr/local/i586-mingw32/bin/fluid.exe -c GUI.fl 
wine /usr/local/i586-mingw32/bin/fluid.exe -c FreeEditUI.fl 

clear 

i586-mingw32msvc-g++ -O3 -DWINDOWS -DKISSFFT -I./contrib GUI.cxx FreeEditUI.cxx *.cpp Input/*.cpp Output/*.cpp contrib/*.c \
`/usr/local/i586-mingw32/bin/fltk-config --cflags` \
`/usr/local/i586-mingw32/bin/fltk-config --ldflags` \
/usr/local/i586-mingw32/lib/libvorbisenc.a \
/usr/local/i586-mingw32/lib/libvorbisfile.a \
/usr/local/i586-mingw32/lib/libvorbis.a \
/usr/local/i586-mingw32/lib/libogg.a \
/usr/local/i586-mingw32/lib/libportaudio.a \
/usr/local/i586-mingw32/lib/libaudiofile.a \
/usr/local/i586-mingw32/lib/libmad.a \
/usr/local/i586-mingw32/lib/libmxml.a \
/usr/local/i586-mingw32/lib/libz.a \
-lm -lwinmm -o $outfile

rm -f GUI.h GUI.cxx FreeEditUI.h FreeEditUI.cxx

strip $outfile

cat version.h | grep -v "#"

#compress the outfile (not necessary, but useful)
#upx $outfile













