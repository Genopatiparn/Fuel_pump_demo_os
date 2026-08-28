#include "main.h"

// Helper: convert hex char to value
short H_data(char _data)
{
    unsigned char buff = 0;
    buff = _data - 0x30;
    if (buff > 0x09)
        buff -= 0x07;
    return buff;
}

// Helper: parse hex data from string
unsigned short p_data(char _data, char num, char *address)
{
    unsigned short buff = 0;
    unsigned char i = 0, buff2;

    if (num > 1)
    {
        for (; num > i; i++)
        {
            buff2 = *(address + _data) - 0x30;
            if (buff2 > 0x09)
                buff2 -= 0x07;
            buff <<= 4;
            buff |= buff2;
            _data++;
        }
    }
    else
    {
        buff = *(address + _data) - 0x30;
        if (buff > 0x09)
            buff -= 0x07;
    }
    return buff;
}

// Send data as hex without checksum
void s_dataBTNwithoutsum(unsigned long _data, short num)
{
    unsigned char buff;
    num -= 1;
    num *= 4;

    for (; num > 0;)
    {
        buff = (_data >> num) & 0x0F;
        buff += 0x30;
        if (buff > 0x39)
            buff += 0x07;
        BT5._tempBTN[BT5.numsend++] = buff;
        num -= 4;
    }
    buff = _data & 0x0F;
    buff += 0x30;
    if (buff > 0x39)
        buff += 0x07;
    BT5._tempBTN[BT5.numsend++] = buff;
}

// Send data as hex with checksum
void s_dataBTN(unsigned long _data, short num)
{
    unsigned char buff;
    num -= 1;
    num *= 4;

    for (; num > 0;)
    {
        buff = (_data >> num) & 0x0F;
        buff += 0x30;
        if (buff > 0x39)
            buff += 0x07;
        BT5._tempBTN[BT5.numsend++] = buff;
        BT5.sumtx += buff;
        num -= 4;
    }
    buff = _data & 0x0F;
    buff += 0x30;
    if (buff > 0x39)
        buff += 0x07;
    BT5._tempBTN[BT5.numsend++] = buff;
    BT5.sumtx += buff;
}

// Send string
void s_strBTN(char *_data)
{
    while (*_data != '\0')
    {
        BT5._tempBTN[BT5.numsend] = *_data;
        BT5.sumtx += *_data;
        BT5.numsend++;
        BT5._tempBTN[BT5.numsend] = 0;
        _data++;
    }
}

// Transmit without checksum
void txuartBTNNotsum(void)
{
    s_strBTN("\r\n");
    addToTxBuffer((char *)BT5._tempBTN, BT5.numsend);
    BT5.delaysend = 0;
    BT5.numsend = 0;
    BT5.sumtx = 0;
}

// Transmit with checksum
void txuartBTN(void)
{
    s_dataBTNwithoutsum(BT5.sumtx, 2); // add checksum
    s_strBTN("\r\n");
    addToTxBuffer((char *)BT5._tempBTN, BT5.numsend);
    BT5.delaysend = 0;
    BT5.numsend = 0;
    BT5.sumtx = 0;
}

// Transmit command
void txuartCMD_BTN(mBTN *btn)
{
    addToTxBuffer((char *)btn->_tempBTN, btn->numsend);
    btn->delaysend = 0;
    btn->numsend = 0;
    btn->sumtx = 0;
}
