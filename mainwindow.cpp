#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "spinintaction.h"

#include <QFileDialog>
#include <QDebug>
#include <QMessageBox>
#include <QPointF>
#include <QVector>
#include <QSettings>
#include <QActionGroup>
#include <QWidgetAction>

#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_grid.h>
#include <qwt_symbol.h>
#include <qwt_legend.h>
#include <qwt_plot_marker.h>
#include <qwt_symbol.h>
#include <qwt_text.h>

#include <NIDAQmx.h>

#include <algorithm>


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{

    ui->setupUi(this);

    /* initialize app settings */
    appSettings = new QSettings(QCoreApplication::organizationName(), QCoreApplication::applicationName());


    /*
       add a subclassed spinbox to the menu
       as i don't know how to do that in designer
    */
    SpinIntAction * spinIntAction = new SpinIntAction("Duration(s):");
    spinIntAction->spinBox()->setValue(appSettings->value("CollectionDuration", 2).toInt());
    connect(spinIntAction->spinBox(), SIGNAL(valueChanged(int)), this, SLOT(collectionDuration_valueChanged(int)));
    ui->menuPlot_Settings->addAction(spinIntAction);




    /* make two menu items toggle within group */
    QActionGroup *plotSettingsNewPlotGroup = new QActionGroup(this);
    plotSettingsNewPlotGroup->addAction(ui->actionOverlays_Existing);
    plotSettingsNewPlotGroup->addAction(ui->actionReplaces_Existing);

    /* make two toolbutton items toggle within group */
    QActionGroup *toolbuttonGroup = new QActionGroup(this);
    toolbuttonGroup->addAction(ui->actionZoom);
    toolbuttonGroup->addAction(ui->actionPan);



    /* set which menu checked based on current settings value */
    if( appSettings->value("PlotClearDepth","NONE").toString().compare("NONE") ) {
        ui->actionOverlays_Existing->setChecked(true);
    } else {
        ui->actionReplaces_Existing->setChecked(true);
    }

    /* List of colors for line plots. Cycle through these as plots overlap. */
    plotColorList << Qt::darkBlue << Qt::darkRed << Qt::darkGreen << Qt::darkYellow << Qt::black << Qt::darkGray<< Qt::green << Qt::red << Qt::blue << Qt::yellow << Qt::black << Qt::gray<< Qt::green << Qt::red << Qt::blue << Qt::yellow << Qt::black << Qt::gray;

    initAppSettings();

    /* initial plot */
    setupPlot();

}


/**
 * @brief Sets plots to default status
 * maybe this should be called reset plot
 */
void MainWindow::setupPlot() {

    /* sensor plot 1 */
    ui->qwtPlot_1->setTitle(appSettings->value("Sensor1/label").toString());
    ui->qwtPlot_1->setCanvasBackground( Qt::white );
    ui->qwtPlot_1->setAxisScale( QwtPlot::yLeft, -20.0, 20.0);
    ui->qwtPlot_1->setAxisTitle( QwtPlot::yLeft, "Linear Acceleration (g)" );
    //ui->qwtPlot_1->setAxisTitle( QwtPlot::xBottom, "Time (what)" );

    QwtPlotGrid *grid1 = new QwtPlotGrid();
    grid1->attach( ui->qwtPlot_1 );


    /* sensor plot 1 */
    ui->qwtPlot_2->setTitle(appSettings->value("Sensor2/label").toString());
    ui->qwtPlot_2->setCanvasBackground( Qt::white );
    ui->qwtPlot_2->setAxisScale( QwtPlot::yLeft, -20.0, 20.0);
    ui->qwtPlot_2->setAxisTitle( QwtPlot::yLeft, "Linear Acceleration (g)" );
    ui->qwtPlot_2->setAxisTitle( QwtPlot::xBottom, "Time (s)" );


    QwtPlotGrid *grid2 = new QwtPlotGrid();
    grid2->attach( ui->qwtPlot_2 );


    ui->qwtPlot_1->replot();
    ui->qwtPlot_2->replot();

    /* change cursor to basic arrow */
    ui->qwtPlot_1->canvas()->setCursor(Qt::ArrowCursor);
    ui->qwtPlot_2->canvas()->setCursor(Qt::ArrowCursor);

}

