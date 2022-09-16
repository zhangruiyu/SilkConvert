#include "easy_mad.h"

#define MP3_BUF_SIZE 4096
#define MP3_FRAME_SIZE 2881

typedef struct buffer {
    FILE *fin;
    FILE *fout;
    char mp3_buf[MP3_BUF_SIZE];
    unsigned char const *start;
    unsigned long length;
} Mp3_Handle;

static inline
signed int scale(mad_fixed_t sample)
{
    /* round */
    sample += (1L << (MAD_F_FRACBITS - 16));
    /* clip */
    if (sample >= MAD_F_ONE)
        sample = MAD_F_ONE - 1;
    else if (sample < -MAD_F_ONE)
        sample = -MAD_F_ONE;
    /* quantize */
    return sample >> (MAD_F_FRACBITS + 1 - 16);
}

static
enum mad_flow output(void *data,
                     struct mad_header const *header,
                     struct mad_pcm *pcm)
{
    unsigned int nchannels, nsamples, rate;
    mad_fixed_t const *left_ch, *right_ch;
    rate = pcm->samplerate;
    nchannels = pcm->channels;
    nsamples  = pcm->length;
    left_ch   = pcm->samples[0];
    right_ch  = pcm->samples[1];


    int len = nsamples*2*nchannels;
    char wbuf[len];
    char * wptr = wbuf;
    while (nsamples--) {
        signed int sample;
        sample = scale(*left_ch++);
        *(wptr++) = ((sample >> 0) & 0xff);
        *(wptr++) = ((sample >> 8) & 0xff);
        if (nchannels == 2) {
            sample = scale(*right_ch++);
            *(wptr++) = ((sample >> 0) & 0xff);
            *(wptr++) = ((sample >> 8) & 0xff);
        }
    }
    Mp3_Handle * handle = (Mp3_Handle *) data;
    fwrite(wbuf,len,1,handle->fout);

    return MAD_FLOW_CONTINUE;
}

static
enum mad_flow error(void *data,
                    struct mad_stream *stream,
                    struct mad_frame *frame)
{
    struct buffer *buffer = data;
    fprintf(stderr, "decoding error 0x%04x (%s) at byte offset %u\n",
            stream->error, mad_stream_errorstr(stream),
            stream->this_frame - buffer->start);
    return MAD_FLOW_CONTINUE;
}
