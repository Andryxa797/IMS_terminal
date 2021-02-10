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


#include <QtDataVisualization/Q3DSurface>
using namespace QtDataVisualization;



QT_CHARTS_USE_NAMESPACE


// BitMap

namespace Ui {  class MainWindow; }

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    #define BuffLen 2000

    explicit MainWindow(QWidget *parent = 0);
    void closeSerialPort();
    void ChartInMainMenu(uint16_t len);
    void settingIMSSend();
    void DefaultValueSettingIMS();
    void StateButton(bool SettConn, bool closeCon, bool push, bool clear, bool start, bool stop, bool Update, bool OnIMS, bool OffIMS, bool NameFolder);
    void ChartInit();
    void WriteToFile(uint16_t val, int command);
    void BuldSurface(QString path);
    void InitSurface();
    int countLenBuf = 0;


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
public slots:
    void openSerialPort(SerialSetting::Settings curset); // Слот для приема структуры данных из класса Настройка
    void dataProcessing(uint8_t* buff, uint16_t len);
    void slotTimerAlarm();



private slots:
    void on_settingConnButton_clicked(); // Слот по обработке кнопки " Настройки соединения"
    void on_closeConButton_clicked(); // Слот по обработке кнопки "Разьедениться"
    void on_pushButton_clicked(); // Слот по обработке кнопки "Отправка"
    void on_clearButtn_clicked(); // Слот по обработке кнопки "Очистка полей"
    void on_startButton_clicked(); // Слот по обработке кнопки "Старт"
    void on_smoothiLineCheckBox_stateChanged(int arg1);
    void on_UpdateConnectComButton_clicked();
    void on_LineBreakCheckBox_stateChanged(int arg1);
    void on_OnIMSButton_clicked();
    void on_OffIMSButton_clicked();
    void on_stopButton_clicked();
    void on_NamePathFolderButton_clicked();
    void on_Buld3DChartButton_clicked();

    void on_GetDataForBuld3DChartButton_clicked();

private:
    Ui::MainWindow *ui;

    QGridLayout *gridLayout;
    bool smoothingLineChart = false;

    SerialSetting *settingClass = new SerialSetting;
    bool LineBreakCheck = false;

    QChart *chart = new QChart();

    SettingIMS settingIMS;
    IonogramIMS ionogramIMS;
    ConditionIMS conditionIMS;

    std::ofstream IonFile;

    QTimer *timer;
    quint16 secondsTimer = 0;
    quint16 minutsTimer = 0;

    QString LastPathFile;


    Q3DSurface *graphSurface = new Q3DSurface();
    MySurfaceGraph *modifierSurface = new MySurfaceGraph(graphSurface);
    QWidget *containerSurface = QWidget::createWindowContainer(graphSurface);


};







#endif // MAINWINDOW_H