/**
 * @brief Clear plots and reset to defaults
 */
void MainWindow::resetPlot()
{
    if(plotZoomer1) {
        /* reset zoom and pan stacks */
        plotZoomer1->zoom(0);
        plotZoomer2->zoom(0);
        plotPanner1->hide();
        plotPanner2->hide();
        /* Disable zoom and pan buttons */
        plotZoomer1->setEnabled(false);
        plotZoomer2->setEnabled(false);
        plotPanner1->setEnabled(false);
        plotPanner2->setEnabled(false);
        /* uncheck zoom and pan buttons */
        ui->actionZoom->setChecked(false);
        ui->actionPan->setChecked(false);
    }


    /* remove any objects currently plotted */
    ui->qwtPlot_1->detachItems();
    ui->qwtPlot_2->detachItems();
    // reset plots
    setupPlot();
}


/**
 * @brief Clear plots and reset to defaults
 */
void MainWindow::resetUI()
{
    resetPlot();
    //
    ui->mainToolBar->setEnabled(false);
}


void MainWindow::setCurrentExperiment(Experiment *experiment)
{
    this->currentExperiment = experiment;
}


/**
 * @brief Writes out some app settings Here, those we want to save when the app closes
 */
void MainWindow::writeAppSettings()
{
    appSettings->beginGroup("MainWindow");
    appSettings->setValue("size", size());
    appSettings->setValue("pos", pos());
    appSettings->endGroup();

}


/**
 * @brief application settings to read at launch
 */
void MainWindow::initAppSettings()
{
    appSettings->beginGroup("MainWindow");
    resize(appSettings->value("size", QSize(800, 600)).toSize());
    move(appSettings->value("pos", QPoint(200, 200)).toPoint());
    appSettings->endGroup();


    // these don't really need to be here but it initializes with default values
    appSettings->beginGroup("Sensor1");
    appSettings->value("label", "Sensor 1");
    appSettings->value("scalar", 0001.00);
    appSettings->endGroup();

    appSettings->beginGroup("Sensor2");
    appSettings->value("label", "Sensor 2");
    appSettings->value("scalar", 0001.00);
    appSettings->endGroup();

    // data directory will default to Windows public to support mulitple user logins
    appSettings->setValue("lastDirectory", "C:\\Users\\Public\\Documents\\MainStream_DropDevice_Exp_Data");

}

/**
 * @brief Write to disk
 */
void MainWindow::saveExperiment()
{
    // data directory will default to Windows public to support mulitple user logins
    QDir dir = QDir(appSettings->value("lastDirectory",".").toString());

    currentExperiment->save(dir);
}


/**
 * @brief Save to png
 */
void MainWindow::png()
{

    /*
    QImage image(600, 600, QImage::Format_ARGB32);
    pixmap.fill(Qt::white);

    QwtPlotPrintFilter filter;
    int options = QwtPlotPrintFilter::PrintAll;
    options &= ~QwtPlotPrintFilter::PrintBackground;
    options &= ~QwtPlotPrintFilter::PrintFrameWithScales;
    options &= ~QwtPlotPrintFilter::PrintMargin;
    options &= ~QwtPlotPrintFilter::PrintTitle;
    options &= ~QwtPlotPrintFilter::PrintLegend ;
    options &= ~QwtPlotPrintFilter::PrintGrid;
    filter.setOptions(options);

    qwt_plot->print(pixmap, image);
    */




    /* unverified
    QPixmap pix = QPixmap::grabWidget(myPlot);
    std::cout<<pix.save("der.png","PNG")<<std::endl;
    QPixmap pixmap (400,400);
    QPainter * painter=new QPainter(&pixmap);

    QwtPlotRenderer rend;

    rend.render(myPlot,painter,myPlot->geometry());
    pixmap.save("Dene.jpg");

    */

    /*
     * QImage img(512, 512, QImage::Format_ARGB32);
    QwtPlotPrintFilter pf;
    pf.setOptions(QwtPlotPrintFilter::PrintAll);
    bool use_painter = false;
    if (use_painter) {
    QPainter painter;
    painter.begin(&img);
    qpFACS->print(&painter,QRect(0, 0, 512, 512),pf);
    painter.end();
    } else {
    qpFACS->print(img,pf);
    }
    img.save(path);

    where qpFACS is a QwtPlot. Both methods work, producing the bare plot. I haven't found how to get the frame, axes, titles etc. If somebody can tell me what I need to do I'll be grateful.

    */

}




