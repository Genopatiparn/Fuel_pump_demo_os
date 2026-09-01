#include "main.h"

// external functions from bluetooth.c
extern short H_data(char _data);
extern unsigned short p_data(char _data, char num, char *address);
extern void s_dataBTN(unsigned long _data, short num);
extern void s_strBTN(char *_data);
extern void txuartBTN(void);
extern void txuartCMD_BTN(BlueTooth_module *btn);
extern void mbsendtext(char *_data);

// Check checksum
FlagStatus checksumhm10(unsigned char _indexsum, char *_data)
{
    short temp = 0, index;
    for (index = 0; index < _indexsum; index++)
        temp += *(_data + index);
    temp &= 0x00FF;
    index = p_data(_indexsum, 2, _data);

    if (temp == index)
        return SET;
    else
        return RESET;
}

// Check MAC address
FlagStatus check_nmac(char *_data)
{
    _data += 4; // skip "#INJ"

    if (*_data != *pump.Mac0)
        return SET;
    _data++;
    if (*_data != *pump.Mac1)
        return SET;
    _data++;
    if (*_data != *pump.Mac2)
        return SET;
    _data++;
    if (*_data != *pump.Mac3)
        return SET;

    return RESET; // MAC matched
}

// Check brand
FlagStatus check_nbrand(char *_data)
{
    _data += 8;

    if (*_data != *pump.Band0)
        return SET;
    _data++;
    if (*_data != *pump.Band1)
        return SET;
    _data++;
    if (*_data != *pump.Band2)
        return SET;
    _data++;
    if (*_data != *pump.Band3)
        return SET;

    return RESET;
}

// Send text
void mbsendtext(char *_data)
{
    addToTxBuffer(_data, strlen(_data));
}

void sendaboutBTN(void)
{
    static short aboutStep = 0;
    uint32_t value;

    switch (aboutStep)
    {
    case 0:
        s_strBTN("#MCUID");
        value = (GetUID_Word2 >> 16) & 0xFFFF;
        s_dataBTN(value, 4);
        value = GetUID_Word2 & 0xFFFF;
        s_dataBTN(value, 4);
        value = (GetUID_Word1 >> 16) & 0xFFFF;
        s_dataBTN(value, 4);
        value = GetUID_Word1 & 0xFFFF;
        s_dataBTN(value, 4);
        value = (GetUID_Word0 >> 16) & 0xFFFF;
        s_dataBTN(value, 4);
        value = GetUID_Word0 & 0xFFFF;
        s_dataBTN(value, 4);
        s_strBTN("\n");
        txuartCMD_BTN(&BT);
        break;
    case 1:
        s_strBTN("#HARDWARE");
        s_dataBTN(_hardware, 4);
        s_strBTN("\n");
        txuartCMD_BTN(&BT);
        break;
    case 2:
        s_strBTN("#SOFTWARE");
        s_dataBTN(_firmware, 4);
        s_strBTN("\n");
        txuartCMD_BTN(&BT);
        break;
    case 3:
        s_strBTN("#CARTYPE");
        s_dataBTN(BT.cartype, 4);
        s_strBTN("\n");
        txuartCMD_BTN(&BT);
        break;
    case 4:
        s_strBTN("#MODEL010\n");
        txuartCMD_BTN(&BT);
        break;
    case 5:
        s_strBTN("#BRAND");
        s_dataBTN(H_data(*pump.Band0), 1);
        s_dataBTN(H_data(*pump.Band1), 1);
        s_dataBTN(H_data(*pump.Band2), 1);
        s_dataBTN(H_data(*pump.Band3), 1);
        s_strBTN("\n");
        txuartCMD_BTN(&BT);
        break;
    default:
        aboutStep = 0;
        BT._Fsenabout = RESET;
        return;
    }

    aboutStep++;
}

