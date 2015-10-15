#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "SettingsWindow.h"
#include "ui_SettingsWindow.h"

#include <QDebug>
#include <QFileDialog>
#include <QSerialPortInfo>

#define SIMULATOR_DEBUG 0

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    m_eCurrentState = stIdle;

    _ManualControlCreate();

    connect(&grbl, SIGNAL(QueuedCommandDone()), this, SLOT(QueuedCommandDone()));
    connect(&grbl, SIGNAL(CommandError(QString)), this, SLOT(CommandError(QString)));
    connect(&grbl, SIGNAL(CommandSent(QString)), this, SLOT(CommandSent(QString)));
    connect(&grbl, SIGNAL(ResponseLineReceieved(QString)), this, SLOT(ResponseLineReceieved(QString)));

    connect(ui->btnXm, SIGNAL(pressed()), this, SLOT(JoggingBtnPressed()));
    connect(ui->btnXp, SIGNAL(pressed()), this, SLOT(JoggingBtnPressed()));
    connect(ui->btnYm, SIGNAL(pressed()), this, SLOT(JoggingBtnPressed()));
    connect(ui->btnYp, SIGNAL(pressed()), this, SLOT(JoggingBtnPressed()));
    connect(ui->btnZm, SIGNAL(pressed()), this, SLOT(JoggingBtnPressed()));
    connect(ui->btnZp, SIGNAL(pressed()), this, SLOT(JoggingBtnPressed()));
    connect(ui->btnHome, SIGNAL(pressed()), this, SLOT(JoggingBtnPressed()));
    connect(ui->menuCommunication, SIGNAL(triggered(QAction*)), this, SLOT(CommPortSelected(QAction*)));
    connect(ui->menuCommunication, SIGNAL(aboutToShow()), this, SLOT(EnumerateCommPorts()));

    connect(&grbl, SIGNAL(ToolChangeRequest()), this, SLOT(ToolChangeRequest()));
    connect(&grbl, SIGNAL(SetZProbe()), this, SLOT(SetZProbe()));
    connect(ui->leGFileName, SIGNAL(textChanged(QString)), this, SLOT(UpdateUIState()));

    UpdateUIState();
    startTimer(100);

#if(SIMULATOR_DEBUG)
    m_CurrentCommPort = "/dev/ttyFAKE";
    grbl.OpenPort(m_CurrentCommPort, 115200);
#endif

}

MainWindow::~MainWindow()
{
    _ManualControlDelete();

    gfile.close();
    grbl.ClosePort();
    delete ui;
}

void MainWindow::QueuedCommandDone()
{
    switch(m_eCurrentState)
    {
    case stCheckingGFile: /* Just falling through to next state */
    case stSendingGFile:
        if(!GFileSendChunk())
        {
            m_eCurrentState = stIdle;
            UpdateUIState();
        }
        break;

    default:
        break;
    }
}

void MainWindow::CommandError(QString response)
{
    qDebug() << response;
}

void MainWindow::CommandSent(QString cmd)
{
    ui->listWidget->clearSelection();
    ui->listWidget->addItem(cmd);
    while(ui->listWidget->count() > 100) ui->listWidget->takeItem(0);
}


void MainWindow::ResponseLineReceieved(QString line)
{
    int idx, nextidx;

    idx = line.indexOf(",");
    ui->gbStatus->setTitle("Status: " + line.mid(1, idx - 1));
    if(line.mid(1, idx - 1)=="Alarm")
    {
        ui->btnUnlock->setStyleSheet("background-color: rgb(255, 0, 0); color: rgb(255, 247, 0);");
    }
    else
    {
        ui->btnUnlock->setStyleSheet("");
    }

    idx = line.indexOf("WPos:");
    if(idx > 0)
    {
        idx += 5;
        nextidx = line.indexOf(QRegularExpression("[,>]"), idx);
        ui->lblWPX->setText(line.mid(idx, nextidx - idx));

        idx = nextidx+1;
        nextidx = line.indexOf(QRegularExpression("[,>]"), idx);
        ui->lblWPY->setText(line.mid(idx, nextidx - idx));

        idx = nextidx+1;
        nextidx = line.indexOf(QRegularExpression("[,>]"), idx);
        ui->lblWPZ->setText(line.mid(idx, nextidx - idx));
    }
    else
    {
        ui->lblWPX->setText("------");
        ui->lblWPY->setText("------");
        ui->lblWPZ->setText("------");
    }

    idx = line.indexOf("MPos:");
    if(idx > 0)
    {
        idx += 5;
        nextidx = line.indexOf(QRegularExpression("[,>]"), idx);
        ui->lblMPX->setText(line.mid(idx, nextidx - idx));
        m_Xpos = ui->lblMPX->text().toDouble();

        idx = nextidx+1;
        nextidx = line.indexOf(QRegularExpression("[,>]"), idx);
        ui->lblMPY->setText(line.mid(idx, nextidx - idx));
        m_Ypos = ui->lblMPY->text().toDouble();

        idx = nextidx+1;
        nextidx = line.indexOf(QRegularExpression("[,>]"), idx);
        ui->lblMPZ->setText(line.mid(idx, nextidx - idx));
        m_Zpos = ui->lblMPZ->text().toDouble();
    }
    else
    {
        ui->lblMPX->setText("------");
        ui->lblMPY->setText("------");
        ui->lblMPZ->setText("------");
    }
    grbl.setCapturingResponse(false);
}

