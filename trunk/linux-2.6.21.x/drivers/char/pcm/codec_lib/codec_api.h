#ifndef _CODEC_API_H_
#define _CODEC_API_H_
typedef struct codec_data_t
{
	int type;
	int nFrame;
	union
	{
		unsigned char* pPCMBuf8;
		unsigned short* pPCMBuf16;
	};
	char* pBitBuf;
	
	int PCMBufWordLen;
	int BitBufByteLen;
	int ch;
	
}codec_data_type;

int voice_init_codec(codec_data_type* pdata);
void voice_encode_frame(codec_data_type* pdata);
void voice_decode_frame(codec_data_type* pdata);

#define G711ULAW_CODEC				1
#define G711ALAW_CODEC				2
#define G729AB_CODEC				3
#define G723A_CODEC					4

#endif /* _CODEC_API_H_ */