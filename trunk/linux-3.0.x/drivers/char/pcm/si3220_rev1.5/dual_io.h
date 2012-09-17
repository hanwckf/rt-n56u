//Labview typedefs
#include "registers.h"
#include "sram.h"
#include <asm/types.h>

typedef char				int8;
typedef unsigned char		uInt8;
typedef uInt8				uChar;
typedef short int			int16;
typedef unsigned short int	uInt16;
typedef long				int32;
typedef unsigned long		uInt32;
typedef float				float32;
typedef double				float64;
typedef int32				Bool32;   
//#define __GNUC__
//#define __s64 _int64

//Function prototypes
void sleep( unsigned long wait );  // milliseconds but with 16 ms grandularity.
void writeReg (int reg,unsigned char data);
__s64 calibratePrecisionClock(void); 
//write direct register

  void writeRam  (int reg,unsigned short data);
//write indirect register

  unsigned char readReg (int reg);
//read direct register

  unsigned short readRam (unsigned char reg);
//read indirect register

  void reset (void);
//press reset

  void unReset(void);
//release reset

  void broadcastReg (int reg,unsigned char data);

  void broadcastRAM (int reg,unsigned short data);

  void changeCID (unsigned char newCID);

  char*findRegName(unsigned short num);

  unsigned short findRegControlRefNum (char*reg);

  unsigned short findRAMNum (char*reg);

  unsigned short findRegNum (char*reg);

  //unsigned _int64
  __u64 delay( unsigned long wait);

  void reInit (unsigned short lpt);

  unsigned short findRAMControlNum (char*reg);

  void changeBroadcast (unsigned char newbroad);

void byteToSpi (unsigned char byte );
unsigned char  spiToByte(void);
unsigned char high8bit (unsigned short data);
unsigned char low8bit (unsigned short data);
unsigned short SixteenBit (unsigned char hi, unsigned char lo);

typedef struct  {
	char *name;
	int num;
	unsigned char Default;
}regData;


//RAM string array 
typedef struct  {
	char *name;
	unsigned short int screenNum; //for labview
	unsigned short int address; //actaul address
	unsigned short Default;
}regData16;

#define NumOfRAM 166
#define NumofREG 72
#define NumOnScreen 50

__s64 calibratePrecisionClock(void);


/* STRUCTURES */

// enum state_type { BUSY, CALL_BACK, CALLING_BACK };






