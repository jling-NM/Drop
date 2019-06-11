#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSettings>
#include "experiment.h"
#include "sensorsettingsdialog.h"
#include "newexperimentdialog.h"

#include <qwt_plot_zoomer.h>
#include <qwt_plot_panner.h>


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT


public:

    explicit MainWindow(QWidget *parent = nullptr);

    ~MainWindow() override;

    void showMessage(const QString txtShort, const QString txtDetailed);



protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void on_actionNew_Experiment_triggered();
    void on_actionOpen_triggered();
    void on_actionStartCollection_triggered();
    void on_actionClearPlot_triggered();
    void on_actionSensor_Settings_triggered();
    void on_menuNew_Plot_triggered();
    void on_actionZoom_triggered();
    void on_actionPan_triggered();
    void on_actionHome_triggered();
    void sensorSettingsChanged();
    void start_newExperiment(Experiment *experiment);
    void collectionDuration_valueChanged(int newValue);


private:
    Ui::MainWindow *ui;
    SensorSettingsDialog *dlgSensorSettings;
    NewExperimentDialog *dlgNewExperiment;

    Experiment *currentExperiment = nullptr;

    void setupPlot();
    void resetPlot();
    void resetUI();

    void setCurrentExperiment(Experiment *experiment = nullptr);

    void writeAppSettings();
    void readAppSettings();

    void saveExperiment();
    void plotExperiment(Experiment *experiment = nullptr);

    void png();


    QList<QColor> plotColorList;

    QwtPlotZoomer *plotZoomer1 = nullptr;
    QwtPlotZoomer *plotZoomer2 = nullptr;
    QwtPlotPanner *plotPanner1 = nullptr;
    QwtPlotPanner *plotPanner2 = nullptr;

    QSettings *appSettings = nullptr;

};

#endif // MAINWINDOW_H