void MainWindow::ToolChangeRequest()
{
    ui->labelToolNumber->setText("TOOL");
    ui->btnHold->setChecked(true);
    ui->btnHold->setEnabled(false);
    ui->btnProbe->setEnabled(true);
    ui->btnToolChangeAccept->setEnabled(true);
    grbl.EnqueueCommand("M0");
    //grbl.EnqueueCommand("G30.1");
    //grbl.EnqueueCommand("G91 Z0");
    //grbl.EnqueueCommand("G91 X0 Y0");
}

void MainWindow::JoggingBtnPressed()
{
    QString dist;
    QString feed = ui->leFeedRate->text();

    if(ui->rb1->isChecked()) dist = "0.1";
    else if(ui->rb2->isChecked()) dist = "1.0";
    else if(ui->rb3->isChecked()) dist = "10.0";
    else if(ui->rb4->isChecked()) dist = "100.0";

    QString cmd;

    if(ui->btnXm->isDown())      cmd = "G91\nG1 X-" + dist + " F" + feed + "\nG90";
    else if(ui->btnXp->isDown()) cmd = "G91\nG1 X"  + dist + " F" + feed + "\nG90";
    else if(ui->btnYm->isDown()) cmd = "G91\nG1 Y-" + dist + " F" + feed + "\nG90";
    else if(ui->btnYp->isDown()) cmd = "G91\nG1 Y"  + dist + " F" + feed + "\nG90";
    else if(ui->btnZm->isDown()) cmd = "G91\nG1 Z-" + dist + " F" + feed + "\nG90";
    else if(ui->btnZp->isDown()) cmd = "G91\nG1 Z"  + dist + " F" + feed + "\nG90";
    else if(ui->btnHome->isDown()) cmd = "G90 G0 Z0\nG0 X0 Y0";

    grbl.EnqueueCommand(cmd);
}

void MainWindow::timerEvent(QTimerEvent *)
{
    ui->progressBar->setValue(grbl.getBufferFill());
    ui->listWidget->scrollToBottom();

    if(gfile.isOpen())
    {
        ui->progressBar_2->setMaximum(gfile.size());
        ui->progressBar_2->setValue(gfile.pos());
    }
    else
    {
        ui->progressBar_2->setMaximum(100);
        ui->progressBar_2->setValue(0);
    }

    bool reset = grbl.isResetInProgress();
    this->setDisabled(reset);
    if(!reset)
    {
        grbl.setCapturingResponse(true);
        grbl.SendAsyncCommand("?", false);
    }

    ui->lblLastFeedRate->setText(QString::number(grbl.getLastFeedRate(), 'f', 3));
}

bool MainWindow::GFileSendChunk()
{
    QString cmd;
    quint64 lastFilePos;

    do
    {
        if(gfile.atEnd())
        {
            gfile.close();
            return false;
        }
        lastFilePos = gfile.pos();
        cmd = gfile.readLine();
    } while(grbl.EnqueueCommand(cmd));

    gfile.seek(lastFilePos);

    return true;
}

void MainWindow::UpdateUIState()
{
    ui->btnStartGFile->setEnabled(!ui->leGFileName->text().isEmpty() && QFile::exists(ui->leGFileName->text()) && (m_eCurrentState != stSendingGFile));
    ui->btnCheckGFile->setEnabled(ui->btnStartGFile->isEnabled());

    if(m_eCurrentState == stIdle)
    {
        ui->btnXm->setEnabled(true);
        ui->btnXp->setEnabled(true);
        ui->btnYm->setEnabled(true);
        ui->btnYp->setEnabled(true);
        ui->btnZm->setEnabled(true);
        ui->btnZp->setEnabled(true);
        ui->btnHome->setEnabled(true);
        ui->btnUnlock->setEnabled(true);
        ui->btnProbe->setEnabled(true);
        ui->btnZeroXY->setEnabled(true);
        ui->btnZeroZ->setEnabled(true);
        ui->btnSpnON->setEnabled(true);
        ui->btnSpnOFF->setEnabled(true);
    }
    else if(m_eCurrentState == stSendingGFile || m_eCurrentState == stCheckingGFile)
    {
        ui->btnXm->setEnabled(false);
        ui->btnXp->setEnabled(false);
        ui->btnYm->setEnabled(false);
        ui->btnYp->setEnabled(false);
        ui->btnZm->setEnabled(false);
        ui->btnZp->setEnabled(false);
        ui->btnHome->setEnabled(false);
        ui->btnUnlock->setEnabled(false);
        ui->btnProbe->setEnabled(false);
        ui->btnZeroXY->setEnabled(false);
        ui->btnZeroZ->setEnabled(false);
        ui->btnSpnON->setEnabled(false);
        ui->btnSpnOFF->setEnabled(false);
    }


    this->setDisabled(grbl.isResetInProgress());
}