/**
 * @brief load experiment file and display experiment file
 */
void MainWindow::on_actionOpen_triggered() {

    // file open dialog to load experiment file
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open Data File"), appSettings->value("lastDirectory").toString(), tr("Data Files (*.drp)"));

    if(!fileName.isEmpty()) {

        // initialize new experiment with file data
        currentExperiment = Experiment::load(fileName);

        // disable data collection
        ui->actionStartCollection->setEnabled(false);

        /* get last used file path and store in user app settings */
        QFileInfo fileInfo(fileName);
        appSettings->setValue("lastDirectory",fileInfo.canonicalPath());

        resetPlot();
        plotExperiment(currentExperiment);

    }

}




/*
 * collect data from daq
 * daq sets DO high, collects data, sets DO low
*/
void MainWindow::on_actionStartCollection_triggered() {


    /*
    QMessageBox *mbox = new QMessageBox;
    mbox->exec();
    QTimer::singleShot(5000, mbox, SLOT(hide()));
    */


    // now that we have run experiment, disnable data collection
    ui->actionStartCollection->setEnabled(false);

    // initialize new experiment settings first
    currentExperiment->setChannel1Name(appSettings->value("Sensor1/label", "Sensor 1").toString());
    currentExperiment->setChannel1Scalar(appSettings->value("Sensor1/scalar").toDouble());
    currentExperiment->setChannel2Name(appSettings->value("Sensor2/label", "Sensor 2").toString());
    currentExperiment->setChannel2Scalar(appSettings->value("Sensor2/scalar").toDouble());

    /* task for digital out signal to drop device */
    TaskHandle  taskHandleDOtrigger=nullptr;
    char        triggerChannelDOStr[] = "Dev1/port1/line0:1";

    /* task for data collection */
    TaskHandle  taskHandleCollection=nullptr;
    /* specify device channels */
    char        deviceChannelStr[256] = "Dev1/ai6, Dev1/ai7";
    const int   numOfChannels = 2;
    /* channel voltage range */
    float64     min = -10, max = 10;

    /* device sampling rate */
    const int samplingRate   = 20000; // 10000
    /* largest data collection is number of channels x rate x 5seconds defined gui menu */
    const int totalArraySize = numOfChannels * samplingRate * 5;


    /*
     * samples per channel
     * Must be hardcoded because of c array below that reads data expecting a const
     */
    int samplesPerChannel = appSettings->value("CollectionDuration").toInt() * samplingRate;
    //const int samplesPerChannel = 20000; // 2 seconds of data
    const int totalSampleSize   = (samplesPerChannel * numOfChannels);
    /* update experiment header */
    currentExperiment->headerMap["Samples Per Channel"] = QString::number(samplesPerChannel);
    currentExperiment->headerMap["Sampling Rate(Hz)"] = QString::number(samplingRate);
    /* init data vectors */
    currentExperiment->voltageDataSensor1 = QVector<double>(samplesPerChannel);
    currentExperiment->voltageDataSensor2 = QVector<double>(samplesPerChannel);
    currentExperiment->timeVector = QVector<double>(samplesPerChannel);

    // DAQmxReturnValue
    int DAQmxReturnValue = 0;
    // error string
    char DAQmxErrorStr[1024];



    /* init DO drop device trigger task */
    DAQmxReturnValue = DAQmxCreateTask("triggerDO",&taskHandleDOtrigger);
    //
    /* init collection task */
    DAQmxReturnValue = DAQmxCreateTask("collection",&taskHandleCollection);


    /*
     * setup DO drop device trigger channel
     * While a pulse channel seemed more intuitive, it didn't work well
     * so a write to the DO channel is used instead
     */
    /*
    DAQmxReturnValue = DAQmxCreateCOPulseChanTime(taskHandleDOtrigger,triggerChannelDOStr,"",DAQmx_Val_Seconds,DAQmx_Val_Low,0.00,0.050,0.950);
    if (DAQmxReturnValue < 0) {
        DAQmxGetErrorString( DAQmxReturnValue, DAQmxErrorStr, 1024 );
        showMessage("DAQ Error", DAQmxErrorStr);
        return;
    }
    */
    DAQmxReturnValue = DAQmxCreateDOChan(taskHandleDOtrigger,triggerChannelDOStr,"",DAQmx_Val_ChanPerLine);
    if (DAQmxReturnValue < 0) {
        DAQmxGetErrorString( DAQmxReturnValue, DAQmxErrorStr, 1024 );
        showMessage("DAQ Error", DAQmxErrorStr);
        return;
    }



    /* setup collection voltage channels */
    DAQmxReturnValue = DAQmxCreateAIVoltageChan(taskHandleCollection, deviceChannelStr, "", DAQmx_Val_RSE, min, max, DAQmx_Val_Volts, nullptr );
    if (DAQmxReturnValue < 0) {
        DAQmxGetErrorString( DAQmxReturnValue, DAQmxErrorStr, 1024 );
        showMessage("DAQ Error", DAQmxErrorStr);
        return;
    }

    /* set task sampling rate */
    DAQmxCfgSampClkTiming(taskHandleCollection, "OnboardClock", static_cast<double>(samplingRate), DAQmx_Val_Rising, DAQmx_Val_FiniteSamps, static_cast<uint64_t>(samplesPerChannel) );
    /* collection task set to start on INPUT trigger */
    // disabled trigger: DAQmxCfgDigEdgeStartTrig(taskHandleCollection, triggerChannelStr, DAQmx_Val_Rising);


    /* begin collection task */
    DAQmxReturnValue = DAQmxStartTask(taskHandleCollection);
    if (DAQmxReturnValue < 0) {
        DAQmxGetErrorString( DAQmxReturnValue, DAQmxErrorStr, 1024 );
        showMessage("DAQ Error", DAQmxErrorStr);
    }

    /* read finite data samples */
    int numRead = 0;
    /* since the array size must be a constant
     * but i want option to varying data collection length,
     * set maximum array size for 2 channels at 10kHz.
     * collection window will be determined by variable "sampledData"
     * size passed into DAQmxReadAnalogF64
     */
    float64 sampledData[totalArraySize];


    /* begin DO trigger drop device task */
    DAQmxReturnValue = DAQmxStartTask(taskHandleDOtrigger);
    if (DAQmxReturnValue < 0) {
        DAQmxGetErrorString( DAQmxReturnValue, DAQmxErrorStr, 1024 );
        showMessage("DAQ Error", DAQmxErrorStr);
    }

    /* set DO trigger high to release drop */
    uInt32      data=1;
    int32		written;
    DAQmxReturnValue = DAQmxWriteDigitalU32(taskHandleDOtrigger,1,1,0,DAQmx_Val_GroupByChannel,&data,&written,nullptr);
    if (DAQmxReturnValue < 0) {
        DAQmxGetErrorString( DAQmxReturnValue, DAQmxErrorStr, 1024 );
        showMessage("DAQ Error", DAQmxErrorStr);
    }




    /* collect data sample */
    // disabled trigger: DAQmxReturnValue = DAQmxReadAnalogF64(taskHandleCollection, samplesPerChannel, DAQmx_Val_WaitInfinitely, DAQmx_Val_GroupByScanNumber, sampledData, static_cast<uint32_t>(totalSampleSize), &numRead, nullptr );
    DAQmxReturnValue = DAQmxReadAnalogF64(taskHandleCollection, samplesPerChannel, 10, DAQmx_Val_GroupByScanNumber, sampledData, static_cast<uint32_t>(totalSampleSize), &numRead, nullptr );
    if (DAQmxReturnValue < 0) {
        DAQmxGetErrorString( DAQmxReturnValue, DAQmxErrorStr, 1024 );
        showMessage("DAQ Error", DAQmxErrorStr);
    }

    /* set DO trigger low */
    data=0;
    DAQmxReturnValue = DAQmxWriteDigitalU32(taskHandleDOtrigger,1,1,0,DAQmx_Val_GroupByChannel,&data,&written,nullptr);
    if (DAQmxReturnValue < 0) {
        DAQmxGetErrorString( DAQmxReturnValue, DAQmxErrorStr, 1024 );
        showMessage("DAQ Error", DAQmxErrorStr);
    }

    /* stop/clear collection task */
    DAQmxReturnValue = DAQmxClearTask(taskHandleCollection);

    /* stop/clear trigger task */
    DAQmxReturnValue = DAQmxClearTask(taskHandleDOtrigger);






    /* populate data arrays with samples */
    double time = 0.0;
    int voltageIndex = 0;
    for(auto i = 0; i < totalSampleSize; i++) {
        if(i % 2) {
            // interleave second channel
            currentExperiment->voltageDataSensor2[voltageIndex] = sampledData[i];
            // after second channel update voltage data array index
            voltageIndex++;
        } else {
            // interleave first channel
            currentExperiment->voltageDataSensor1[voltageIndex] = sampledData[i];
            // time vector
            currentExperiment->timeVector[voltageIndex] = time;
            time+=1.0/samplingRate;
        }

    }


    /* */
    MainWindow::saveExperiment();
    MainWindow::plotExperiment(currentExperiment);

}



