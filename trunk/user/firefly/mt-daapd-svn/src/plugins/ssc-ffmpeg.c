/*
 * $Id: $
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#ifdef WIN32
#include <windows.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>

#include "ff-plugins.h"

#ifndef TRUE
# define TRUE 1
# define FALSE 0
#endif

#if LIBAVUTIL_VERSION_INT < AV_VERSION_INT(50,38,0)
# define AV_SAMPLE_FMT_S16 SAMPLE_FMT_S16
# define AV_SAMPLE_FMT_S32 SAMPLE_FMT_S32
#endif

#define BUFFER_SIZE (AVCODEC_MAX_AUDIO_FRAME_SIZE * 3)/2

typedef struct tag_ssc_handle {
    AVCodec *pCodec;
    AVCodecContext *pCodecCtx;
    AVFormatContext *pFmtCtx;
    AVFrame *pFrame;
    AVPacket packet;
    AVInputFormat *pFormat;

    uint8_t *packet_data;
    int packet_size;
    int audio_stream;

    char* buffer;

    char *buf_remainder;
    int buf_remainder_len;
    int first_frame;

    int duration;

    int total_decoded;
    int total_written;

    int errnum;
    int swab;

    char *error;

    int channels;
    int sample_rate;
    int bits_per_sample;
    uint32_t samples;

    char wav_header[44];
    int wav_offset;
} SSCHANDLE;

#define SSC_FFMPEG_E_SUCCESS      0
#define SSC_FFMPEG_E_BADCODEC     1
#define SSC_FFMPEG_E_CODECOPEN    2
#define SSC_FFMPEG_E_FILEOPEN     3
#define SSC_FFMPEG_E_NOSTREAM     4
#define SSC_FFMPEG_E_NOAUDIO      5

char *ssc_ffmpeg_errors[] = {
    "Success",
    "Don't have appropriate codec",
    "Can't open codec",
    "Cannot open file",
    "Cannot find any streams",
    "No audio streams"
};


/* Forwards */
void *ssc_ffmpeg_init(void);
void ssc_ffmpeg_deinit(void *pv);
int ssc_ffmpeg_open(void *pv, MP3FILE *pmp3);
int ssc_ffmpeg_close(void *pv);
int ssc_ffmpeg_read(void *pv, char *buffer, int len);
char *ssc_ffmpeg_error(void *pv);

/* Globals */
PLUGIN_TRANSCODE_FN _ptfn = {
    ssc_ffmpeg_init,
    ssc_ffmpeg_deinit,
    ssc_ffmpeg_open,
    ssc_ffmpeg_close,
    ssc_ffmpeg_read,
    ssc_ffmpeg_error
};

PLUGIN_INFO _pi = {
    PLUGIN_VERSION,        /* version */
    PLUGIN_TRANSCODE,      /* type */
    "ssc-ffmpeg/" VERSION, /* server */
    NULL,                  /* output fns */
    NULL,                  /* event fns */
    &_ptfn,                /* fns */
    NULL,                  /* rend info */
    "flac,alac"            /* codeclist */
};

char *ssc_ffmpeg_error(void *pv) {
    SSCHANDLE *handle = (SSCHANDLE*)pv;

    return ssc_ffmpeg_errors[handle->errnum];
}

PLUGIN_INFO *plugin_info(void) {
    av_register_all();

    return &_pi;
}

void *ssc_ffmpeg_init(void) {
    SSCHANDLE *handle;

    handle=(SSCHANDLE *)malloc(sizeof(SSCHANDLE));
    if(handle) {
        memset(handle,0,sizeof(SSCHANDLE));

        handle->buffer = (char *)av_malloc(BUFFER_SIZE + 16);
        if (!handle->buffer) {
            free(handle);
            return NULL;
        }

        memset(handle->buffer, 0, BUFFER_SIZE + 16);
    }

    return (void*)handle;
}

void ssc_ffmpeg_deinit(void *vp) {
    SSCHANDLE *handle = (SSCHANDLE *)vp;
    ssc_ffmpeg_close(handle);
    if(handle) {
        av_free(handle->buffer);
        free(handle);
    }

    return;
}

