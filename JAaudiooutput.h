/*
  JAaudiooutput.h - Audio output for JACK
  Copyright (C) 2002 Nasca Octavian Paul
  Author: Robin Gareus <robin@gareus.org>

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

#ifndef JACK_AUDIO_OUTPUT_H
#define JACK_AUDIO_OUTPUT_H

#ifdef HAVE_JACK 

#include "Player.h"


void JACKaudiooutputinit(Player *player_,int samplerate);
void JACKclose();

#endif
#endif

/* vim: set ts=8 sw=4: */
