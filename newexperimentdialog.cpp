#include "newexperimentdialog.h"
#include "ui_newexperimentdialog.h"
#include <QSettings>
#include "experiment.h"
#include "mainwindow.h"



NewExperimentDialog::NewExperimentDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::NewExperimentDialog)
{
    ui->setupUi(this);

    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
    ui->leSubjectId->setText(settings.value("SubjectId","").toString());
    ui->spinBoxWeight->setValue(settings.value("Weight", 450).toInt());
    ui->doubleSpinBoxHeight->setValue(settings.value("Height").toDouble());

    // connect form to controller
    connect(this,SIGNAL(start_newExperiment(Experiment *)),parentWidget(),SLOT(start_newExperiment(Experiment *)));
}


NewExperimentDialog::~NewExperimentDialog()
{
    delete ui;
}

void NewExperimentDialog::on_buttonBox_accepted()
{
    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());

    if( !ui->leSubjectId->text().trimmed().isEmpty() ){
        settings.setValue("SubjectId", ui->leSubjectId->text().trimmed());
        settings.setValue("Weight", ui->spinBoxWeight->value());
        settings.setValue("Height", ui->doubleSpinBoxHeight->value());

        Experiment *experiment = new Experiment();
        experiment->setWeight(ui->spinBoxWeight->value());
        experiment->setHeight(ui->doubleSpinBoxHeight->value());
        experiment->setSubjectId(ui->leSubjectId->text().trimmed());

        emit start_newExperiment(experiment);
    }
}

