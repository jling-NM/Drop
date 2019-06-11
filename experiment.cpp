#include "experiment.h"

#include <QFile>
#include <QMessageBox>
#include <QTextStream>
#include <QDebug>
#include <QVector>
#include <QPointF>
#include <QDateTime>



Experiment::Experiment(QObject *parent) : QObject(parent)
{   

    this->headerMap.insert("Channel1 SensorName", "Sensor 1");
    this->headerMap.insert("Channel2 SensorName", "Sensor 2");
    this->headerMap.insert("Channel1 SensorScalar", "1.00");
    this->headerMap.insert("Channel2 SensorScalar", "1.00");
    this->headerMap.insert("Date", "");
    this->headerMap.insert("Drop Weight(g)", "450");
    this->headerMap.insert("Drop Height(m)", "2.25");
    this->headerMap.insert("Samples Per Channel", "");
    this->headerMap.insert("Sampling Rate(Hz)", "");
    this->headerMap.insert("SubjectID", "");
    this->headerMap.insert("Time", "");
    this->headerMap.insert("X_Dimension", "Time");
    this->headerMap.insert("Y_Unit_Label", "Volts");

    /* generate a new filename from experiment */
    QDateTime dt = QDateTime::currentDateTime();
    this->headerMap["Time"] = dt.time().toString("hhmmss");
    this->headerMap["Date"] = dt.date().toString("yyyyMMdd");

}




Experiment * Experiment::load(const QString &fileName) {

    Experiment *e = new Experiment();

    QFile file(fileName);

    if(!file.open(QFile::ReadOnly)) {
        QMessageBox msgBox;
        msgBox.setWindowTitle(tr("Unable to open file"));
        msgBox.setText(file.errorString());
        msgBox.exec();
        return nullptr;
    }

    // set experiment filename
    e->experimentFile = fileName;


    QTextStream in(&file);

    /* this is slow, basic implementation
        QFile could be used directly and loading into fully initialiezed array
        would all be faster
        TODO: refactor for large file load
    */

    // TODO: must be better way of doing this time axis
    double time = 0.0;

    while (!in.atEnd()) {
        QString line = in.readLine();

        if(line.startsWith(tr("#"))) {
            // put in header;
            e->headerMap.insert(line.mid(2).split(':')[0].trimmed(), line.mid(2).split(':')[1].trimmed());
        } else {
            // put into numeric data vector
            e->timeVector.append(time);
            e->voltageDataSensor1.append( line.split(',')[0].toDouble() );
            e->voltageDataSensor2.append( line.split(',')[1].toDouble() );

            // increment
            time+=1.0/e->headerMap["Sampling Rate(Hz)"].toDouble();
        }

    }


    file.close();

    return e;
}



/**
 * @brief Get experiment filename
 * @return
 */
QString Experiment::getExperimentFile() {
    /* if experiment already has a filename, return that */
    if( this->experimentFile.isEmpty()) {
        // generate a new filename from experiment //
        this->experimentFile = this->headerMap["SubjectID"] + "_" + this->headerMap["Date"] + "_" + this->headerMap["Time"] + ".drp";
    }

    return this->experimentFile;
}



/**
 * @brief Write experiment to disk at DIR
 * @param dir
 * @return
 */
int Experiment::save(const QDir &dir)
{

    // TODO: at some point voltage needs to be scaled
    // e->voltageDataSensor1.append(  (line.split(',')[0].toDouble()/e->headerMap["Channel1 SensorScalar"].toDouble())  );

    //QString oFileName = this->getExperimentFile().split("/").last();
    QString oFileName = this->getExperimentFile();
    QFile file(dir.path() + "/" + oFileName);

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qDebug() << "file opening failed";
        return 1;
    }


    QTextStream out(&file);


    QMap<QString, QString>::const_iterator i = headerMap.constBegin();
    while(i != headerMap.constEnd()) {
        out << "# " << i.key() << ":" << i.value() << endl;
        ++i;
    }

    for(auto i = 0; i < voltageDataSensor1.size(); i++) {
        out << voltageDataSensor1.at(i) << "," << voltageDataSensor2.at(i) << endl;
    }

    return 0;
}



QString Experiment::getChannel1Name()
{
    return headerMap["Channel1 SensorName"];
}
double Experiment::getChannel1Scalar()
{
    return headerMap["Channel1 SensorScalar"].toDouble();
}
QString Experiment::getChannel2Name()
{
    return headerMap["Channel2 SensorName"];
}
double Experiment::getChannel2Scalar()
{
    return headerMap["Channel2 SensorScalar"].toDouble();
}
int Experiment::getSamplePerChannel()
{
    return headerMap["Samples Per Channel"].toInt();
}
double Experiment::getSamplingRate()
{
    return headerMap["Sampling Rate(Hz)"].toDouble();
}
QString Experiment::getSubjectId()
{
    return headerMap["SubjectID"];
}

const QVector<double> Experiment::getAccelerationData(int channelNum)
{

    if( channelNum == 1 ) {

        if( accelerationDataSensor1.empty() ) {

            double channelScalar = getChannel1Scalar()/1000.0;
            accelerationDataSensor1.resize(voltageDataSensor1.size());
            std::transform(voltageDataSensor1.begin(),
                           voltageDataSensor1.end(),
                           accelerationDataSensor1.begin(),
                           [&](double i) { return i/channelScalar; }
                           );
        }
        return accelerationDataSensor1;

    } else {

        if( accelerationDataSensor2.empty() ) {

            double channelScalar = getChannel2Scalar()/1000.0;
            accelerationDataSensor2.resize(voltageDataSensor2.size());
            std::transform(voltageDataSensor2.begin(),
                           voltageDataSensor2.end(),
                           accelerationDataSensor2.begin(),
                           [&](double i) { return i/channelScalar; }
                           );
        }
        return accelerationDataSensor2;

    }
}


void Experiment::setChannel1Name(QString newValue)
{
    headerMap["Channel1 SensorName"] = newValue;
}
void Experiment::setChannel1Scalar(double newValue)
{
    headerMap["Channel1 SensorScalar"] = QString::number(newValue);
}
void Experiment::setChannel2Name(QString newValue)
{
    headerMap["Channel2 SensorName"] = newValue;
}
void Experiment::setChannel2Scalar(double newValue)
{
    headerMap["Channel2 SensorScalar"] = QString::number(newValue);
}
void Experiment::setSamplePerChannel(int newValue)
{
    headerMap["Samples Per Channel"] = QString::number(newValue);
}
void Experiment::setSamplingRate(double newValue)
{
    headerMap["Sampling Rate(Hz)"] = QString::number(newValue);
}
void Experiment::setSubjectId(QString newValue)
{
    headerMap["SubjectID"] = newValue;
}
void Experiment::setHeight(double newValue)
{
    headerMap["Drop Height(m)"] = QString::number(newValue);
}
void Experiment::setWeight(int newValue)
{
    headerMap["Drop Weight(g)"] = QString::number(newValue);
}

