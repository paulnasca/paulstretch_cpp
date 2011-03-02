/*
  Copyright (C) 2011 Nasca Octavian Paul
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
#ifndef PLAYER_H
#define PLAYER_H

#include <string>
#include "Input/AInputS.h"
#include "Input/VorbisInputS.h"
#include "Input/MP3InputS.h"
#include "ProcessedStretch.h"
#include "Thread.h"
#include "BinauralBeats.h"
#include "Mutex.h"
#include "globals.h"

#define PA_SOUND_BUFFER_SIZE 8192


class Player:public Thread{
    public:
	Player();
	~Player();
	
	void startplay(std::string filename, REALTYPE startpos,REALTYPE rap, int fftsize,FILE_TYPE intype,bool bypass,ProcessParameters *ppar,BinauralBeatsParameters *bbpar);
	    //startpos is from 0 (start) to 1.0 (end of file)
	void stop();
	void pause();
	
	void freeze();
	void setrap(REALTYPE newrap);
	
	void seek(REALTYPE pos);
	
	void getaudiobuffer(int nsamples, float *out);//data este stereo
	
	enum ModeType{
	    MODE_PLAY,MODE_STOP,MODE_PREPARING,MODE_PAUSE
	};
	
	ModeType getmode();
	
	struct{
	    float position;//0 is for start, 1 for end
	    int playing;
	    int samplerate;
	    bool eof;
	}info;

	bool is_freeze(){
	    return freeze_mode;
	};
	void set_window_type(FFTWindow window);
	
	void set_volume(REALTYPE vol);
	void set_onset_detection_sensitivity(REALTYPE onset);
	
	void set_process_parameters(ProcessParameters *ppar,BinauralBeatsParameters *bbpar);
	
	BinauralBeats *binaural_beats;

    private:
	void run();
    
	InputS *ai;
	ProcessedStretch *stretchl,*stretchr;

	short int *inbuf_i;
	int inbufsize;

	Mutex taskmutex,bufmutex;
	
	ModeType mode;
	
	enum TaskMode{
	    TASK_NONE, TASK_START, TASK_STOP,TASK_SEEK,TASK_RAP,TASK_PARAMETERS, TASK_ONSET
	};
	struct {
	    TaskMode mode;

	    REALTYPE startpos;
	    REALTYPE rap;
	    int fftsize;
	    std::string filename;
	    FILE_TYPE intype;
	    bool bypass;
	    ProcessParameters *ppar;
	    BinauralBeatsParameters *bbpar;
	}newtask,task;
	
	struct{
	    REALTYPE *l,*r;
	}inbuf;
	
	struct{
	    int n;//how many buffers
	    float **datal,**datar;//array of buffers
	    int size;//size of one buffer
	    int computek,outk;//current buffer
	    int outpos;//the sample position in the current buffer (for out)
	    int nfresh;//how many buffers are fresh added and need to be played
		    //nfresh==0 for empty buffers, nfresh==n-1 for full buffers
	    float *in_position;//the position(for input samples inside the input file) of each buffers
	}outbuf;
	bool first_in_buf;
	
	void newtaskcheck();
	void computesamples();
	bool freeze_mode,bypass_mode,paused;
	REALTYPE volume,onset_detection_sensitivity;

	std::string current_filename;
	FFTWindow window_type;
};

#endif

