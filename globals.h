/*
  Copyright (C) 2006-2011 Nasca Octavian Paul
  Author: Nasca Octavian Paul

  This program is free software; you can redistribute it and/or modify
  it under the terms of version 2 of the GNU General Public License 
  as published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License (version 2) for more details.

  You should have received a copy of the GNU General Public License (version 2)
  along with this program; if not, write to the Free Software Foundation,
  Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
*/
#ifndef GLOBALS_H
#define GLOBALS_H

#define REALTYPE float

#ifndef NULL
#define NULL 0
#endif

void sleep(int ms);

#define ZERO(data,size) {char *data_=(char *) data;for (int i=0;i<size;i++) data_[i]=0;};

enum FILE_TYPE{
    FILE_WAV,FILE_VORBIS,FILE_MP3
};


#endif

