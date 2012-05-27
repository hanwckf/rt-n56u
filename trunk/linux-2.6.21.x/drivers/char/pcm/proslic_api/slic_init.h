#include "../pcm_ctrl.h"

#ifdef SI322X
#include "si3226_constants.h"
        #define CHAN_PER_DEVICE 2
        #define NUMBER_OF_CHAN (NUMBER_OF_DEVICES*CHAN_PER_DEVICE)
        #define NUMBER_OF_PROSLIC (NUMBER_OF_CHAN)
#endif

typedef struct chanStatus chanState; //forward declaration

typedef void (*procState) (chanState *pState, ProslicInt eInput);


/*
** structure to hold state information for pbx demo
*/
struct chanStatus {
        proslicChanType *ProObj;
        SiVoiceChanType_ptr VoiceObj;
        timeStamp onHookTime;
        timeStamp offHookTime;
        procState currentState;
        uInt16 digitCount;
        uInt8 digits[20];
        uInt8 ringCount;
        uInt16 connectionWith;
        uInt16 powerAlarmCount;
        pulseDialType pulseDialData;
        BOOLEAN eventEnable;
} ;
