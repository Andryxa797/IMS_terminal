#ifndef SERIALSETTING_H
#define SERIALSETTING_H

#include <QDialog>
#include <QtSerialPort/QSerialPort>
#include <QSerialPortInfo>


namespace Ui { class SerialSetting; }

class SerialSetting : public QDialog {
    Q_OBJECT

public:
    explicit SerialSetting(QWidget *parent = 0);
    struct Settings {
           QString name;
           qint32 baudRate;
           QString stringBaudRate;
           QSerialPort::DataBits dataBits;
           QString stringDataBits;
           QSerialPort::Parity parity;
           QString stringParity;
           QSerialPort::StopBits stopBits;
           QString stringStopBits;
           QSerialPort::FlowControl flowControl;
           QString stringFlowControl;
           bool localEchoEnabled;
       };

    Settings currentSettings;

    void searchComPort();
    void fillPortsParameters();
    void updateSettings();
/*============ Создаем сигнал для отправки ==============*/
signals:
    void sendText(SerialSetting::Settings);  // Функция которой не нужен прототип, достаточно только название и тип передаваемого параметра


public slots:
    void on_redreshButton_clicked();
    void on_apllySettButton_clicked();

private:
    Ui::SerialSetting *ui;

};

#endif // SERIALSETTING_H