// Get tune data from mobile
void getdatatunemobile(char *_data)
{
    short i, x;
    static short cntsave = 0;
    
    if (strstr(_data, "#TTUN") != NULL)
    {
        if (checksumhm10(54, _data) == SET)
        {
            x = p_data(5, 1, _data);
            for (i = 0; i < 12; i++)
            {
                uint16_t d = p_data(((i * 4) + 6), 4, _data);
                d *= 0.1f;
                pump.mem_save.tabletune[((x * 12) + i)] = d;
            }

            if (x == 0)
            {
                mbsendtext("#YES_TTUN0\r\n");
                cntsave = 0;
            }
            else if (x == 1)
            {
                cntsave++;
                mbsendtext("#YES_TTUN1\r\n");
            }
            else if (x == 2)
            {
                cntsave++;
                mbsendtext("#YES_TTUN2\r\n");
            }
            else if (x == 3)
            {
                cntsave++;
                mbsendtext("#YES_TTUN3\r\n");
            }
            else if (x == 4)
            {
                cntsave++;
                mbsendtext("#YES_TTUN4\r\n");
            }
            else if (x == 5)
            {
                cntsave++;
                mbsendtext("#YES_TTUN5\r\n");
                if (cntsave == 5)
                {
                    mem_write(&pump.mem_save);
                }
                else
                    BT._FsenNO = SET;
            }
            else
                BT._FsenNO = SET;
        }
        else
            BT._FsenNO = SET;
    }
    else
        BT._FsenNO = SET;
}

// Get setup data from mobile
void getdatasetupmobile(char *_data, pump_t *pump)
{
    short i;

    if (strstr(_data, "#SET0"))
    {
        if (checksumhm10(53, _data) == SET)
        {
            for (i = 0; i < 12; i++)
                pump->xrpm[i] = p_data(((i * 4) + 5), 4, _data);
            mem_write(&pump->mem_save);
            mbsendtext("#YES_SET0\r\n");
        }
    }
    else if (strstr(_data, "#SET1"))
    {
        if (checksumhm10(29, _data) == SET)
        {
            for (i = 0; i < 6; i++)
                pump->yduty[i] = p_data(((i * 4) + 5), 4, _data);
            mem_write(&pump->mem_save);
            mbsendtext("#YES_SET1\r\n");
        }
    }
    else if (strstr(_data, "#SET2"))
    {
        if (checksumhm10(15, _data) == SET)
        {
            *pump->enginetype = p_data(5, 4, _data);
            *pump->cylinder = p_data(9, 4, _data);
            *pump->temptype = p_data(13, 1, _data);
            *pump->o2type = p_data(14, 1, _data);
            mem_write(&pump->mem_save);
            mbsendtext("#YES_SET2\r\n");
        }
    }
}

// Send setup to BTN
void sendsetupBTN(void)
{
    static short lop = 0;
    short i;
    lop++;
    
    switch (lop)
    {
    case 1:
        if (pump.mem_read_setup == SET)
            lop--;
        break;
    case 2:
        s_strBTN("#SET0");
        for (i = 0; i < 12; i++)
            s_dataBTN(*(pump.xrpm + i), 4);
        txuartBTN();
        break;
    case 3:
        s_strBTN("#SET1");
        for (i = 0; i < 6; i++)
            s_dataBTN(*(pump.yduty + i), 4);
        txuartBTN();
        break;
    case 4:
        s_strBTN("#SET2");
        s_dataBTN(*pump.enginetype, 4);
        s_dataBTN(*pump.cylinder, 4);
        s_dataBTN(*pump.temptype, 1);
        s_dataBTN(*pump.o2type, 1);
        txuartBTN();
        break;
    case 5:
        lop = 0;
        BT._FsenTset = RESET;
        break;
    }
}

// Send tune data to BTN
void sendTuneBTN(void)
{
    static short i = 0;
    short i2;

    s_strBTN("#TTUN");
    s_dataBTN(i, 1);
    for (i2 = 0; i2 < 12; i2++)
        s_dataBTN(pump.mem_save.tabletune[((i * 12) + i2)] * 10, 4);
    txuartBTN();
    
    i++;
    if (i > 5)
    {
        BT._FsenTune = RESET;
        i = 0;
    }
}

