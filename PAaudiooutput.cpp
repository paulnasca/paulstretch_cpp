/*
  PAaudiooutput.C - Audio output for PortAudio
  Copyright (C) 2002-2009 Nasca Octavian Paul
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

#include <stdlib.h>
#include "PAaudiooutput.h"

Player *player=NULL;
PaStream *stream=NULL;

static int PAprocess(const void *inputBuffer,void *outputBuffer,
		unsigned long framesPerBuffer,
		const PaStreamCallbackTimeInfo *outTime,PaStreamCallbackFlags statusFlags,void *userData){
	float *out=(float *)outputBuffer;
    player->getaudiobuffer(framesPerBuffer,out);

    return(0);
};

void PAaudiooutputinit(Player *player_,int samplerate){
    player=player_;
    if (stream) return;
    Pa_Initialize();
    Pa_OpenDefaultStream(&stream,0,2,paFloat32,samplerate,PA_SOUND_BUFFER_SIZE,PAprocess,NULL);
    Pa_StartStream(stream);
};

void PAfinish(){
    if (stream){
	Pa_StopStream(stream);
	Pa_CloseStream(stream);
	Pa_Terminate();
    };
    stream=NULL;

};




