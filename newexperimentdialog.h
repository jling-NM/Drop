#ifndef NEWEXPERIMENTDIALOG_H
#define NEWEXPERIMENTDIALOG_H

#include <QDialog>
#include "experiment.h"



namespace Ui {
class NewExperimentDialog;
}

class NewExperimentDialog : public QDialog
{
    Q_OBJECT

public:
    explicit NewExperimentDialog(QWidget *parent = nullptr);
    ~NewExperimentDialog();

signals:
    void start_newExperiment(Experiment *);

private slots:

    void on_buttonBox_accepted();

private:
    Ui::NewExperimentDialog *ui;
};

#endif // NEWEXPERIMENTDIALOG_H
