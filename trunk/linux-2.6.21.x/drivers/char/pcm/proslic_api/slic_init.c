#include "proslic.h"
#include "si_voice_datatypes.h"
#include "si_voice_ctrl.h"
#include "si_voice.h"
#include "spi.h"
#include "timer.h"
#include "../pcm_ctrl.h"

extern pcm_config_type* ppcm_config;
/*
**
** Multi-port ProSLIC Initialization
**
*/

#define NUMBER_OF_DEVICES (CONFIG_PCM_CH>>1)
#define PROSLIC_DEVICE_TYPE SI3226_TYPE

#include "slic_init.h"

int ProSLIC_HWInit(void)
{
	int32 i, result = 0;
	ctrl_S spiGciObj; /* User¡¦s control interface object */
	systemTimer_S timerObj; /* User¡¦s timer object */
	chanState ports[NUMBER_OF_CHAN]; /* User¡¦s channel object, which has
									** a member defined as
									** proslicChanType_ptr ProObj;
									*/
	/* Define ProSLIC control interface object */
	controlInterfaceType *ProHWIntf;
	/* Define array of ProSLIC device objects */
	ProslicDeviceType *ProSLICDevices[NUMBER_OF_PROSLIC];
	/* Define array of ProSLIC channel object pointers */
	proslicChanType_ptr arrayOfProslicChans[NUMBER_OF_CHAN];
	/*
	** Step 1: (optional)
	** Initialize user¡¦s control interface and timer objects ¡V this
	** may already be done, if not, do it here
	*/
	/*
	** Step 2: (required)
	** Create ProSLIC Control Interface Object
	*/
	ProSLIC_createControlInterface(&ProHWIntf);
	/*
	** Step 3: (required)
	** Create ProSLIC Device Objects
	*/
	for(i=0;i<NUMBER_OF_PROSLIC;i++)
	{
		ProSLIC_createDevice(&(ProSLICDevices[i]));
	}
	/*
	** Step 4: (required)
	** Create and initialize ProSLIC channel objects
	** Also initialize array pointers to user¡¦s proslic channel object
	** members to simplify initialization process.
	*/
	for(i=0;i<NUMBER_OF_CHAN;i++)
	{
		ProSLIC_createChannel(&(ports[i].ProObj));
		ProSLIC_SWInitChan(ports[i].ProObj,i,PROSLIC_DEVICE_TYPE,
		ProSLICDevices[i/CHAN_PER_DEVICE],ProHWIntf);
		arrayOfProslicChans[i] = ports[i].ProObj;
		ProSLIC_setSWDebugMode(ports[i].ProObj,TRUE); /* optional */
	}
	/*
	** Step 5: (required)
	** Establish linkage between host objects/functions and
	** ProSLIC API
	*/
	ProSLIC_setControlInterfaceCtrlObj (ProHWIntf, &spiGciObj);
	ProSLIC_setControlInterfaceReset (ProHWIntf, ctrl_ResetWrapper);
	ProSLIC_setControlInterfaceWriteRegister (ProHWIntf, ctrl_WriteRegisterWrapper);
	ProSLIC_setControlInterfaceReadRegister (ProHWIntf, ctrl_ReadRegisterWrapper);
	ProSLIC_setControlInterfaceWriteRAM (ProHWIntf, ctrl_WriteRAMWrapper);
	ProSLIC_setControlInterfaceReadRAM (ProHWIntf, ctrl_ReadRAMWrapper);
	ProSLIC_setControlInterfaceTimerObj (ProHWIntf, &timerObj);
	ProSLIC_setControlInterfaceDelay (ProHWIntf, time_DelayWrapper);
	ProSLIC_setControlInterfaceTimeElapsed (ProHWIntf, time_TimeElapsedWrapper);
	ProSLIC_setControlInterfaceGetTime (ProHWIntf, time_GetTimeWrapper);
	ProSLIC_setControlInterfaceSemaphore (ProHWIntf, NULL);
	/*
	** Step 6: (system dependent)
	** Assert hardware Reset ¡V ensure VDD, PCLK, and FSYNC are present and stable
	** before releasing reset
	*/
	ProSLIC_Reset(ports[0].ProObj);
	
	//initialize SPI interface
    if (SPI_Init (NULL) ==  FALSE){

        printk ("Cannot connect\n");

        return 0;
    }

	/*
	** Step 7: (required)
	** Initialize device (loading of general parameters, calibrations,
	** dc-dc powerup, etc.)
	*/
	ProSLIC_Init(arrayOfProslicChans,NUMBER_OF_CHAN);
	printk("ProSLIC_Init done\n");
	for(i=0;i<NUMBER_OF_CHAN;i++)
	{
		if(arrayOfProslicChans[i]->error!=0)
		{
			printk("ProSLIC_Init[%d] ERR=%d\n",i,arrayOfProslicChans[i]->error);
			return 0;
		}
	}
	
	/*
	** Step 8: (design dependent)
	** Execute longitudinal balance calibration
	** or reload coefficients from factory LB cal
	**
	** Note: all batteries should be up and stable prior to
	** executing the lb cal
	*/
	ProSLIC_LBCal(arrayOfProslicChans,NUMBER_OF_CHAN);
	for(i=0;i<NUMBER_OF_CHAN;i++)
	{
		ProSLIC_GetLBCalResultPacked(arrayOfProslicChans[i], &result);
		printk("LBCal=0x%08X\n",result);
	}
	/*
	** Step 9: (design dependent)
	** Load custom configuration presets(generated using
	** ProSLIC API Config Tool)


	*/
	for(i=0;i<NUMBER_OF_CHAN;i++)
	{
		ProSLIC_PCMTimeSlotSetup(ports[i].ProObj,ppcm_config->ts_start[i],ppcm_config->ts_start[i]);
		
		ProSLIC_DCFeedSetup(ports[i].ProObj,DCFEED_48V_20MA);
		ProSLIC_RingSetup(ports[i].ProObj,RING_F20_45VRMS_0VDC_LPR);
#if defined(PCM_LINEAR)||defined(PCM_A2L2A)||defined(PCM_U2L2U)			
		ProSLIC_PCMSetup(ports[i].ProObj,PCM_DEFAULT_CONFIG);
#endif
#if defined(PCM_ULAW)||defined(PCM_L2U2L)		
		ProSLIC_PCMSetup(ports[i].ProObj,PCM_ULAW_CONFIG);
#endif
#if defined(PCM_ALAW)			
		ProSLIC_PCMSetup(ports[i].ProObj,PCM_ALAW_CONFIG);
#endif
#if defined(PCM_L2A2L)			
		ProSLIC_PCMSetup(ports[i].ProObj,PCM_ALAW_INV_CONFIG);
#endif
		ProSLIC_ZsynthSetup(ports[i].ProObj,ZSYN_600_0_0);
		ProSLIC_ToneGenSetup(ports[i].ProObj,TONEGEN_FCC_DIAL);
		
	}
	for(i=0;i<NUMBER_OF_CHAN;i++)
	{
		ProSLIC_PCMStart(ports[i].ProObj);
	}	
	for(i=0;i<NUMBER_OF_CHAN;i++)
	{
		ProSLIC_SetLinefeedStatus(ports[i].ProObj,LF_FWD_ACTIVE);
	}	
	return 1;
}	
/*
** END OF TYPICAL INITIALIZATION
*/

