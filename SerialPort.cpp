#include "SerialPort.h"

/*============ Инициализация глобальных переменных ==============*/
SerialPort serialPort;
QSerialPort *serial;
void MywriteDataByte(uint8_t data);
void MyreadDataHandler(uint8_t* buff, uint16_t len);
TT_DataLinkLayerPoint<4008,20> ttDataLinkLayerPoint(MywriteDataByte, MyreadDataHandler);

/*============ Конструктор для класса SerialPort ==============*/
SerialPort::SerialPort(){
    /*============ Инициализируем все перменные и сигналы ==============*/
    serial = new QSerialPort();
    QObject::connect(serial, SIGNAL(readyRead()), this, SLOT(MyReadData()));
}


/*============ Функция записи данных из COM ==============*/
void SerialPort::MyReadData(){
    QByteArray Newdata = serial->readAll();
    int len = Newdata.size();
    for(int i = 0; i<len; i++){
        ttDataLinkLayerPoint.RxByteHandler((uint8_t)Newdata[i]);
    }
}


/*============ Функция записи данных в COM ==============*/
void SerialPort::MyWriteData(const QByteArray &data){
    serial->write(data);
}


/*============ Функция для открытия порта ==============*/
bool SerialPort::openSerialPort(SerialSetting::Settings p){
    serial->setPortName(p.name);
    serial->setBaudRate(p.baudRate);
    serial->setDataBits(p.dataBits);
    serial->setParity(p.parity);
    serial->setStopBits(p.stopBits);
    serial->setFlowControl(p.flowControl);
    bool result = serial->open(QIODevice::ReadWrite) ?  true :  false;
    return result;
}


/*============ Функция для закрытия порта ==============*/
bool SerialPort::closeSerialPort(){
    if (serial->isOpen()){
        serial->close(); return true;}
                   else{return false;}
}

/* Функция для отпраки данных, которые прошли проверку контрольной суммы, в класс главного меню, где будут проходить их дальнейшая обработка */
/* Использование данного костыли обуслевленно тем, что произвести сигнал из функции не принадлежащий не к одному из классов нельзя*/
void SerialPort::RxBufV(uint8_t* buff, uint16_t len){
    emit RxBuf(buff, len);
}

/*============ Функция отправки данных в COM переменной Uint8_t ==============*/
void MywriteDataByte(uint8_t data){
    QByteArray qData;
    qData.append((char)data);
    //qDebug()<<qData;
    serialPort.MyWriteData(qData);
}


/*============ Функция для обработки полученных данных ==============*/
void MyreadDataHandler(uint8_t* buff, uint16_t len){
    serialPort.RxBufV(buff, len);
}


