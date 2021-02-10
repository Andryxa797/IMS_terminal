/*
 * DataLinkLayer.cpp
 *
 *  Created on: 31 окт. 2014 г.
 *      Author: Dmitry F. Semenyuk
 */
#include "DataLinKLayer.h"


void T_DataLinkLayer_base::SendByteWithStaffing(uint8_t val)
{
    if((val==DLL_STAF_TOKEN)||(val==DLL_START_OR_END_FRAME))
    {
        val-=0x20;
        SendByte_p(DLL_STAF_TOKEN);
    }
    SendByte_p(val);
}



void T_DataLinkLayer_base::SendData_base(const uint8_t* Data_p, uint32_t Len, const uint16_t TxDataLenLimit)
{
    while(Len)
    {
        uint16_t crc = INIT_CRC16_Q921;

        //Передача заголовка фрейма
        SendByte_p(DLL_START_OR_END_FRAME);				//Признак начала кадра

        //Определение количества байт данных в передаваемом фрейме, если данных больше
        // лимита TxDataLenLimit, то данные будут разбиты на несколько фреймов
        uint32_t cnt;
        if(Len>TxDataLenLimit) cnt = TxDataLenLimit;
        else cnt = Len;
        Len -= cnt;

        //Передача данных
        while(cnt--)
        {
            uint8_t byte = *Data_p++;
            SendByteWithStaffing(byte);
            crc = CRC16_Q921_update(crc, byte);
        }
        crc ^= 0xffff;
        //Передача контрольной суммы
        SendByteWithStaffing(crc);		//Младший байт
        SendByteWithStaffing(crc>>8);	//Старший байт
    }
    SendByte_p(DLL_START_OR_END_FRAME);
}

