#ifndef SENSORSETTINGSDIALOG_H
#define SENSORSETTINGSDIALOG_H

#include <QDialog>

namespace Ui {
class SensorSettingsDialog;
}

class SensorSettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SensorSettingsDialog(QWidget *parent = nullptr);
    ~SensorSettingsDialog();

signals:
    void settingsChanged();

private slots:

    void on_buttonBox_accepted();

private:
    Ui::SensorSettingsDialog *ui;
};

#endif // SENSORSETTINGSDIALOG_H
