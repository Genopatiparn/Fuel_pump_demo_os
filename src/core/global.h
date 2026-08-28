#ifndef __GLOBAL_H
#define __GLOBAL_H

#include "main.h"

//structure for save data to flash 
typedef struct
{
    uint32_t index;
    uint8_t tabletune[72];   //fuel map 12x6
    uint16_t setup[40];
} mem_save_t;

// for 2d interpolate calculation 
typedef struct
{
    uint16_t *datatune;
    uint16_t *data_row;
    uint16_t *data_column;
    int32_t real_row;
    int32_t real_column;
    uint16_t max_row;
    uint16_t max_column;

    int32_t x;
    int32_t y;
    int32_t ix1;
    int32_t ix2;
    int32_t iy1;
    int32_t iy2;
    int32_t v11;
    int32_t v12;
    int32_t v21;
    int32_t v22;
    int32_t y1i;
    int32_t y2i;
    int32_t x1i;
    int32_t x2i;
    int32_t X1;
    int32_t Y1;
    float subX;
    float subY;
} mdatainterpolate_t;

typedef struct
{
    uint16_t controlDelay;
    uint32_t systick;
    uint32_t lastTickRpm;
    uint32_t lastTimeCap;
    mem_save_t mem_save;
    
    unsigned short *xrpm;
    unsigned short *yduty;
    short rpm;

    uint16_t timeInjIn;
    uint16_t counterInj;
    uint16_t counterRpm;
    uint16_t timeCapRpm;
    uint16_t timeLastCapRpm;
    uint16_t countDuty;
    uint16_t duty;
    uint16_t tempCalDuty;

    short V2;
    uint16_t *mode;
    uint16_t *enginetype;
    uint16_t *cylinder;
    uint16_t *brand;
    uint16_t *password;
    char *Mac0;
    char *Mac1;
    char *Mac2;
    char *Mac3;

    char *Band0;
    char *Band1;
    char *Band2;
    char *Band3;

    unsigned char *temptype;
    uint8_t *o2type;

    short countOvertimeInj;
    unsigned short capTimeInjOn;
    unsigned short capRpm;
    FlagStatus newRpm;
    FlagStatus _Ftestoutput;
    FlagStatus statIdCpu;
    short delayWriteEeprom;
    short countOverRpm;
    short countDownRpm;
    short waitConnection;
    short delay100ms;
    short buffInjInSum;
    short countInjInSum;

    FlagStatus _Fcleardata;
    FlagStatus _Fadduser;

    FlagStatus mem_write_ee;
    FlagStatus mem_read_ee;
    FlagStatus mem_reset;
    FlagStatus mem_write_datatune;
    FlagStatus mem_read_datatune;
    FlagStatus mem_write_setup;
    FlagStatus mem_read_setup;

    uint16_t powerTune;
    uint16_t powerCal;
    uint16_t currentControl;
    FlagStatus ecuLock;
    uint16_t realCurrent;

    osEventFlagsId_t eventFlagsSerial;
} pump_t;

// bluetooth struct 
typedef struct
{
    unsigned char _tempBTN[100];
    
    // DMA Buffer management
    unsigned char txBuffer[5][100]; 
    unsigned char bufferSizes[5];
    unsigned char bufferReadIndex;
    unsigned char bufferWriteIndex;
    unsigned char buffersUsed;
    FlagStatus dmaBusy;

    unsigned char sumtx;
    short numsend;
    short delaysend;
    short delayonATCMD;
    FlagStatus _FATmode;
    FlagStatus _FATrespond;
    FlagStatus _FATreset;
    FlagStatus _Fsendiscon;
    FlagStatus _Fchangename;
    FlagStatus _FsendAT;

    FlagStatus _FsenYES;
    FlagStatus _FsenNO;
    FlagStatus _FuserFull;
    FlagStatus mobileconnect;
    FlagStatus _Fsenabout;
    FlagStatus _FsenDONE;
    FlagStatus _Fsetpower;
    FlagStatus _Fsetgain;
    FlagStatus _Finitbth;
    FlagStatus _Fsendtext;
    FlagStatus _Fadduser;
    FlagStatus _Freset;
    FlagStatus _Fgetname;
    FlagStatus _Fsetconni;
    FlagStatus _FsenTset;
    FlagStatus _FsenTune;

    FlagStatus _Fgetmac;
    FlagStatus testpass;
    FlagStatus _FNULL;
    uint32_t mobileActivity;

    char mbtext[90];
    short errorCountBTN;
    char *mobilename;
    char macaddress[20];
    short userin;
    short countdownAdduser;
    unsigned short delayoff;
    short cartype;
    short indexDataTune;
    unsigned char buffname[4];
} mBTN;


extern pump_t pump;
extern mBTN BT5;

// driver function prototypes
void init_gpio(void);
void init_adc(void);
void init_PwmFet(void);
void init_LED(void);
void init_injector(void);
void init_uart_bluetooth(void);
void init_uart0(void);

// function prototypes
void module_init(void);
void init_0(void);
void mem_write(mem_save_t *source);
void mem_read(mem_save_t *dest);
FlagStatus FLASH_Erase_Range_0x08030000_to_0x08040000(void);
FlagStatus FLASH_Erase_Page(uint32_t address);
uint32_t find_last_index(void);

void save_token(void);
FlagStatus read_token(void);
FlagStatus checkIdCpu(void);
void save_cpuid(void);
void FLASH_Program_Word(unsigned long address, unsigned long dest);

// DMA Buffer 
FlagStatus addToTxBuffer(char *data, unsigned char size);
void serviceDmaInterrupt(void);
void initTxBuffers(void);

// pump control
void updatePwmAdc(uint16_t duty);
void updatePwmLed(uint16_t duty);
void updateCurrentMeasurement(void);
FlagStatus checkFuelPumpOn(void);
void controlPump(uint16_t power);

// interpolation
int32_t interpolate(mdatainterpolate_t *D);
short calInterPolate2D(void);

// rpm calculator
void calGeneral(void);

// led effects
void updateLed(void);

// protocol functions
void senddatamonitorBTN(void);
void sendsetupBTN(void);
void sendTuneBTN(void);
void getdatatunemobile(char *_data);
void getdatasetupmobile(char *_data, pump_t *pump);
FlagStatus check_nmac(char *_data);
FlagStatus check_nbrand(char *_data);
FlagStatus checksumhm10(unsigned char _indexsum, char *_data);

// tasks
void vMainTask(void *pvParameters);
void task_bluetooth(void *pvParameter);
void service_serialPort(void *pvParameters);

#endif
