#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtCore/QtGlobal>
#include <QtSerialPort/QSerialPort>
#include "serialsetting.h"
#include <QtCore/QtGlobal>
#include <QMessageBox>

#include <QtWidgets/QApplication>
#include <QtWidgets/QMainWindow>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QGridLayout>
#include <QBarCategoryAxis>
#include <QSplineSeries>
#include <QValueAxis>

#include <QFileDialog>
#include <QFile>
#include <QDir>

#include <QDebug>
#include <QTimer>
#include "CRC.h"
#include "DataLinKLayer.h"
#include "SerialPort.h"
#include "surface.h"
#include <iostream>
#include <fstream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <QDateTime>
#include <QThread>
#include <QtDataVisualization/Q3DSurface>
QT_CHARTS_USE_NAMESPACE
using namespace QtDataVisualization;



/*======== Длина буфера для приема 1 ионограммы ===============*/
#define BuffLen 2000



namespace Ui {  class MainWindow; }

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    void closeSerialPort();
    void ChartInMainMenu();
    void settingIMSSend();
    void DefaultValueSettingIMS();
    void StateButton(bool SettConn, bool closeCon, bool start, bool stop, bool Update, bool OnIMS, bool OffIMS, bool NameFolder);
    void ChartInit();
    void WriteToFile();
    void InitSurface();
    void BuldSurface(QString path, quint16 CountIon); // Сигнал для построения 3D графика

/*==================== Перечень и структура пакетов, отправляемых из ПЭВМ ===========================================*/
const uint8_t IdOnIMS = 0x01; //  По этой команде включается высоковольтная часть IMS
const uint8_t IdOffIMS = 0x02; // По этой команде выключается высоковольтная часть IMS
const uint8_t IdStartIMS = 0x03; // По этой команде начинается отправка
/*==================== Струтура данных передаваемых в IMS  при старте ===========================================*/
struct SettingIMS {
       uint32_t CountSelection; // Количество выборок
       uint32_t IonogramSelection; // период между выборками (нс)
       uint16_t TimeOpenGate; // длительность открытия затвора (мкс)
       uint16_t PeriodOpenGate; // периодичность открытия затвора (мкс)
       uint16_t DelayOpenGate; // задержка от открытия затвора до начала оцифровки (мкс)

   };
const uint8_t IdStopIMS = 0x04; // По этой команде IMS прекращает оцифровку и передачу данных в ПЭВМ

/*==================== Перечень и структура пакетов, отправляемых из IMS ===========================================*/
const uint8_t IdIonogramIMS = 0x20; // Id прихода Ионограмма
/*==================== Струтура данных принятых ионограмм ===========================================*/
struct IonogramIMS {
        uint32_t serialNumberIonogram; // циклический порядковый номер ионограммы от получения команды «Старт», начиная с 1
        uint16_t serialNumberPackage; // порядковый номер пакета с выборками в пределах одной ионограммы – от 1 до максимума
        uint16_t IonogramData[BuffLen]; // Выборки
   };
const uint8_t IdConditionIMS = 0x10; // Id прихода состояния IMS
/*==================== Струтура данных состояния IMS ===========================================*/
struct ConditionIMS{
        uint8_t faultCode; // код неисправности (0 – неисправностей нет)
         /*биты состояния элементов IMS:
        бит 0 – высоковольтный источник (1 – включен, 0 – выключен),
        бит 1 – система управления ионным затвором (1 – включена, 0 – выключена),
        бит 2 – трансимпедансный усилитель (1 – включен, 0 – выключен),
        биты[7-3] – всегда 0.*/
        uint8_t statusBits;
        /* Текущие параметры работы */
        uint32_t CountSelection; // Количество выборок
        uint32_t IonogramSelection; // период между выборками (нс)
        uint16_t TimeOpenGate; // длительность открытия затвора (мкс)
        uint16_t DelayOpenGate; // задержка от открытия затвора до начала оцифровки (мкс)
        uint16_t PeriodOpenGate; // периодичность открытия затвора (мкс)
};

/*============ Создаем слот для открытия порта ==============*/
//signals:
    //void BuldSurface(QString path, quint16 CountIon); // Сигнал для построения 3D графика

public slots:
    void openSerialPort(SerialSetting::Settings curset); // Слот для приема структуры данных из класса Настройка
    void dataProcessing(uint8_t* buff); // Слот для обработки полученных данных
    void slotTimerAlarm(); // Слот для обработки таймера
    void Button3Dchart(bool GetData);



private slots:
    void on_settingConnButton_clicked(); // Слот обработки нажатия кнопки "Настройки соединения"
    void on_closeConButton_clicked(); // Слот обработки нажатия кнопки"Разьедениться"
    void on_smoothiLineCheckBox_stateChanged(int arg1); // Слот по обработке Check Box "Сглаживать ли углы?"
    void on_UpdateConnectComButton_clicked(); // Слот обработки нажатия кнопки "Переподключиться"
    void on_OnIMSButton_clicked();// Слот обработки нажатия кнопки "Включить IMS"
    void on_OffIMSButton_clicked(); // Слот обработки нажатия кнопки"Выключить IMS"
    void on_startButton_clicked(); // Слот обработки нажатия кнопки "Старт"
    void on_stopButton_clicked(); // Слот обработки нажатия кнопки "Стоп"
    void on_NamePathFolderButton_clicked(); // Слот обработки нажатия кнопки "Выбрать папку для хранения данных"
    void on_GetDataForBuld3DChartButton_clicked(); // Слот обработки нажатия кнопки "Выбрать данные для построения"
    void on_CountIonSpinBox_textChanged(const QString &arg1); // Слот по обработке Spin Box "Отображение ионнограмм:"
    void on_spinBox_textChanged(const QString &arg1); // Слот по обработке Spin Box "Обновление 3D графика(с):"
    void on_EnableChartcheckBox_stateChanged(int arg1); // Слот по обработке Check Box "Заблокировать изображение на графике"
    void on_AutoUpdate3DchartCheckBox_stateChanged(int arg1);
    void AutoBuld3DChart();

private:
    Ui::MainWindow *ui;
    uint SecondUpdate3D = 1;
    bool smoothingLineChart = false;
    bool ChartDisable = false;
    bool AutoUpdate3Dchart = false;
    SerialSetting *settingClass = new SerialSetting;
    QChart *chart = new QChart();
    SettingIMS settingIMS;
    IonogramIMS ionogramIMS;
    ConditionIMS conditionIMS;
    std::ofstream IonFile;
    QTimer *timer;
    QTimer *AutoBuldTimer = new QTimer;
    quint16 secondsTimer = 0;
    quint16 minutsTimer = 0;
    QString LastPathFile;
    uint16_t CountIonSpinBox;
    Q3DSurface *graphSurface = new Q3DSurface();
    MySurfaceGraph *modifierSurface = new MySurfaceGraph(graphSurface);
    QWidget *containerSurface = QWidget::createWindowContainer(graphSurface);
    uint ContErrorFindFile = 0;

};


#endif // MAINWINDOW_H
