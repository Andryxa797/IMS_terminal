#include "surface.h"

#include <QtDataVisualization/QSurfaceDataProxy>
#include <QtDataVisualization/QHeightMapSurfaceDataProxy>
#include <QtDataVisualization/QSurface3DSeries>

#include <QtDataVisualization/QValue3DAxis>
#include <QtDataVisualization/Q3DTheme>
#include <QtGui/QImage>
#include <QtCore/qmath.h>
#include <QElapsedTimer>


const int sampleCountX = 2001;
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


void MySurfaceGraph::fillSqrtSinProxy(QString path){
    //    float stepX = (sampleMax - sampleMin) / float(sampleCountX - 1);
    //    float stepZ = (sampleMax - sampleMin) / float(sampleCountZ - 1);
    QSurfaceDataArray *dataArray = new QSurfaceDataArray;
    dataArray->reserve(sampleCountZ);


    float x = 0, y = 0, z = 0;
    QFile file(path);
    QString data = "";
    QStringList list;


    int p = 0;
    int32_t CountLineBefor = 0;
    int32_t CountLineAfter = 0;


        QElapsedTimer timer;
        timer.start();
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return;
    while(!file.atEnd()){
        file.readLine();
        CountLineBefor++;
    }
            qDebug() << "До закрытия " << timer.elapsed() << "milliseconds";
    file.close();
            qDebug() << "После закрытия " << timer.elapsed() << "milliseconds";
            timer.start();
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return;

    qDebug()<<CountLineBefor;
    while(!file.atEnd())
    {
        data = file.readLine();
        CountLineAfter++;
        if(CountLineAfter>CountLineBefor - 1000){
            if(data.indexOf('<')!=-1){
                data.remove('<');
                data.remove('>');
                list = data.split(QRegExp("\\s+"));
                //            uint16_t Ionogram = list[0].toInt();
                //            uint16_t Package = list[1].toInt();
                z = p++;
            }
            if(data.indexOf('[')!=-1 && data.indexOf(']')!=-1){
                QSurfaceDataRow *newRow = new QSurfaceDataRow(sampleCountX);
                data.remove('[');
                data.remove(']');
                list = data.split(QRegExp("\\s+"));
                uint16_t index = 0;
                for(int i = 0; i<list.count(); i++){
                    x = i;
                    y = list[i].toUInt();
                    (*newRow)[index++].setPosition(QVector3D(x, y, z));
                }
                while(list.count()<2000){
                    x = index;
                    y = 0;
                    (*newRow)[index++].setPosition(QVector3D(x, y, z));
                }
                *dataArray << newRow;
                qDebug()<<dataArray->count();
            }
        }
    }
    m_sqrtSinProxy->resetArray(dataArray);
    qDebug() << "После записи " << timer.elapsed() << "milliseconds";
}


void MySurfaceGraph::enableSqrtSinModel(QString path){
    m_sqrtSinSeries->setDrawMode(QSurface3DSeries::DrawSurface);
    m_sqrtSinSeries->setFlatShadingEnabled(false);
    m_graph->axisX()->setLabelFormat("%.2f");
    m_graph->axisZ()->setLabelFormat("%.2f");
    m_graph->axisX()->setLabelAutoRotation(90);
    m_graph->axisY()->setLabelAutoRotation(90);
    m_graph->axisZ()->setLabelAutoRotation(90);
    m_graph->removeSeries(m_sqrtSinSeries);

    fillSqrtSinProxy(path);
    m_graph->addSeries(m_sqrtSinSeries);

    //        m_rangeMinX = sampleMin;
    //        m_rangeMinZ = sampleMin;
    //        m_stepX = (sampleMax - sampleMin) / float(sampleCountX - 1);
    //        m_stepZ = (sampleMax - sampleMin) / float(sampleCountZ - 1);
    //        m_axisMinSliderX->setMaximum(sampleCountX - 2);
    //        m_axisMinSliderX->setValue(0);
    //        m_axisMaxSliderX->setMaximum(sampleCountX - 1);
    //        m_axisMaxSliderX->setValue(sampleCountX - 1);
    //        m_axisMinSliderZ->setMaximum(sampleCountZ - 2);
    //        m_axisMinSliderZ->setValue(0);
    //        m_axisMaxSliderZ->setMaximum(sampleCountZ - 1);
    //        m_axisMaxSliderZ->setValue(sampleCountZ - 1);

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


