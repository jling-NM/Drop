#include "sensorsettingsdialog.h"
#include "ui_sensorsettingsdialog.h"
#include <QDebug>
#include <QSettings>


SensorSettingsDialog::SensorSettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SensorSettingsDialog)
{
    ui->setupUi(this);

    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
    ui->leSen1Label->setText(settings.value("Sensor1/label").toString());
    ui->leSen1Scale->setText( QString("%1").arg( settings.value("Sensor1/scalar").toDouble(), 5, 'f', 2, '0' ) );
    ui->leSen2Label->setText(settings.value("Sensor2/label").toString());
    ui->leSen2Scale->setText( QString("%1").arg( settings.value("Sensor2/scalar").toDouble(), 5, 'f', 2, '0' ) );

    // connect sensor label xxxxxxx
    //connect(ui->leSen1Label,SIGNAL(textEdited(QString)),parentWidget(),SLOT(sensor1_textEdited(QString)));
    connect(this, SIGNAL(settingsChanged()), parentWidget(), SLOT(sensorSettingsChanged()));
}



SensorSettingsDialog::~SensorSettingsDialog()
{
    delete ui;
}



void SensorSettingsDialog::on_buttonBox_accepted()
{

    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());

    if( !ui->leSen1Label->text().trimmed().isEmpty() ){
        settings.setValue("Sensor1/label", ui->leSen1Label->text().trimmed());
        emit settingsChanged();
    }

    if( ui->leSen1Scale->text().trimmed().compare(".") ) {
        settings.setValue("Sensor1/scalar", ui->leSen1Scale->text().toDouble());
    }

    if( !ui->leSen2Label->text().trimmed().isEmpty() ){
        settings.setValue("Sensor2/label", ui->leSen2Label->text().trimmed());
        emit settingsChanged();
    }

    if( ui->leSen2Scale->text().trimmed().compare(".") ) {
        settings.setValue("Sensor2/scalar", ui->leSen2Scale->text().toDouble());
    }


}