/**
 * @brief plot Experiment data
 */
void MainWindow::plotExperiment(Experiment *experiment)
{

    /* determine if we should clear or overlay this plot */
    if( appSettings->value("PlotClearDepth").toString().compare("ALL") ) {
        this->on_actionClearPlot_triggered();
     } else if(ui->qwtPlot_1->itemList().size() > 1){
        // a specific item
        ui->qwtPlot_1->detachItems(ui->qwtPlot_1->itemList().size(), true);
    }


    // current plot count for color selection
    QwtPlotItemList il = ui->qwtPlot_1->itemList();


    /* sensor 1 plot */
    QwtPlotCurve *plot1_curve1 = new QwtPlotCurve("Curve1");

    int peak_deceleration_index = std::min_element(experiment->voltageDataSensor1.begin(), experiment->voltageDataSensor1.end()) - experiment->voltageDataSensor1.begin();


    // display sensor scaled data
    //plot1_curve1->setRawSamples(experiment->timeVector.data(), experiment->getAccelerationData(1).data(), experiment->voltageDataSensor1.size() );
    // center on peak and display subvector once we understand window of interest
    int d_range_begin_pos = peak_deceleration_index-100;
    int d_range_length    = 1000;
    plot1_curve1->setSamples(experiment->timeVector.mid(d_range_begin_pos, d_range_length).data(), experiment->getAccelerationData(1).mid(d_range_begin_pos, d_range_length).data(), d_range_length );
    // show voltage
    //plot1_curve1->setRawSamples(experiment->timeVector.data(), experiment->voltageDataSensor1.data(), experiment->voltageDataSensor1.size() );


    plot1_curve1->setTitle( experiment->headerMap["SubjectID"]);
    plot1_curve1->setPen( plotColorList[il.size()-1], 1 );
    plot1_curve1->setRenderHint( QwtPlotItem::RenderAntialiased, true );
    plot1_curve1->attach(ui->qwtPlot_1);

    ui->qwtPlot_1->insertLegend( new QwtLegend() );
    // make sure axis updates for data range
    ui->qwtPlot_1->setAxisAutoScale(QwtPlot::yLeft, true);
    ui->qwtPlot_1->replot();


    //ui->qwtPlot_1->setAxisMaxMinor(QwtPlot::xBottom, (20000/1000));
    //qDebug() << experiment->voltageDataSensor1.size();
    //ui->qwtPlot_1->setAxisAutoScale(QwtPlot::xBottom, true);



    //int peak_acceleration_index = std::max_element(experiment->voltageDataSensor1.begin(), experiment->voltageDataSensor1.end()) - experiment->voltageDataSensor1.begin();
    //double peak_acceleration = experiment->getAccelerationData(1)[peak_acceleration_index];

    /* add peak decel label to plot */
    //int peak_deceleration_index = std::min_element(experiment->voltageDataSensor1.begin(), experiment->voltageDataSensor1.end()) - experiment->voltageDataSensor1.begin();
    double peak_deceleration = experiment->getAccelerationData(1)[peak_deceleration_index];
    QwtPlotMarker* m = new QwtPlotMarker();
    m->setSymbol(new QwtSymbol( QwtSymbol::Diamond, Qt::red, Qt::NoPen, QSize( 5, 5 ) ));
    m->setValue( QPointF( experiment->timeVector[peak_deceleration_index], peak_deceleration ) );
    m->setLabel( QString::number(peak_deceleration) );
    m->setLabelAlignment(Qt::AlignLeft);
    m->attach(ui->qwtPlot_1);





    /* sensor 2 plot */
    QwtPlotCurve *plot1_curve2 = new QwtPlotCurve("Curve2");

    // display all data
    plot1_curve2->setRawSamples(experiment->timeVector.data(), experiment->getAccelerationData(2).data(), experiment->voltageDataSensor2.size() );
    // show voltage
    //plot1_curve2->setRawSamples(experiment->timeVector.data(), experiment->voltageDataSensor2.data(), experiment->voltageDataSensor2.size() );
    // center on peak and display subvector
    //plot1_curve2->setSamples(e->timeVector.mid(maxElementIndex-2000, 4000), e->voltageDataSensor2.mid(maxElementIndex-2000, 4000));

    plot1_curve2->setTitle( experiment->headerMap["SubjectID"]);
    plot1_curve2->setPen( plotColorList[il.size()-1], 1 );
    plot1_curve2->setRenderHint( QwtPlotItem::RenderAntialiased, true );
    plot1_curve2->attach(ui->qwtPlot_2);

    ui->qwtPlot_2->insertLegend( new QwtLegend() );
    // make sure axis updates for data range
    ui->qwtPlot_2->setAxisAutoScale(QwtPlot::yLeft, true);
    ui->qwtPlot_2->replot();


    /* provide zooming feature on plot using mouse */
    plotZoomer1 = new QwtPlotZoomer( ui->qwtPlot_1->canvas() );
    plotZoomer1->setRubberBandPen(QPen(Qt::red));
    plotZoomer1->setEnabled(false);
    plotZoomer2 = new QwtPlotZoomer( QwtPlot::xBottom, QwtPlot::yLeft, ui->qwtPlot_2->canvas() );
    plotZoomer2->setRubberBandPen(QPen(Qt::red));
    plotZoomer2->setEnabled(false);

    /* provide panning feature on plots */
    plotPanner1 = new QwtPlotPanner( ui->qwtPlot_1->canvas() );
    plotPanner1->setEnabled(false);
    plotPanner2 = new QwtPlotPanner( ui->qwtPlot_2->canvas() );
    plotPanner2->setEnabled(false);

    ui->mainToolBar->setEnabled(true);
}



