#include "serialsetting.h"
#include "ui_serialsetting.h"


/*================= Основной класс для окна настроек =====================*/
SerialSetting::SerialSetting(QWidget *parent) : QDialog(parent), ui(new Ui::SerialSetting){
    ui->setupUi(this);
    searchComPort();
    fillPortsParameters();
    updateSettings();
}


/*================= Обработка нажатия на кнопку "Обновить" =====================*/
void SerialSetting::on_redreshButton_clicked(){
    searchComPort();
    fillPortsParameters();
    updateSettings();
}


/*================= Обработка нажатия на кнопку "Подклютиться" =====================*/
void SerialSetting::on_apllySettButton_clicked(){
    updateSettings();
    emit sendText(currentSettings); // Когда собираемся передавать пишем слово Emit и указываем функцию из h, где прописан тип отправляемых параметров
    close();
}


/*================= Функция для поска свободных COM портов =====================*/
void SerialSetting::searchComPort(){
    ui->serialPortInfoListBox->clear();
    foreach(const QSerialPortInfo &info, QSerialPortInfo::availablePorts()) {
        QString PortString =  info.portName();
        ui->serialPortInfoListBox->addItem(info.portName());
    }
}


/*================= Функция для заполнения полей всеми параметрами =====================*/
void SerialSetting::fillPortsParameters(){
    ui->baudRateBox->clear();
    ui->baudRateBox->addItem(QStringLiteral("9600"), QSerialPort::Baud9600);
    ui->baudRateBox->addItem(QStringLiteral("19200"), QSerialPort::Baud19200);
    ui->baudRateBox->addItem(QStringLiteral("38400"), QSerialPort::Baud38400);
    ui->baudRateBox->addItem(QStringLiteral("115200"), QSerialPort::Baud115200);
    ui->dataBitsBox->clear();
    ui->dataBitsBox->addItem(QStringLiteral("5"), QSerialPort::Data5);
    ui->dataBitsBox->addItem(QStringLiteral("6"), QSerialPort::Data6);
    ui->dataBitsBox->addItem(QStringLiteral("7"), QSerialPort::Data7);
    ui->dataBitsBox->addItem(QStringLiteral("8"), QSerialPort::Data8);
    ui->dataBitsBox->setCurrentIndex(3);
    ui->parityBox->clear();
    ui->parityBox->addItem(tr("None"), QSerialPort::NoParity);
    ui->parityBox->addItem(tr("Even"), QSerialPort::EvenParity);
    ui->parityBox->addItem(tr("Odd"), QSerialPort::OddParity);
    ui->parityBox->addItem(tr("Mark"), QSerialPort::MarkParity);
    ui->parityBox->addItem(tr("Space"), QSerialPort::SpaceParity);
    ui->stopBitsBox->clear();
    ui->stopBitsBox->addItem(QStringLiteral("1"), QSerialPort::OneStop);
#ifdef Q_OS_WIN
    ui->stopBitsBox->addItem(tr("1.5"), QSerialPort::OneAndHalfStop);
#endif
    ui->stopBitsBox->addItem(QStringLiteral("2"), QSerialPort::TwoStop);
    ui->flowControlBox->clear();
    ui->flowControlBox->addItem(tr("None"), QSerialPort::NoFlowControl);
    ui->flowControlBox->addItem(tr("RTS/CTS"), QSerialPort::HardwareControl);
    ui->flowControlBox->addItem(tr("XON/XOFF"), QSerialPort::SoftwareControl);
}


/*================= Функция для обнавления пересенной со всеми настройками =====================*/
void SerialSetting::updateSettings(){
    currentSettings.name = ui->serialPortInfoListBox->currentText();

    currentSettings.baudRate = static_cast<QSerialPort::BaudRate>(
                    ui->baudRateBox->itemData(ui->baudRateBox->currentIndex()).toInt());
    currentSettings.stringBaudRate = QString::number(currentSettings.baudRate);

    currentSettings.dataBits = static_cast<QSerialPort::DataBits>(
                ui->dataBitsBox->itemData(ui->dataBitsBox->currentIndex()).toInt());
    currentSettings.stringDataBits = ui->dataBitsBox->currentText();

    currentSettings.parity = static_cast<QSerialPort::Parity>(
                ui->parityBox->itemData(ui->parityBox->currentIndex()).toInt());
    currentSettings.stringParity = ui->parityBox->currentText();

    currentSettings.stopBits = static_cast<QSerialPort::StopBits>(
                ui->stopBitsBox->itemData(ui->stopBitsBox->currentIndex()).toInt());
    currentSettings.stringStopBits = ui->stopBitsBox->currentText();

    currentSettings.flowControl = static_cast<QSerialPort::FlowControl>(
                ui->flowControlBox->itemData(ui->flowControlBox->currentIndex()).toInt());
    currentSettings.stringFlowControl = ui->flowControlBox->currentText();

    currentSettings.localEchoEnabled = ui->localEchoCheckBox->isChecked();
}






