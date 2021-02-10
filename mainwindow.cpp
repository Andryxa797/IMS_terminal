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
    StateButton(true, false, false, false, false, false, false, false, false, false);
    DefaultValueSettingIMS();
    connect(&serialPort, &SerialPort::RxBuf, this, &MainWindow::dataProcessing);
    connect(settingClass,SIGNAL(sendText(SerialSetting::Settings)),this, SLOT(openSerialPort(SerialSetting::Settings))); // Создаем свзяь сигнал -> слот
    ChartInit();
    ui->NamePathFolderlabel->setText("D:/DataTerminal/");

    timer = new QTimer();
    connect(timer, SIGNAL(timeout()), this, SLOT(slotTimerAlarm()));
    ui->Timerlabel->setText("Время со старта: 0");

    InitSurface();


    QFile file("D:/Hello.bin");
    if (!file.open(QIODevice::ReadWrite))
        return;
    QDataStream out(&file);
    out <<(quint16)5 << "\n";




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
    StateButton(false, true, true, false, true, false, true, false, true, true);
}

/*============ Функция обработки нажатия на кнопку "Выключить IMS" ==============*/
void MainWindow::on_OffIMSButton_clicked(){
    ttDataLinkLayerPoint.Send(IdOffIMS);
    StateButton(false, true, true, true, false, false, true, true, false, true);
}


/*============ Функция обработки нажатия на кнопку "Построить 3D график" ==============*/
void MainWindow::on_Buld3DChartButton_clicked(){
    BuldSurface(LastPathFile);
}
/*============ Функция обработки нажатия на кнопку "Выбрать данные для построения 3D графика" ==============*/
void MainWindow::on_GetDataForBuld3DChartButton_clicked(){
    QString Path = QFileDialog::getOpenFileName(this, tr("Open File"), "/D:/" , tr("*.txt"));
    if(Path.indexOf(".txt")!=-1){
        BuldSurface(Path);
    }
}

/*============ Функция обработки нажатия на кнопку "Старт" ==============*/
void MainWindow::on_startButton_clicked(){
    /*============ Отправка настроек для IMS ==============*/
    QString path2 = (QDateTime::currentDateTime().toString("yyyy-MM-dd"));
    QString pathFolder = ui->NamePathFolderlabel->text();
    if(!QDir(pathFolder + path2).exists()){
        QDir().mkdir(pathFolder + path2);
    }
    QString name = QDateTime::currentDateTime().toString("hh-mm-ss");
    QString type = ".txt";
    IonFile.open((pathFolder + path2 + '/' + name + type).toStdString(), std::ofstream::out | std::ofstream::app);
    LastPathFile = pathFolder + path2 + '/' + name + type;
    qDebug()<<'a';

    settingIMSSend();
    timer->start(1000); // И запустим таймер

    StateButton(false, false, false, false, false, true, false, false, false, false);
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
    StateButton(false, true, true, false, true, false, true, false, true, true);
    timer->stop(); // И запустим таймер
    secondsTimer = 0;
}

/*============ Функция обработки нажатия на кнопку "Отправка" ==============*/
void MainWindow::on_pushButton_clicked(){
    /*============ Предача написанной строки из поля LineEdit  ==============*/
    ui->sendlabel->setText((ui->sendlabel->toPlainText()) + ui->sendlineEdit->text());
    serialPort.MyWriteData(ui->sendlineEdit->text().toUtf8());
    if(LineBreakCheck){
        ui->sendlabel->setText((ui->sendlabel->toPlainText()) +"\r");
        serialPort.MyWriteData("\r\n");
    }

}


/*============ Функция обработки нажатия на кнопку "Очистить поля" ==============*/
void MainWindow::on_clearButtn_clicked(){
    /*============ Очистка полей приема и отправки данных  ==============*/
    ui->sendlabel->setText(" ");
    ui->labelExamSett->setText(" ");
}


/*============ Функция обработки нажатия на кнопку "Разьединить" ==============*/
void MainWindow::on_closeConButton_clicked(){
    /*============ COM порт будет отключен по нажатию кнопки ==============*/
    bool result = serialPort.closeSerialPort();
    if(result){
        StateButton(true, false, false, false, false, false, false, false, false, false);
        ui->statusBar->showMessage(tr("Подключений нет"));
    }else{
        StateButton(false, true, true, true, false, false, true, true, false, true);
    }

}

/*============ Функция для проерки Check Box на сглаживание углов графика ==============*/
void MainWindow::on_smoothiLineCheckBox_stateChanged(int arg1){
    smoothingLineChart = arg1;
}


void MainWindow::on_UpdateConnectComButton_clicked(){
    bool result = serialPort.closeSerialPort();
    if(result){
        StateButton(true, false, false, false, false, false, false, false, false, false);

    }else{
        StateButton(false, true, true, true, true, true, true, true, true, true);
    }
    openSerialPort(settingClass->currentSettings);
}

/*============ Функция обработки Check Box перенос строки ==============*/
void MainWindow::on_LineBreakCheckBox_stateChanged(int arg1){
    LineBreakCheck = arg1;
}


/*===================================================== Методы класса ==============================================*/