// Send monitor data
void senddatamonitorBTN(void)
{
    s_strBTN("#M");
    s_dataBTN(pump.rpm, 4);
    s_dataBTN(pump.timeInjIn, 4);
    s_dataBTN(pump.realCurrent, 4);
    s_dataBTN(pump.powerTune, 4);
    s_dataBTN(0000, 4); // AFR placeholder
    s_dataBTN(pump.duty, 2);
    s_dataBTN(1, 1); // mode
    s_dataBTN(pump.V2, 4);
    txuartBTN();
}

// UART2 Interrupt Handler
void USART2_IRQHandler(void)
{
    static char rxbuf[90];
    static short index = 0;
    uint8_t temp = 0;
    short i;

    if (usart_interrupt_flag_get(USART2, USART_INT_FLAG_RBNE) == SET)
    {
        temp = usart_data_receive(USART2);
        rxbuf[index++] = temp;
        index %= 90;
        rxbuf[index] = '\0';
    }

    if (temp != '\n')
        return;

    temp = 0;
    index = 0;

    // Parse commands
    if (strstr(rxbuf, "CC+NAME") != NULL)
    {
        for (i = 0; i < 12; i++)
        {
            temp = rxbuf[(i + 7)];
            if ((temp != '\n') && (temp != '\r'))
            {
                BT.buffname[i] = temp;
            }
            else
                break;
        }
        BT._Fgetname = SET;
    }
    else if (strstr(rxbuf, "CC+MAC") != NULL)
    {
        for (i = 0; i < 17; i++)
        {
            temp = rxbuf[(i + 6)];
            if ((temp != '\n') && (temp != '\r'))
            {
                BT.macaddress[i] = temp;
            }
            else
                break;
        }
        BT._Fgetmac = SET;
    }
    else if (strstr(rxbuf, "#ABOUT") != NULL)
    {
        BT._Fsenabout = SET;
    }
    else if (strstr(rxbuf, "#GETSET") != NULL || strstr(rxbuf, "#TSETLD") != NULL)
    {
        BT._FsenTset = SET;
    }
    else if (strstr(rxbuf, "#GETTUN") != NULL || strstr(rxbuf, "#TTUNLD") != NULL)
    {
        BT._FsenTune = SET;
    }
    else if (strstr(rxbuf, "#CALLINJ") != NULL)
    {
        BT.mobileActivity = 0;
    }
    else if (strstr(rxbuf, "#INJ") != NULL)
    {
        if (check_nmac(rxbuf) == RESET)
        {
            BT.mobileconnect = SET;
            BT.mobileActivity = 0;
            BT._FsenYES = SET;
            BT._Fsenabout = SET;
            BT._FsenTset = SET;
            BT._FsenTune = SET;
        }
        else
        {
            if (pump.rpm == 0)
            {
                BT.buffname[0] = rxbuf[4];
                BT.buffname[1] = rxbuf[5];
                BT.buffname[2] = rxbuf[6];
                BT.buffname[3] = rxbuf[7];
                BT._Fadduser = SET;
            }
            else
            {
                BT._FsenNO = SET;
            }
        }
    }
    else if (strstr(rxbuf, "#SETB") != NULL)
    {
        if (check_nmac(rxbuf) == RESET)
        {
            *pump.Band0 = rxbuf[8];
            *pump.Band1 = rxbuf[9];
            *pump.Band2 = rxbuf[10];
            *pump.Band3 = rxbuf[11];
            mem_write(&pump.mem_save);
            BT._FsenYES = SET;
        }
        else
        {
            BT._FsenNO = SET;
        }
    }
    else if (strstr(rxbuf, "#CLEAR") != NULL)
    {
        pump._Fcleardata = SET;
    }
    else if (strstr(rxbuf, "#SET") != NULL)
    {
        getdatasetupmobile(rxbuf, &pump);
    }
    else if (strstr(rxbuf, "#TTUN") != NULL)
    {
        getdatatunemobile(rxbuf);
    }

    memset(rxbuf, 0, 90);
}
