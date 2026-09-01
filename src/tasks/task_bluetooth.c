#include "main.h"

// external functions
extern void senddatamonitorBTN(void);
extern void sendsetupBTN(void);
extern void sendTuneBTN(void);
extern void sendaboutBTN(void);
extern void mbsendtext(char *_data);
extern void s_strBTN(char *_data);
extern void txuartCMD_BTN(BlueTooth_module *btn);

void task_bluetooth(void *pvParameter)
{
    (void)pvParameter;
    static uint16_t monitorDelay = 0;
    
    BT.mobileconnect = RESET;
    BT._FATmode = RESET;     // Disable AT mode
    initTxBuffers();
    
    while (1)
    {
        osDelay(1);

        if (BT.mobileconnect == SET && BT.mobileActivity < 20000)
        {
            BT.mobileActivity++;
            if (BT.mobileActivity == 20000)
            {
                mbsendtext("#TIMEOUT\r\n");
                BT.mobileconnect = RESET;
            }
        }

        if (BT._Fadduser == SET)
        {
            *pump.Mac0 = BT.buffname[0];
            *pump.Mac1 = BT.buffname[1];
            *pump.Mac2 = BT.buffname[2];
            *pump.Mac3 = BT.buffname[3];
            mem_write(&pump.mem_save);
            BT._Fadduser = RESET;
            BT.mobileconnect = SET;
            BT.mobileActivity = 0;
            BT._FsenYES = SET;
            BT._Fsenabout = SET;
            BT._FsenTset = SET;
            BT._FsenTune = SET;
        }

        if (BT.delaysend < 100)
        {
            BT.delaysend++;
        }
        else if (BT._FsenYES == SET)
        {
            mbsendtext("#YES\n");
            BT._FsenYES = RESET;
        }
        else if (BT._FsenNO == SET)
        {
            mbsendtext("#NO\r\n");
            BT._FsenNO = RESET;
        }
        else if (BT._FsenDONE == SET)
        {
            mbsendtext("#DONE\r\n");
            BT._FsenDONE = RESET;
        }
        else if (BT._Fsenabout == SET)
        {
            sendaboutBTN();
        }
        else if (BT._FsenTset == SET)
        {
            sendsetupBTN();
        }
        else if (BT._FsenTune == SET)
        {
            sendTuneBTN();
        }
        else if (BT.mobileconnect == SET)
        {
            if (monitorDelay++ > 100)
            {
                monitorDelay = 0;
                senddatamonitorBTN();
            }
        }
        else
        {
            monitorDelay = 0;
        }
    }
}
