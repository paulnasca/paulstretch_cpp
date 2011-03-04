outfile=paulstretch.exe

rm -f $outfile

mingw_dir="/usr/i586-mingw32msvc"

wine "$mingw_dir/bin/fluid.exe" -c GUI.fl 
wine "$mingw_dir/bin/fluid.exe" -c FreeEditUI.fl 

clear 

i586-mingw32msvc-g++ -O3 -DWINDOWS -DKISSFFT -I./contrib GUI.cxx FreeEditUI.cxx *.cpp Input/*.cpp Output/*.cpp contrib/*.c \
`"$mingw_dir/bin/fltk-config" --cflags` \
`"$mingw_dir/bin/fltk-config" --ldflags` \
"$mingw_dir/lib/libvorbisenc.a" \
"$mingw_dir/lib/libvorbisfile.a" \
"$mingw_dir/lib/libvorbis.a" \
"$mingw_dir/lib/libogg.a" \
"$mingw_dir/lib/libportaudio.a" \
"$mingw_dir/lib/libaudiofile.a" \
"$mingw_dir/lib/libmad.a" \
"$mingw_dir/lib/libmxml.a" \
"$mingw_dir/lib/libz.a" \
-lm -lwinmm -o $outfile

rm -f GUI.h GUI.cxx FreeEditUI.h FreeEditUI.cxx

strip $outfile

cat version.h | grep -v "#"

#compress the outfile (not necessary, but useful)
#upx $outfile













