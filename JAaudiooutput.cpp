/*
  JAaudiooutput.C - Audio output for JACK
  Copyright (C) 2002-2009 Nasca Octavian Paul
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

#ifdef HAVE_JACK

#ifndef DEFAULT_RB_SIZE
#define DEFAULT_RB_SIZE 16384
#endif

#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <jack/jack.h>
#include <jack/ringbuffer.h>
#include "JAaudiooutput.h"

Player *myplayer=NULL;
jack_client_t *j_client = NULL;
jack_port_t **j_output_port = NULL;
jack_default_audio_sample_t **j_out = NULL;
jack_ringbuffer_t *rb = NULL;

int m_channels = 2;
int thread_run = 0;

pthread_t player_thread_id;
pthread_mutex_t player_thread_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  buffer_ready = PTHREAD_COND_INITIALIZER;

#ifdef ENABLE_RESAMPLING
#include <samplerate.h>
float m_fResampleRatio = 1.0;

#ifndef SRC_QUALITY // alternatives: SRC_SINC_MEDIUM_QUALITY, SRC_SINC_BEST_QUALITY, (SRC_ZERO_ORDER_HOLD, SRC_LINEAR)
#define SRC_QUALITY SRC_SINC_FASTEST
#endif

#endif

void jack_shutdown_callback(void *arg){
    fprintf(stderr, "jack server [unexpectedly] shut down.\n");
    j_client=NULL;
    JACKclose();
}

int jack_audio_callback(jack_nframes_t nframes, void *arg){
    int c,s;

    for(c=0; c< m_channels; c++) {
	j_out[c] = (jack_default_audio_sample_t*) jack_port_get_buffer(j_output_port[c], nframes);
    }

    if(jack_ringbuffer_read_space(rb) < m_channels * nframes * sizeof(jack_default_audio_sample_t)) {
	for(c=0; c< m_channels; c++) {
	    memset(j_out[c], 0, nframes * sizeof(jack_default_audio_sample_t));
	}
    } else {
	/* de-interleave */
	for(s=0; s<nframes; s++) {
	    for(c=0; c< m_channels; c++) {
		jack_ringbuffer_read(rb, (char*) &j_out[c][s], sizeof(jack_default_audio_sample_t));
	    }
	}
    }

    /* Tell the player thread there is work to do. */
    if(pthread_mutex_trylock(&player_thread_lock) == 0) {
	pthread_cond_signal(&buffer_ready);
	pthread_mutex_unlock(&player_thread_lock);
    }

    return(0);
};

void *jack_player_thread(void *){
    const int nframes = 4096;
    float *tmpbuf = (float*) calloc(nframes * m_channels, sizeof(float));
    float *bufptr = tmpbuf;
#ifdef ENABLE_RESAMPLING
    SRC_STATE* src_state = src_new(SRC_QUALITY, 2, NULL);
    SRC_DATA src_data;
    int nframes_r = floorf((float) nframes*m_fResampleRatio); ///< # of frames after resampling
    float *smpbuf = (float*) calloc((1+nframes_r) * m_channels, sizeof(float));

    src_data.input_frames  = nframes;
    src_data.output_frames = nframes_r;
    src_data.end_of_input  = 0;
    src_data.src_ratio     = m_fResampleRatio;
    src_data.input_frames_used = 0;
    src_data.output_frames_gen = 0;
    src_data.data_in       = tmpbuf;
    src_data.data_out      = smpbuf;
#else
    int nframes_r = nframes;
#endif

    size_t rbsize = nframes_r * m_channels * sizeof(float);

    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
    pthread_mutex_lock(&player_thread_lock);

    while(thread_run) {
	myplayer->getaudiobuffer(nframes, tmpbuf);

#ifdef ENABLE_RESAMPLING
	if(m_fResampleRatio != 1.0) {
	    src_process(src_state, &src_data);
	    bufptr = smpbuf;
	    rbsize = src_data.output_frames_gen * m_channels * sizeof(float);
	}
#endif
	jack_ringbuffer_write(rb, (char *) bufptr, rbsize);

	while(thread_run && jack_ringbuffer_write_space(rb) <= rbsize) {
	    pthread_cond_wait(&buffer_ready, &player_thread_lock);
	}
    }

    pthread_mutex_unlock(&player_thread_lock);
    free(tmpbuf);
#ifdef ENABLE_RESAMPLING
    src_delete(src_state);
    free(smpbuf);
#endif
}