int ssc_ffmpeg_open(void *vp, MP3FILE *pmp3) {
    int i;
    SSCHANDLE *handle = (SSCHANDLE*)vp;
#ifdef WIN32
    WCHAR utf16_path[_MAX_PATH+1];
    WCHAR utf16_mode[3];
#endif
    char *file;
    char *codec;
    int duration;

    file = pmp3->path;
    codec = pmp3->codectype;
    duration = pmp3->song_length;

    if(!handle)
        return FALSE;

    handle->duration = duration;
    handle->first_frame = 1;

    pi_log(E_DBG,"opening file with format\n");
#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(53, 2, 0)
    if(av_open_input_file(&handle->pFmtCtx, file, handle->pFormat, 0, NULL) < 0) {
#else
    if(avformat_open_input(&handle->pFmtCtx, file, handle->pFormat, NULL) < 0) {
#endif
        handle->errnum = SSC_FFMPEG_E_FILEOPEN;
        return FALSE;
    }

    handle->pFmtCtx->max_analyze_duration = 300000;

    /* find the streams */
#if LIBAVFORMAT_VERSION_INT < AV_VERSION_INT(53, 5, 0)
    if(av_find_stream_info(handle->pFmtCtx) < 0) {
#else
    if(avformat_find_stream_info(handle->pFmtCtx, NULL) < 0) {
#endif
        handle->errnum = SSC_FFMPEG_E_NOSTREAM;
        return FALSE;
    }

    //    dump_format(handle->pFmtCtx,0,file,FALSE);

    handle->audio_stream = -1;
    for(i=0; i < handle->pFmtCtx->nb_streams; i++) {
#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(52, 64, 0)
        if(handle->pFmtCtx->streams[i]->codec->codec_type==CODEC_TYPE_AUDIO) {
#else
        if(handle->pFmtCtx->streams[i]->codec->codec_type==AVMEDIA_TYPE_AUDIO) {
#endif
            handle->audio_stream = i;
            break;
        }
    }

    if(handle->audio_stream == -1) {
        handle->errnum = SSC_FFMPEG_E_NOAUDIO;
        return FALSE;
    }

    handle->pCodecCtx = handle->pFmtCtx->streams[handle->audio_stream]->codec;

    handle->pCodec = avcodec_find_decoder(handle->pCodecCtx->codec_id);
    if(!handle->pCodec) {
        handle->errnum = SSC_FFMPEG_E_BADCODEC;
        return FALSE;
    }

    if(handle->pCodec->capabilities & CODEC_CAP_TRUNCATED)
        handle->pCodecCtx->flags |= CODEC_FLAG_TRUNCATED;

#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(53, 5, 0)
    if(avcodec_open(handle->pCodecCtx, handle->pCodec) < 0) {
#else
    if(avcodec_open2(handle->pCodecCtx, handle->pCodec, NULL) < 0) {
#endif
        handle->errnum = SSC_FFMPEG_E_CODECOPEN;
        return FALSE;
    }

    handle->pFrame = avcodec_alloc_frame();

    return TRUE;
}

int ssc_ffmpeg_close(void *vp) {
    SSCHANDLE *handle = (SSCHANDLE *)vp;

    if(!handle)
        return TRUE;

    if(handle->pFrame)
        av_free(handle->pFrame);

    if(handle->pFmtCtx)
#if LIBAVFORMAT_VERSION_INT < AV_VERSION_INT(53, 21, 0)
        av_close_input_file(handle->pFmtCtx);
#else
        avformat_close_input(&handle->pFmtCtx);
#endif

    memset(handle,0,sizeof(SSCHANDLE));
    return TRUE;
}

#if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(53, 25, 0)
inline int copy_avsamples(AVCodecContext *ctx, AVFrame *frame, char *buffer, int len) {
    int dataSize = av_samples_get_buffer_size(NULL, ctx->channels,
                                             frame->nb_samples, ctx->sample_fmt, 1);
    if (len < dataSize)
       return -1;

    memcpy(buffer, frame->data[0], dataSize);

    return dataSize;
}
#endif


int _ssc_ffmpeg_read_frame(void *vp, char *buffer, int len) {
    SSCHANDLE *handle = (SSCHANDLE *)vp;
    int data_size;
    int len1;
#if LIBAVCODEC_VERSION_INT > AV_VERSION_INT(52, 25, 0)
    int got_frame;
    AVPacket pkt;
    av_init_packet(&pkt);
#endif

    while(1) {
        while(handle->packet_size > 0) {
#if LIBAVCODEC_VERSION_INT <= AV_VERSION_INT(52, 25, 0)
            data_size = len;
            len1=avcodec_decode_audio2(handle->pCodecCtx,
                                      (int16_t*)buffer,
                                      &data_size,
                                      handle->packet_data,
                                      handle->packet_size);
#elif LIBAVCODEC_VERSION_INT < AV_VERSION_INT(53, 25, 0)
            data_size = len;
            pkt.data = handle->packet_data;
            pkt.size = handle->packet_size;
            len1=avcodec_decode_audio3(handle->pCodecCtx,
                                      (int16_t*)buffer,
                                      &data_size,
                                      &pkt);
#else
            data_size = 0;
            got_frame = 0;
            pkt.data = handle->packet_data;
            pkt.size = handle->packet_size;
            len1=avcodec_decode_audio4(handle->pCodecCtx, handle->pFrame,
                                        &got_frame, &pkt);

            if (len1 >= 0 && got_frame) {
                data_size = copy_avsamples(handle->pCodecCtx, handle->pFrame, buffer, len);
                if (data_size < 0)
                    len1 = -1;
            } else if (len1 == 0)
                len1 = -1;
#endif
            if(len1 < 0) {
                /* skip frame */
                handle->packet_size=0;
                break;
            }

            handle->packet_data += len1;
            handle->packet_size -= len1;

            if(data_size <= 0)
                continue;

            handle->total_decoded += data_size;
            return data_size;
        }

        do {
            if(handle->packet.data)
                av_free_packet(&handle->packet);

#if LIBAVCODEC_VERSION_INT <= AV_VERSION_INT(52, 25, 0)
            if(av_read_packet(handle->pFmtCtx, &handle->packet) < 0)
#else
            if(av_read_frame(handle->pFmtCtx, &handle->packet) < 0)
#endif
                return -1;
        } while(handle->packet.stream_index != handle->audio_stream);

        handle->packet_size = handle->packet.size;
        handle->packet_data = handle->packet.data;
    }
}

void _ssc_ffmpeg_swab(char *buffer, int bytes_returned) {
    int blocks = bytes_returned / 2;
    int index;
    char tmp;

    for(index = 0; index < blocks; index++) {
        tmp = buffer[index*2];
        buffer[index*2] = buffer[index*2 + 1];
        buffer[index*2 + 1] = tmp;
    }
}

void _ssc_ffmpeg_le32(char *dst, int value) {
    dst[0] = value & 0xFF;
    dst[1] = (value >> 8) & 0xFF;
    dst[2] = (value >> 16) & 0xFF;
    dst[3] = (value >> 24) & 0xFF;
}

void _ssc_ffmpeg_le16(char *dst, int value) {
    dst[0] = value & 0xFF;
    dst[1] = (value >> 8) & 0xFF;
}

int ssc_ffmpeg_read(void *vp, char *buffer, int len) {
    SSCHANDLE *handle = (SSCHANDLE *)vp;
    int bytes_returned = 0;
    int bytes_to_copy;
    int size;

    int channels;
    int sample_rate;
    int bits_per_sample;
    int byte_rate;
    int duration = 180000; /* in ms -- 3 min */
    int data_len;
    int block_align;
    uint16_t test1 = 0xaabb;
    char test2[2] = { 0xaa, 0xbb };

    /* if we have not yet sent the header, let's do that first */
    if(handle->wav_offset != sizeof(handle->wav_header)) {
        /* still have some to send */
        if(!handle->wav_offset) {
            /* generate the wav header */
            channels = handle->pCodecCtx->channels;
            sample_rate = handle->pCodecCtx->sample_rate;
            switch(handle->pCodecCtx->sample_fmt) {
            case AV_SAMPLE_FMT_S16:
                bits_per_sample = 16;
                break;
            case AV_SAMPLE_FMT_S32:
                /* BROKEN */
                bits_per_sample = 32;
                break;
            default:
                bits_per_sample = 16;
                break;
            }

            handle->swab = (bits_per_sample == 16) && 
                (memcmp((void*)&test1,test2,2) == 0);

            if(handle->duration)
                duration = handle->duration;

            if(handle->samples) {
                data_len = ((bits_per_sample * channels / 8) * handle->samples);
            } else {
                data_len = ((bits_per_sample * sample_rate * channels / 8) * (duration/1000));
            }

            byte_rate = sample_rate * channels * bits_per_sample / 8;
            block_align = channels * bits_per_sample / 8;

            pi_log(E_DBG,"Channels.......: %d\n",channels);
            pi_log(E_DBG,"Sample rate....: %d\n",sample_rate);
            pi_log(E_DBG,"Bits/Sample....: %d\n",bits_per_sample);
            pi_log(E_DBG,"Swab...........: %d\n",handle->swab);

            memcpy(&handle->wav_header[0],"RIFF",4);
            _ssc_ffmpeg_le32(&handle->wav_header[4],36 + data_len);
            memcpy(&handle->wav_header[8],"WAVE",4);
            memcpy(&handle->wav_header[12],"fmt ",4);
            _ssc_ffmpeg_le32(&handle->wav_header[16],16);
            _ssc_ffmpeg_le16(&handle->wav_header[20],1);
            _ssc_ffmpeg_le16(&handle->wav_header[22],channels);
            _ssc_ffmpeg_le32(&handle->wav_header[24],sample_rate);
            _ssc_ffmpeg_le32(&handle->wav_header[28],byte_rate);
            _ssc_ffmpeg_le16(&handle->wav_header[32],block_align);
            _ssc_ffmpeg_le16(&handle->wav_header[34],bits_per_sample);
            memcpy(&handle->wav_header[36],"data",4);
            _ssc_ffmpeg_le32(&handle->wav_header[40],data_len);
        }
        
        bytes_to_copy = sizeof(handle->wav_header) - handle->wav_offset;
        if(len < bytes_to_copy)
            bytes_to_copy = len;

        memcpy(buffer,&handle->wav_header[handle->wav_offset],bytes_to_copy);
        handle->wav_offset += bytes_to_copy;
        return bytes_to_copy;
    }            
    
    /* could test for good len here */

    /* otherwise, start pumping out data */
    if(handle->buf_remainder_len) {
        /* dump remainder into the buffer */
        bytes_to_copy = handle->buf_remainder_len;
        if(handle->buf_remainder_len > len) {
            bytes_to_copy = len;
        }

        memcpy(buffer,handle->buf_remainder,bytes_to_copy);
        bytes_returned = bytes_to_copy;
        handle->buf_remainder_len -= bytes_to_copy;
        if(handle->buf_remainder_len) {
            handle->buf_remainder += bytes_returned;
        }
    }

    /* keep reading until we have filled the output buffer */
    while(bytes_returned < len) {
        size = _ssc_ffmpeg_read_frame(handle,handle->buffer,BUFFER_SIZE);
        if(size == 0) {
            /* oops, we're done */
            if(handle->swab)
                _ssc_ffmpeg_swab(buffer,bytes_returned);
            return bytes_returned;
        }

        if(size < 0) {
            return 0;
        }

        bytes_to_copy = len - bytes_returned;
        if(size < bytes_to_copy) 
            bytes_to_copy = size;

        memcpy(buffer + bytes_returned, handle->buffer, bytes_to_copy);
        bytes_returned += bytes_to_copy;

        if(size > bytes_to_copy) {
            handle->buf_remainder = handle->buffer + bytes_to_copy;
            handle->buf_remainder_len = size - bytes_to_copy;
        }

    }

    if(handle->swab)
        _ssc_ffmpeg_swab(buffer,bytes_returned);

    return bytes_returned;
}