/*============ Функция открытия соединения с портом ==============*/
void MainWindow::openSerialPort(SerialSetting::Settings p)
{
    /*============ Даннная функция открывает COM порт ==============*/
    bool result = serialPort.openSerialPort(p);
    if(result){
        ui->statusBar->showMessage(tr("Подключен к %1 : %2, %3, %4, %5, %6").arg(p.name).arg(p.stringBaudRate).arg(p.stringDataBits).arg(p.stringParity).arg(p.stringStopBits).arg(p.stringFlowControl));
        StateButton(false, true, true, true, false, false, true, true, false, true);
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
void MainWindow::ChartInMainMenu(uint16_t len){

    chart->removeAllSeries(); // удаляем старые линии перед тем как построить новые

    /*============ Проверяется условие, нужно ли сглаживать углы на графике? ==============*/
    if(!smoothingLineChart){
        QLineSeries *seriesNew = new QLineSeries;
        seriesNew->setOpacity(1.0);
        QPen pen1(Qt::green, 3, Qt::SolidLine);
        seriesNew->setPen(pen1);
        for (int i = 0; i < (len-7)/2; i++) {
            seriesNew->append(i, ionogramIMS.IonogramData[i]);
        }
        chart->addSeries(seriesNew);
    }
    else{
        QSplineSeries *seriesNew = new QSplineSeries(chart);
        seriesNew->setOpacity(1.0);
        QPen pen1(Qt::green, 3, Qt::SolidLine);
        seriesNew->setPen(pen1);

        for (int i = 0; i < (len-7)/2; i++) {
            seriesNew->append(i, ionogramIMS.IonogramData[i]);
        }
        chart->addSeries(seriesNew);

    }
    chart->createDefaultAxes();


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
void MainWindow::StateButton(bool SettConn, bool closeCon, bool push, bool clear, bool start, bool stop, bool Update, bool OnIMS, bool OffIMS, bool NameFolder){
    /*============ ЗЗначение True разрешает нажатие на кнопку, False запрещает ==============*/
    ui->settingConnButton->setEnabled(SettConn);
    ui->closeConButton->setEnabled(closeCon);
    ui->pushButton->setEnabled(push);
    ui->clearButtn->setEnabled(clear);
    ui->startButton->setEnabled(start);
    ui->stopButton->setEnabled(stop);
    ui->UpdateConnectComButton->setEnabled(Update);
    ui->OnIMSButton->setEnabled(OnIMS);
    ui->OffIMSButton->setEnabled(OffIMS);
    ui->NamePathFolderButton->setEnabled(NameFolder);
}


/*============ Функция вызывается для обновления графика и записи пришедших данных в Label ==============*/
void MainWindow::dataProcessing(uint8_t* buff, uint16_t len){
    ui->labelExamSett->setText((ui->labelExamSett->toPlainText()) + *buff);
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
        ionogramIMS.serialNumberIonogram = (buff[4]<<24)|(buff[3]<<16)|(buff[2]<<8)|buff[1];
        WriteToFile(ionogramIMS.serialNumberIonogram, 1);
        ionogramIMS.serialNumberPackage = (buff[6]<<8)|buff[5];
        WriteToFile(ionogramIMS.serialNumberPackage, 2);
        int k = 7;
        for (int i = 0; i<(len-7)/2; i++) {
            ionogramIMS.IonogramData[i] = (buff[k+1]<<8)|buff[k];
            WriteToFile(ionogramIMS.IonogramData[i], 3);
            k+=2;
        }
        IonFile<<"]\n\n";
        ChartInMainMenu(len);
    }
}

/*============ Слот для обработки timeout() (таймера) ==============*/
void MainWindow::slotTimerAlarm(){
    secondsTimer++;
    QString second = QString::number(secondsTimer);
    ui->Timerlabel->setText(("Время со старта: ") + second);
}

/*============ Функция для записи вспомогательных символов в файл ==============*/
void MainWindow::WriteToFile(uint16_t val, int command){
    int CountNumber;
    uint16_t number;
    char bufNumber[10];

    if(command == 1){
        CountNumber = 0;
        number =  val;
        snprintf(bufNumber, sizeof(bufNumber), "%d", number);
        CountNumber = 0;
        while(number){
            number = number / 10;
            CountNumber++;
        }
        IonFile <<'<';
        for (int i = 0; i < CountNumber ; i++ )  IonFile<<bufNumber[i];
    }

    if(command == 2){
        CountNumber = 0;
        number =  val;
        snprintf(bufNumber, sizeof(bufNumber), "%d", number);
        CountNumber = 0;
        while(number){
            number = number / 10;
            CountNumber++;
        }
        IonFile <<' ';
        for (int i = 0; i < CountNumber ; i++ )  IonFile<<bufNumber[i];
        IonFile <<">\n[";
    }

    if(command == 3){
        CountNumber = 0;
        number =  val;
        snprintf(bufNumber, sizeof(bufNumber), "%d", number);
        CountNumber = 0;
        while(number){
            number = number / 10;
            CountNumber++;
        }
        CountNumber =  (CountNumber == 0) ? 1 : CountNumber;
        for (int i = 0; i < CountNumber ; i++ )  IonFile<<bufNumber[i];
        IonFile <<" ";
    }

    if(command == 4){
        IonFile <<"\n";
    }
}


/*============ Функция для инициализации осей 3D графика ==============*/
void MainWindow::InitSurface(){
    ui->SurfaceLayout->addWidget(containerSurface);
}

/*============ Функция для построения 3D графика ==============*/
void MainWindow::BuldSurface(QString path){
    modifierSurface->enableSqrtSinModel(path);
}












