#ifndef EXPERIMENT_H
#define EXPERIMENT_H

#include <QObject>
#include <QString>
#include <QVector>
#include <QMap>
#include <QPointF>
#include <QDir>


class Experiment : public QObject
{
    Q_OBJECT

public:

    Experiment(QObject *parent = nullptr);

    static Experiment * load(const QString &fileName);

    QString getExperimentFile();
    int save(const QDir &dir);

    // TODO: private?
    QVector<double> timeVector;
    QVector<double> voltageDataSensor1;
    QVector<double> voltageDataSensor2;
    QVector<double> accelerationDataSensor1;
    QVector<double> accelerationDataSensor2;

    QMap<QString, QString> headerMap;

    QString getChannel1Name();
    double getChannel1Scalar();
    QString getChannel2Name();
    double getChannel2Scalar();
    int getSamplePerChannel();
    double getSamplingRate();
    QString getSubjectId();
    const QVector<double> getAccelerationData(int channelNum);


    void setChannel1Name(QString newValue);
    void setChannel1Scalar(double newValue);
    void setChannel2Name(QString newValue);
    void setChannel2Scalar(double newValue);
    void setSamplePerChannel(int newValue);
    void setSamplingRate(double newValue);
    void setSubjectId(QString newValue);
    void setHeight(double newValue);
    void setWeight(int newValue);


private:

    QString experimentFile;

    void readHeader();
    void readVoltageData();


signals:

public slots:

};

#endif // EXPERIMENT_H
