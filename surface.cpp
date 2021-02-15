#include "surface.h"
#include "mainwindow.h"

#include <QtDataVisualization/QSurfaceDataProxy>
#include <QtDataVisualization/QHeightMapSurfaceDataProxy>
#include <QtDataVisualization/QSurface3DSeries>

#include <QtDataVisualization/QValue3DAxis>
#include <QtDataVisualization/Q3DTheme>
#include <QtGui/QImage>
#include <QtCore/qmath.h>
#include <QElapsedTimer>


const int sampleCountX = BuffLen;
const int sampleCountZ = 1000000;
//const float sampleMin = 0.0f;
//const float sampleMax = 1000000.0f;


MySurfaceGraph::MySurfaceGraph(Q3DSurface *surface) : m_graph(surface){
    m_graph->setAxisX(new QValue3DAxis);
    m_graph->setAxisY(new QValue3DAxis);
    m_graph->setAxisZ(new QValue3DAxis);
    m_sqrtSinProxy = new QSurfaceDataProxy();
    m_sqrtSinSeries = new QSurface3DSeries(m_sqrtSinProxy);
}


void MySurfaceGraph::SurfaceGraphProxy(QString path, quint16 CountIon){
    QSurfaceDataArray *dataArray = new QSurfaceDataArray;
    dataArray->reserve(sampleCountZ);
    float x = 0, y = 0, z = 0;
    std::ifstream file;
    file.open(path.toStdString(), std::ifstream::binary | std::ifstream::in| std::ifstream::ate);
    if(file.is_open()){
        MainWindow::IonogramIMS structIMS;
        uint32_t sizeFile = file.tellg();
        uint32_t sizeStruct = sizeof(structIMS.serialNumberIonogram) + sizeof(structIMS.serialNumberPackage) + sizeof(structIMS.IonogramData) + sizeof('<') + sizeof('>');
        uint32_t countStruct = sizeFile / sizeStruct;
        int32_t BegIndex = countStruct - CountIon;
        if(BegIndex<0){
            BegIndex = 0;
        }
        qDebug()<< "BegIndex - " <<BegIndex;
        qDebug()<< "Size file - " <<sizeFile;
        qDebug()<< "Size struct - "<<sizeStruct;
        qDebug()<< "Count struct in file  - "<<countStruct;
        file.seekg(BegIndex * sizeStruct, file.beg);

        while(!file.eof()){
            char begin = 0;
            char end = 0;
            begin = file.get();
            if (begin == '<')
            {
                file.read((char*)&structIMS.serialNumberIonogram, sizeof(structIMS.serialNumberIonogram));
                file.read((char*)&structIMS.serialNumberPackage, sizeof(structIMS.serialNumberPackage));
                file.read((char*)&structIMS.IonogramData, sizeof(structIMS.IonogramData));
                file.get(end);
//                if(!(end == '>')) {
//                    qDebug() << "Error Find '>' ";
//                }

                QSurfaceDataRow *newRow = new QSurfaceDataRow(sampleCountX);
                z = structIMS.serialNumberIonogram;
                int16_t index = 0;
                for(int i = 0; i<BuffLen; i++){
                    x = i;
                    y = structIMS.IonogramData[i];
                    (*newRow)[index++].setPosition(QVector3D(x, y, z));
                }
                *dataArray << newRow;
            }
        }
        file.close();
        m_sqrtSinProxy->resetArray(dataArray);
    }
}

void MySurfaceGraph::EnableGraphSurface(QString path, quint16 CountIon){
    m_sqrtSinSeries->setDrawMode(QSurface3DSeries::DrawSurface);
    m_sqrtSinSeries->setFlatShadingEnabled(false);
    m_graph->axisX()->setLabelFormat("%.2f");
    m_graph->axisZ()->setLabelFormat("%.2f");
    m_graph->axisX()->setLabelAutoRotation(90);
    m_graph->axisY()->setLabelAutoRotation(90);
    m_graph->axisZ()->setLabelAutoRotation(90);
    if(!(CountIon == 0)){
        SurfaceGraphProxy(path, CountIon);
        m_graph->removeSeries(m_sqrtSinSeries);
        m_graph->addSeries(m_sqrtSinSeries);
        emit StationButton(true);
    }
}



void MySurfaceGraph::adjustXMin(int min)
{
    float minX = m_stepX * float(min) + m_rangeMinX;

    int max = m_axisMaxSliderX->value();
    if (min >= max) {
        max = min + 1;
        m_axisMaxSliderX->setValue(max);
    }
    float maxX = m_stepX * max + m_rangeMinX;

    setAxisXRange(minX, maxX);
}

void MySurfaceGraph::adjustXMax(int max)
{
    float maxX = m_stepX * float(max) + m_rangeMinX;

    int min = m_axisMinSliderX->value();
    if (max <= min) {
        min = max - 1;
        m_axisMinSliderX->setValue(min);
    }
    float minX = m_stepX * min + m_rangeMinX;

    setAxisXRange(minX, maxX);
}

void MySurfaceGraph::adjustZMin(int min)
{
    float minZ = m_stepZ * float(min) + m_rangeMinZ;

    int max = m_axisMaxSliderZ->value();
    if (min >= max) {
        max = min + 1;
        m_axisMaxSliderZ->setValue(max);
    }
    float maxZ = m_stepZ * max + m_rangeMinZ;

    setAxisZRange(minZ, maxZ);
}

void MySurfaceGraph::adjustZMax(int max)
{
    float maxX = m_stepZ * float(max) + m_rangeMinZ;

    int min = m_axisMinSliderZ->value();
    if (max <= min) {
        min = max - 1;
        m_axisMinSliderZ->setValue(min);
    }
    float minX = m_stepZ * min + m_rangeMinZ;

    setAxisZRange(minX, maxX);
}


void MySurfaceGraph::setAxisXRange(float min, float max)
{
    m_graph->axisX()->setRange(min, max);
}

void MySurfaceGraph::setAxisZRange(float min, float max)
{
    m_graph->axisZ()->setRange(min, max);
}