void MainWindow::on_btnHold_pressed()
{
    if(ui->btnHold->isChecked()) grbl.SendAsyncCommand("~", false);
    else grbl.SendAsyncCommand("!", false);
}


void MainWindow::on_btnReset_pressed()
{
    if(m_eCurrentState == stSendingGFile || m_eCurrentState == stCheckingGFile)
    {
        gfile.close();
        m_eCurrentState = stIdle;
        UpdateUIState();
    }
    grbl.SendReset();
    ui->btnToolChangeAccept->setEnabled(false);
}

void MainWindow::on_btnOpenGFile_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Open File","","GCode Files(*.cnc *.nc *.gcode *.tap)");
    ui->leGFileName->setText(fileName);
    gfile.setFileName(fileName);
}

void MainWindow::on_btnStartGFile_clicked()
{
    if(gfile.isOpen()) gfile.close();
    gfile.setFileName(ui->leGFileName->text());
    if(!gfile.open(QFile::ReadOnly)) return;
    m_eCurrentState = stSendingGFile;
    UpdateUIState();
    GFileSendChunk();
}

void MainWindow::on_btnStopGFile_clicked()
{
    if(m_eCurrentState == stSendingGFile || m_eCurrentState == stCheckingGFile)
    {
        gfile.close();
        m_eCurrentState = stIdle;
        UpdateUIState();
    }
    ui->btnToolChangeAccept->setEnabled(false);
}

void MainWindow::on_btnUnlock_clicked()
{
    grbl.SendAsyncCommand("$X");
}

void MainWindow::on_btnCheckGFile_clicked()
{
    if(gfile.isOpen()) gfile.close();
    gfile.setFileName(ui->leGFileName->text());
    if(!gfile.open(QFile::ReadOnly)) return;
    m_eCurrentState = stCheckingGFile;
    UpdateUIState();
    grbl.SendAsyncCommand("$C");
    GFileSendChunk();
}

void MainWindow::on_btnToolChangeAccept_clicked()
{
    ui->btnToolChangeAccept->setEnabled(false);
    ui->btnProbe->setEnabled(false);
    grbl.SendAsyncCommand("~", false);
    //grbl.SendAsyncCommand("G30",true);
}

void MainWindow::CommPortSelected(QAction* action)
{
    grbl.OpenPort(action->toolTip(), 115200);
    m_CurrentCommPort = action->toolTip();
}

void MainWindow::EnumerateCommPorts()
{
    ui->menuCommunication->clear();

    if(QSerialPortInfo::availablePorts().isEmpty())
    {
        ui->menuCommunication->addAction("There is no available devices!")->setDisabled(true);
    }
    else
    {
        foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
        {
            QAction *a = ui->menuCommunication->addAction(info.portName() + (info.description().isEmpty() ? "" : " - " + info.description()));
            a->setCheckable(true);
            a->setToolTip(info.portName());
            if(info.portName() == m_CurrentCommPort) a->setChecked(true);
        }
    }
}

void MainWindow::on_btnZeroXY_clicked()
{
    int currSyst = QString::number(ui->coordSyst->currentIndex()).toInt() + 1;
    grbl.SendAsyncCommand("G10 L2 P" + QString::number(currSyst) + " X" + QString::number(m_Xpos) + " Y" + QString::number(m_Ypos));
}

void MainWindow::on_btnZeroZ_clicked()
{
    int currSyst = QString::number(ui->coordSyst->currentIndex()).toInt() + 1;
    grbl.SendAsyncCommand("G10 L2 P" + QString::number(currSyst) + " Z" + QString::number(m_Zpos));
}

void MainWindow::opengrblSettings()
{
    grblSet = new SettingsWindow(&grbl, this);
    grblSet->show();
}

void MainWindow::on_actionGRBL_Settings_triggered()
{
    opengrblSettings();
}

void MainWindow::on_setSpnRpm_clicked()
{
    grbl.SendAsyncCommand("S" + ui->rpmBox->text());
}

void MainWindow::on_btnSpnON_clicked()
{
    grbl.SendAsyncCommand("M3");
}

void MainWindow::on_btnSpnOFF_clicked()
{
    grbl.SendAsyncCommand("M5");
}

void MainWindow::on_rpmBox_returnPressed()
{
    on_setSpnRpm_clicked();
}

void MainWindow::on_btnProbe_clicked()
{
    grbl.SendAsyncCommand("G91 G38.2 Z-10 F10");
    grbl.SendAsyncCommand("G90");
}

void MainWindow::SetZProbe()
{
    grbl.SendAsyncCommand("G92 Z" + ui->lineZProbe->text());
}

void MainWindow::on_dsbFeedOverride_valueChanged(double arg1)
{
    grbl.setFeedRateMultiplier(arg1);
}

void MainWindow::on_coordSyst_currentIndexChanged(int index)
{
    int currSyst = index + 54;
    grbl.SendAsyncCommand("G" + QString::number(currSyst));
}
