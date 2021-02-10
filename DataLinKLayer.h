/*
 * DataLinkLayer.h
 *
 *  Created on: 31 окт. 2014 г.
 *      Author: Dmitry F. Semenyuk
 */

#ifndef DATALINKLAYER_H_
#define DATALINKLAYER_H_

#include <stdint.h>
#include "CRC.h"

//#include "common.h"
//#include "CLI.h"

//====================================================================================
//	КОД ОПТИМИЗИРОВАН ПОД СЛЕДУЮЩИЕ ЗНАЧЕНИЯ КОНСТАНТ:
//
//	0x7E	- флаг начала/конца кадра
//	0x7D	- признак байтстаффинга
//	Байтстаффинг:
//	байт 0x7Е внутри кадра заменяется последовательностью из двух байт: 0x7D, 0x5E
//	байт 0x7D внутри кадра заменяется последовательностью из двух байт: 0x7D, 0x5D

#define DLL_START_OR_END_FRAME		((uint8_t)0x7E)	// НЕ МЕНЯТЬ!!!
#define DLL_STAF_TOKEN				((uint8_t)0x7D)	// НЕ МЕНЯТЬ!!!

//====================================================================================
//
class T_DataLinkLayer_base
{
protected:
    //переменные состояния приема пакета
    uint16_t FrameLen;
    uint16_t crc;
    bool ReceivingData;		//разрешение приема данных
    bool ByteDeStaffing;

    //Указатель на функцию отправки байта через физический интерфейс
    void (*SendByte_p)(uint8_t val);

    //Указатель на функцию обработки принятых данных
    void (*RxDataHandler_p)(uint8_t* Buf, uint16_t Len);

    T_DataLinkLayer_base(void(*SendByteFunc_p)(uint8_t), void(*RxDataHandlerFunc_p)(uint8_t*, uint16_t))
    : FrameLen(0), crc(0), ReceivingData(false), ByteDeStaffing(false), SendByte_p(SendByteFunc_p), RxDataHandler_p(RxDataHandlerFunc_p)
    {}

    inline void SendByteWithStaffing(uint8_t val);

    //Отправляет отрезок данных в виде одного или нескольких фреймов, согласно протоколу
    void SendData_base(const uint8_t* Data_p, uint32_t Len, const uint16_t TxDataLenLimit);

public:

    void RxReset()
    {
        FrameLen = 0;
        crc = 0;
        ReceivingData = false;
        ByteDeStaffing = false;
    }

};

//====================================================================================
/*
 * Шаблон класса TT_DataLinkLayerPoint обеспечивает обмен данными между двумя устройствами (соединение точка-точка) в
 * соответствии с протоколом канального уровня. Обмен данными производится в виде фреймов. Каждый фрейм состоит
 * из признака начала фрейма, поля данных, контрольной суммы, признака конца фрейма / начала следующего.
 */

template <uint16_t RxDataLenLimit, uint16_t TxDataLenLimit>
class TT_DataLinkLayerPoint: public T_DataLinkLayer_base
{
    //Рабочий буфер приемника
    uint8_t RxBuf[RxDataLenLimit+2];

public:

    TT_DataLinkLayerPoint(void(*SendByteFunc_p)(uint8_t), void(*RxDataHandlerFunc_p)(uint8_t*, uint16_t))
        : T_DataLinkLayer_base(SendByteFunc_p, RxDataHandlerFunc_p)
    {}

    //Отправляет отрезок данных в виде одного или нескольких фреймов, согласно протоколу
    inline void Send(const uint8_t* Data_p, uint32_t Len)
    {
        SendData_base(Data_p, Len, TxDataLenLimit);
    }
    //Отправляет 1 байт данных в виде фрейма, согласно протоколу
    inline void Send(const uint8_t data)
    {
        SendData_base(&data, 1, TxDataLenLimit);
    }
    //Обрабатывает очередной байт, принятый через физический интерфейс
    void RxByteHandler(uint8_t NewByte)
    {
        if(NewByte==DLL_START_OR_END_FRAME)
        {
            if(FrameLen>2)	//если фрейм содержал данные помимо контрольной суммы
            {
                if(GOOD_CRC16_Q921 == crc)	//проверка контрольной суммы
                {
                    RxDataHandler_p(RxBuf, FrameLen-2);	//обработка принятых данных
                }
            }
            FrameLen=0;
            ReceivingData=true;
            crc=INIT_CRC16_Q921;
        }
        else
        {
            if(ReceivingData)
            {
                //обработка байтстаффинга
                if(ByteDeStaffing)
                {
                    ByteDeStaffing = false;
                    if(NewByte!=0x5D && NewByte!=0x5E)
                    {
                        ReceivingData=false;
                        //terminal_printf("\033[1;31mОШИБКА: RxByteHandler_base: Ошибка байтстаффинга в принятом фрейме!\033[0m\n");
                        return;
                    }
                    NewByte += 0x20;
                }
                else if(NewByte == DLL_STAF_TOKEN)
                {
                    ByteDeStaffing = true;
                    return;
                }
                //обработка данных
                if(FrameLen >= RxDataLenLimit+2)
                {
                    ReceivingData=false;
                    ByteDeStaffing=false;
                    //terminal_printf("\033[1;31mОШИБКА: RxByteHandler_base: Превышен лимит количества данных в принимаемом фрейме! Лимит:%u\033[0m\n", RxDataLenLimit);
                    return;
                }
                RxBuf[FrameLen++] = NewByte;
                crc = CRC16_Q921_update(crc, NewByte);
            }
        }
    }
};


#endif /* DATALINKLAYER_H_ */