/**
 * @brief Clear plots and reset to defaults
 */
void MainWindow::on_actionClearPlot_triggered()
{
    resetUI();
}



/**
 * @brief Open Sensor Settings form
 */
void MainWindow::on_actionSensor_Settings_triggered()
{
    dlgSensorSettings = new SensorSettingsDialog(this);
    dlgSensorSettings->show();
}


/**
 * @brief Change how multiple plots are overlayed
 */
void MainWindow::on_menuNew_Plot_triggered()
{
    if( ui->actionOverlays_Existing->isChecked() ) {
        appSettings->setValue("PlotClearDepth", "ALL");
    } else {
        appSettings->setValue("PlotClearDepth", "NONE");
    }
}

/**
 * @brief provide zooming feature on plot using mouse
 */
void MainWindow::on_actionZoom_triggered()
{
    if(plotZoomer1) {
        plotZoomer1->setEnabled(true);
        plotZoomer2->setEnabled(true);
        plotPanner1->setEnabled(false);
        plotPanner2->setEnabled(false);

        ui->qwtPlot_1->canvas()->setCursor(Qt::CrossCursor);
        ui->qwtPlot_2->canvas()->setCursor(Qt::CrossCursor);
    }
}


void MainWindow::on_actionPan_triggered()
{
    //if(plotZoomer1) {
        plotZoomer1->setEnabled(false);
        plotZoomer2->setEnabled(false);
        plotPanner1->setEnabled(true);
        plotPanner2->setEnabled(true);

        ui->qwtPlot_1->canvas()->setCursor(Qt::SizeAllCursor);
        ui->qwtPlot_2->canvas()->setCursor(Qt::SizeAllCursor);
    //}
}


