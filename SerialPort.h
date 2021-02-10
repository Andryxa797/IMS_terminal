#ifndef SERIALPORT_H
#define SERIALPORT_H

#include <QtSerialPort/QSerialPort>
#include "serialsetting.h"
#include <QDebug>
#include <QObject>

#include "DataLinKLayer.h"
#include "mainwindow.h"


class SerialPort : public QSerialPort{
Q_OBJECT
public:

explicit SerialPort();
void RxBufV(uint8_t* buff, uint16_t len); // Функция вызывает emit - синал и передает данные которые уже обработаны

signals:
    void RxBuf(uint8_t* buff, uint16_t len); // Сигнал который передает буфер с данными и его длину

public slots:
    void MyReadData(); // Слот для приема данных из порта
    void MyWriteData(const QByteArray &data); // Слот для отправки данных в порт
    bool openSerialPort(SerialSetting::Settings p); // Слот для открытия COM порта
    bool closeSerialPort(); // Слот для закрытия COM порта
};


#endif // SERIALPORT_H
