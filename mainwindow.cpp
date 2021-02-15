#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "serialsetting.h"

// 1 SettConn, 2 clCon,  push, clear start  stop  Update OnIMS OffIMS NameFolder
// StateButton(true, false, false, false, false, false, false, false, false, false);
/*============ Инициализация глобальных переменных ==============*/
extern SerialPort serialPort;
extern void MywriteDataByte(uint8_t data);
extern void MyreadDataHandler(uint8_t* buff, uint16_t len);
extern TT_DataLinkLayerPoint<4008,20> ttDataLinkLayerPoint;


MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);
    StateButton(true, false, false, false, false, false, false, false);
    DefaultValueSettingIMS();
    connect(&serialPort, &SerialPort::RxBuf, this, &MainWindow::dataProcessing);
    connect(settingClass,SIGNAL(sendText(SerialSetting::Settings)),this, SLOT(openSerialPort(SerialSetting::Settings))); // Создаем свзяь сигнал -> слот
    ChartInit();
    ui->NamePathFolderlabel->setText("D:/DataTerminal/");
    timer = new QTimer();
    connect(timer, SIGNAL(timeout()), this, SLOT(slotTimerAlarm()));
    connect(AutoBuldTimer, SIGNAL(timeout()), this, SLOT(AutoBuld3DChart()));
    connect(modifierSurface, SIGNAL(StationButton), this, SLOT(Button3Dchart));
    ui->Timerlabel->setText("Время со старта: 0");
    CountIonSpinBox = 500;

    InitSurface();
    modifierSurface->EnableGraphSurface(" ", 0);

    ui->EnableChartcheckBox->setChecked(true);
    ui->EnableChartcheckBox->setEnabled(false);
}

/*===================================================== Обработка нажатий на кнопки ==============================================*/

/*============ Функция обработки нажатия на кнопку "Настройки подключения" ==============*/
void MainWindow::on_settingConnButton_clicked(){
    /*============ Для приема данных, необходимо создать обьект класса ==============*/
    settingClass->setModal(true);
    settingClass->show();
}

/*============ Функция обработки нажатия на кнопку "Включить IMS" ==============*/
void MainWindow::on_OnIMSButton_clicked(){
    ttDataLinkLayerPoint.Send(IdOnIMS);
    StateButton(false, true, true, false, true, false, true, true);
}

/*============ Функция обработки нажатия на кнопку "Выключить IMS" ==============*/
void MainWindow::on_OffIMSButton_clicked(){
    ttDataLinkLayerPoint.Send(IdOffIMS);
    StateButton(false, true, false, false, true, true, false, true);
}


/*============ Функция обработки нажатия на кнопку "Построить 3D график" ==============*/
void MainWindow::AutoBuld3DChart(){
    if(LastPathFile.indexOf(".bin") != -1){
        AutoBuldTimer->stop();
        Button3Dchart(false);
        BuldSurface(LastPathFile, CountIonSpinBox);
        ContErrorFindFile = 0;
    }
    else{
        ContErrorFindFile++;
        if(ContErrorFindFile > 0){
            AutoBuldTimer->stop();
            ui->AutoUpdate3DchartCheckBox->setChecked(false);
        }
        QMessageBox::information(this, tr("Ошибка"), "Файл не найден");
    }

}

/*============ Функция обработки нажатия на кнопку "Выбрать данные для построения 3D графика" ==============*/
void MainWindow::on_GetDataForBuld3DChartButton_clicked(){
    QString Path = QFileDialog::getOpenFileName(this, tr("Open File"), "/D:/" , tr("*.bin"));
    if(Path.indexOf(".bin")!=-1){
        Button3Dchart(false);
        BuldSurface(Path, CountIonSpinBox);
    }

}

void MainWindow::on_AutoUpdate3DchartCheckBox_stateChanged(int arg1){
    AutoUpdate3Dchart = arg1;
    if(AutoUpdate3Dchart){
        AutoBuldTimer->start(SecondUpdate3D * 1000);
    }
    else{
        AutoBuldTimer->stop();
        ui->GetDataForBuld3DChartButton->setEnabled(true);
    }
}