void MainWindow::on_actionHome_triggered()
{
    if(plotZoomer1) {
        plotZoomer1->zoom(0);
        plotZoomer2->zoom(0);
        plotPanner1->hide();
        plotPanner2->hide();
    }
}




/**
 * @brief generic message box for now
 * @param str
 */
void MainWindow::showMessage(const QString txtShort, const QString txtDetailed) {
    QMessageBox msgBox;
    msgBox.setWindowTitle("Drop Data Collection");
    msgBox.setText(txtShort);
    msgBox.setInformativeText(txtDetailed);
    msgBox.setWindowModality(Qt::WindowModality::ApplicationModal);
    msgBox.exec();
}



/**
 * @brief When window is closing
 * @param event
 */
void MainWindow::closeEvent(QCloseEvent *event) {
    writeAppSettings();
}


/**
 * @brief Setup new experiment
 */
void MainWindow::on_actionNew_Experiment_triggered()
{
    /*
    QMessageBox *mbox = new QMessageBox;
    mbox->setStandardButtons(QMessageBox::Abort);

    connect(mbox, SIGNAL(rejected()), this, SLOT(rejected_triggered()));
    mbox->show();
    QTimer::singleShot(5000, mbox, SLOT(launch_new_experiment()));
    */

    dlgNewExperiment = new NewExperimentDialog(this);
    dlgNewExperiment->show();
}



MainWindow::~MainWindow()
{
    delete ui;
}



/**
 * @brief a listener slot for when sensor settings have been changed
 */
void MainWindow::sensorSettingsChanged()
{
    // update plot titles
    ui->qwtPlot_1->setTitle(appSettings->value("Sensor1/label").toString());
    ui->qwtPlot_2->setTitle(appSettings->value("Sensor2/label").toString());
}

void MainWindow::start_newExperiment(Experiment *experiment)
{
    // reset plots
    resetPlot();
    // make experiment current
    this->setCurrentExperiment(experiment);
    // now that we have init experiment, enable data collection
    ui->actionStartCollection->setEnabled(true);
    ui->mainToolBar->setEnabled(true);
}


/**
 * @brief Update settings with new data collection duration window
 * @param newValue
 */
void MainWindow::collectionDuration_valueChanged(int newValue)
{
    appSettings->setValue("CollectionDuration", newValue);
}

void MainWindow::rejected_triggered()
{
    qDebug() << "rejected";
}

void MainWindow::launch_new_experiment()
{
    dlgNewExperiment = new NewExperimentDialog(this);
    dlgNewExperiment->show();
}

