#include "main.h"

// external functions
extern void senddatamonitorBTN(void);
extern void sendsetupBTN(void);
extern void sendTuneBTN(void);
extern void sendaboutBTN(void);
extern void mbsendtext(char *_data);
extern void s_strBTN(char *_data);
extern void txuartCMD_BTN(mBTN *btn);

void task_bluetooth(void *pvParameter)
{
    (void)pvParameter;
    static uint16_t monitorDelay = 0;
    
    BT5.mobileconnect = RESET;
    BT5._FATmode = RESET;     // Disable AT mode
    initTxBuffers();
    
    while (1)
    {
        osDelay(1);

        if (BT5.mobileconnect == SET && BT5.mobileActivity < 20000)
        {
            BT5.mobileActivity++;
            if (BT5.mobileActivity == 20000)
            {
                mbsendtext("#TIMEOUT\r\n");
                BT5.mobileconnect = RESET;
            }
        }

        if (BT5.delaysend < 100)
        {
            BT5.delaysend++;
        }
        else if (BT5._FsenYES == SET)
        {
            mbsendtext("#YES\n");
            BT5._FsenYES = RESET;
        }
        else if (BT5._FsenNO == SET)
        {
            mbsendtext("#NO\r\n");
            BT5._FsenNO = RESET;
        }
        else if (BT5._FsenDONE == SET)
        {
            mbsendtext("#DONE\r\n");
            BT5._FsenDONE = RESET;
        }
        else if (BT5._Fsenabout == SET)
        {
            sendaboutBTN();
        }
        else if (BT5._FsenTset == SET)
        {
            sendsetupBTN();
        }
        else if (BT5._FsenTune == SET)
        {
            sendTuneBTN();
        }
        else if (BT5.mobileconnect == SET)
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