/*============ Функция обработки нажатия на кнопку "Старт" ==============*/
void MainWindow::on_startButton_clicked(){
    /*============ Отправка настроек для IMS ==============*/
    QString path2 = (QDateTime::currentDateTime().toString("yyyy-MM-dd"));
    QString pathFolder = ui->NamePathFolderlabel->text();
    if(pathFolder.indexOf("//") != -1){
        uint16_t pos = pathFolder.indexOf("//");
        pathFolder.remove(pos,1);
    }
    if(!QDir(pathFolder + path2).exists()){
        QDir().mkdir(pathFolder);
        QDir().mkdir(pathFolder + path2);
    }
    QString name = QDateTime::currentDateTime().toString("hh-mm-ss");
    QString type = ".bin";
    IonFile.open((pathFolder + path2 + '/' + name + type).toStdString(), std::ofstream::binary | std::ofstream::app);
    LastPathFile = pathFolder + path2 + '/' + name + type;
    settingIMSSend();
    timer->start(1000); // И запустим таймер
    StateButton(false, false, false, true, false, false, false, false);
    ui->EnableChartcheckBox->setEnabled(true);
    ui->EnableChartcheckBox->setChecked(false);
}

/*============ Функция обработки нажатия на кнопку "Выбрать папку для хранения" ==============*/
void MainWindow::on_NamePathFolderButton_clicked(){
    QString str = QFileDialog::getExistingDirectory(0, "Directory Dialog", "") + '/';
    ui->NamePathFolderlabel->setText(str);
}


/*============ Функция обработки нажатия на кнопку "Стоп" ==============*/
void MainWindow::on_stopButton_clicked(){
    ttDataLinkLayerPoint.Send(IdStopIMS);
    IonFile.close();
    StateButton(false, true, true, false, true, false, true, true);
    timer->stop(); // И запустим таймер
    secondsTimer = 0;
    ui->EnableChartcheckBox->setChecked(true);
    ui->EnableChartcheckBox->setEnabled(false);
}

void MainWindow::on_EnableChartcheckBox_stateChanged(int arg1){
    ChartDisable = arg1;
}

/*============ Функция обработки нажатия на кнопку "Разьединить" ==============*/
void MainWindow::on_closeConButton_clicked(){
    /*============ COM порт будет отключен по нажатию кнопки ==============*/
    bool result = serialPort.closeSerialPort();
    if(result){
        StateButton(true, false, false, false, false, false, false, false);
        ui->statusBar->showMessage(tr("Подключений нет"));
    }else{
        StateButton(false, true,  false, false, true, true, false, true);
    }

}

/*============ Функция для проерки Check Box на сглаживание углов графика ==============*/
void MainWindow::on_smoothiLineCheckBox_stateChanged(int arg1){
    smoothingLineChart = arg1;
}

/*============ Функция для проерки Check Box на количество ионограмм ==============*/
void MainWindow::on_CountIonSpinBox_textChanged(const QString &arg1){
    CountIonSpinBox = arg1.toInt();
}

void MainWindow::on_UpdateConnectComButton_clicked(){
    serialPort.closeSerialPort();
    openSerialPort(settingClass->currentSettings);
}


/*============ Функция обработки spin Box обнавления графика ==============*/
void MainWindow::on_spinBox_textChanged(const QString &arg1){
    SecondUpdate3D = arg1.toInt();
}


/*===================================================== Методы класса ==============================================*/

/*============ Функция открытия соединения с портом ==============*/
void MainWindow::openSerialPort(SerialSetting::Settings p)
{
    /*============ Даннная функция открывает COM порт ==============*/
    bool result = serialPort.openSerialPort(p);
    if(result){
        ui->statusBar->showMessage(tr("Подключен к %1 : %2, %3, %4, %5, %6").arg(p.name).arg(p.stringBaudRate).arg(p.stringDataBits).arg(p.stringParity).arg(p.stringStopBits).arg(p.stringFlowControl));
        StateButton(false, true, false, false, true, true, false, true);
    }
    else {
        QMessageBox::critical(this, tr("Ошибка"), "Отказано в доступе");
        ui->statusBar->showMessage(tr("Ошибка подключения"));
    }
}