void JACKaudiooutputinit(Player *player_, int samplerate){
    int i;
    myplayer=player_;

    if(j_client || thread_run || rb) {
	fprintf(stderr, "already connected to jack.\n");
	return;
    }

    j_client = jack_client_open("paulstretch", (jack_options_t) 0, NULL);

    if(!j_client) {
	fprintf(stderr, "could not connect to jack.\n");
	return;
    }	

    jack_on_shutdown(j_client, jack_shutdown_callback, NULL);
    jack_set_process_callback(j_client, jack_audio_callback, NULL);

    j_output_port=(jack_port_t**) calloc(m_channels, sizeof(jack_port_t*));

    for(i=0;i<m_channels;i++) {
	char channelid[16];
	snprintf(channelid, 16, "output_%i", i);
	j_output_port[i] = jack_port_register(j_client, channelid, JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
	if(!j_output_port[i]) {
	    fprintf(stderr, "no more jack ports availabe.\n");
	    JACKclose();
	    return;
	}
    }

    j_out = (jack_default_audio_sample_t**) calloc(m_channels, sizeof(jack_default_audio_sample_t*));
    const size_t rbsize = DEFAULT_RB_SIZE * m_channels * sizeof(jack_default_audio_sample_t);
    rb = jack_ringbuffer_create(rbsize);
    memset(rb->buf, 0, rbsize);

    jack_nframes_t jsr = jack_get_sample_rate(j_client);
    if(jsr != samplerate) {
#ifdef ENABLE_RESAMPLING
	m_fResampleRatio = (float) jsr / (float) samplerate;
#else
	fprintf(stderr, "Note: paulstretch audio samplerate does not match JACK's samplerate.\n");
#endif
    }

    thread_run = 1;
    pthread_create(&player_thread_id, NULL, jack_player_thread, NULL);
    pthread_yield();

    jack_activate(j_client);

    char *jack_autoconnect = getenv("JACK_AUTOCONNECT");
    if(!jack_autoconnect) {
	jack_autoconnect = (char*) "system:playback_";
    } else if(!strncmp(jack_autoconnect,"DISABLE", 7)) {
	jack_autoconnect = NULL;
    }
    if(jack_autoconnect) {
	int myc=0;
	const char **found_ports = jack_get_ports(j_client, jack_autoconnect, NULL, JackPortIsInput);
	for(i = 0; found_ports && found_ports[i]; i++) {
	    if(jack_connect(j_client, jack_port_name(j_output_port[myc]), found_ports[i])) {
		fprintf(stderr, "can not connect to jack output\n");
	    }
	    if(myc >= m_channels) break;
	}
    }
}

void JACKclose(){
    if(thread_run) {
	thread_run = 0;
	if(pthread_mutex_trylock(&player_thread_lock) == 0) {
	    pthread_cond_signal(&buffer_ready);
	    pthread_mutex_unlock(&player_thread_lock);
	}
	pthread_join(player_thread_id, NULL);
    }
    if(j_client){
	jack_client_close(j_client);
    }
    if(j_output_port) {
	free(j_output_port);
	j_output_port=NULL;
    }
    if(j_out) {
	free(j_out);
	j_out = NULL;
    }
    if(rb) {
	jack_ringbuffer_free(rb);
	rb=NULL;
    }
    j_client=NULL;
};

#endif /* HAVE_JACK */

/* vim: set ts=8 sw=4: */