/*============ Функция инициализации графика ==============*/
void MainWindow::ChartInit(){
    QChartView *chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);
    ui->ChartVLayout->addWidget(chartView);
    chart->legend()->setVisible(false);
    chart->setTitle("<h3>График</h3>");
}

/*============ Функция для постройки графика ==============*/
void MainWindow::ChartInMainMenu(){

    if(!ChartDisable){
        chart->removeAllSeries(); // удаляем старые линии перед тем как построить новые
        /*============ Проверяется условие, нужно ли сглаживать углы на графике? ==============*/
        if(!smoothingLineChart){
            QLineSeries *seriesNew = new QLineSeries;
            seriesNew->setOpacity(1.0);
            QPen pen1(Qt::green, 3, Qt::SolidLine);
            seriesNew->setPen(pen1);
            for (int i = 0; i < BuffLen; i++) {
                seriesNew->append(i, ionogramIMS.IonogramData[i]);
            }
            chart->addSeries(seriesNew);
        }
        else{
            QSplineSeries *seriesNew = new QSplineSeries(chart);
            seriesNew->setOpacity(1.0);
            QPen pen1(Qt::green, 3, Qt::SolidLine);
            seriesNew->setPen(pen1);

            for (int i = 0; i < BuffLen; i++) {
                seriesNew->append(i, ionogramIMS.IonogramData[i]);
            }
            chart->addSeries(seriesNew);

        }
        chart->createDefaultAxes();
    }

}
/*============ Функция для отправки параметром в IMS ==============*/
void MainWindow::settingIMSSend(){

    settingIMS.CountSelection = (ui->lineEditCountSelection->text().toUInt());
    settingIMS.IonogramSelection = (ui->lineEditTimeOpenGate->text().toUInt());
    settingIMS.TimeOpenGate = (ui->lineEditDelayOpenGate->text().toUInt());
    settingIMS.PeriodOpenGate = (ui->lineEditIonogramPeriod->text().toUInt());
    settingIMS.DelayOpenGate = (ui->lineEditSamplePeriod->text().toUInt());


    uint8_t buf[15];
    buf[0] = IdStartIMS;

    uint8_t *tCSel = (uint8_t *)&settingIMS.CountSelection;
    buf[1] = tCSel[0];
    buf[2] = tCSel[1];
    buf[3] = tCSel[2];
    buf[4] = tCSel[3];

    uint8_t *tISel = (uint8_t *)&settingIMS.IonogramSelection;
    buf[5] = tISel[0];
    buf[6] = tISel[1];
    buf[7] = tISel[2];
    buf[8] = tISel[3];

    uint8_t *tTOGate = (uint8_t *)&settingIMS.TimeOpenGate;
    buf[9] = tTOGate[0];
    buf[10] = tTOGate[1];

    uint8_t *tDOGate =  (uint8_t *)&settingIMS.DelayOpenGate;
    buf[11] = tDOGate[0];
    buf[12] = tDOGate[1];

    uint8_t *tPOGate =  (uint8_t *)&settingIMS.PeriodOpenGate;
    buf[13] = tPOGate[0];
    buf[14] = tPOGate[1];

    ttDataLinkLayerPoint.Send(buf, 15);
}


/*============ Функция для задания полей параметров IMS по умолчанию ==============*/
void MainWindow::DefaultValueSettingIMS(){
    /*============ Задача параметром по умолчанию для поля настройки ==============*/
    ui->lineEditCountSelection->setText("60000");
    ui->lineEditTimeOpenGate->setText("500");
    ui->lineEditDelayOpenGate->setText("1000");
    ui->lineEditSamplePeriod->setText("20");
    ui->lineEditIonogramPeriod->setText("3000");
}


/*============ Функция состояния кнопок  ==============*/
void MainWindow::StateButton(bool SettConn, bool closeCon, bool start, bool stop, bool Update, bool OnIMS, bool OffIMS, bool NameFolder){
    /*============ ЗЗначение True разрешает нажатие на кнопку, False запрещает ==============*/
    ui->settingConnButton->setEnabled(SettConn);
    ui->closeConButton->setEnabled(closeCon);
    ui->startButton->setEnabled(start);
    ui->stopButton->setEnabled(stop);
    ui->UpdateConnectComButton->setEnabled(Update);
    ui->OnIMSButton->setEnabled(OnIMS);
    ui->OffIMSButton->setEnabled(OffIMS);
    ui->NamePathFolderButton->setEnabled(NameFolder);
}


/*============ Функция вызывается для обновления графика и записи пришедших данных в Label ==============*/
void MainWindow::dataProcessing(uint8_t* buff){
    // ui->labelExamSett->setText((ui->labelExamSett->toPlainText()) + *buff);
    /*============ Прием параметров состояния IMS ==============*/
    if(buff[0] == IdConditionIMS){
        conditionIMS.faultCode = buff[1];
        conditionIMS.statusBits = buff[2];
        conditionIMS.CountSelection = (buff[3]<<24)|(buff[4]<<16)|(buff[5]<<8)|buff[6];
        conditionIMS.IonogramSelection = (buff[7]<<24)|(buff[8]<<16)|(buff[9]<<8)|buff[10];
        conditionIMS.TimeOpenGate = (buff[11]<<8)|buff[12];
        conditionIMS.DelayOpenGate = (buff[13]<<8)|buff[14];
        conditionIMS.PeriodOpenGate = (buff[15]<<8)|buff[16];
    }
    /*============ Прием данных с выборками IMS ==============*/
    if(buff[0] == IdIonogramIMS){
        memset(ionogramIMS.IonogramData, 0, sizeof(uint16_t) * BuffLen);
        ionogramIMS.serialNumberIonogram = (buff[4]<<24)|(buff[3]<<16)|(buff[2]<<8)|buff[1];
        ionogramIMS.serialNumberPackage = (buff[6]<<8)|buff[5];
        int k = 7;
        for (int i = 0; i<BuffLen; i++) {
            ionogramIMS.IonogramData[i] = (buff[k+1]<<8)|buff[k];
            k+=2;
        }
        WriteToFile();
        ChartInMainMenu();
    }
}

/*============ Функция для записи вспомогательных символов в файл ==============*/
void MainWindow::WriteToFile(){
    char begin = '<';
    char end = '>';
    IonFile.put(begin);
    IonFile.write((char*)&ionogramIMS.serialNumberIonogram, sizeof(ionogramIMS.serialNumberIonogram));
    IonFile.write((char*)&ionogramIMS.serialNumberPackage, sizeof(ionogramIMS.serialNumberPackage));
    IonFile.write((char*)ionogramIMS.IonogramData, sizeof(ionogramIMS.IonogramData));
    IonFile.put(end);
}

/*============ Слот для обработки timeout() (таймера) ==============*/
void MainWindow::slotTimerAlarm(){
    secondsTimer++;
    QString second = QString::number(secondsTimer);
    ui->Timerlabel->setText(("Время со старта: ") + second);
}

/*============ Функция для инициализации осей 3D графика ==============*/
void MainWindow::InitSurface(){
    ui->SurfaceLayout->addWidget(containerSurface);
}

void MainWindow::Button3Dchart(bool GetData){
    ui->GetDataForBuld3DChartButton->setEnabled(GetData);
    if(AutoUpdate3Dchart == true){
    AutoBuldTimer->start(SecondUpdate3D * 1000);
    }
}

void MainWindow::BuldSurface(QString path, quint16 CountIon){
   modifierSurface->EnableGraphSurface(path,  CountIon);
}















