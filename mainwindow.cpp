#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QThread>
#include <QFileDialog>
#include <QtSerialPort/QSerialPortInfo>


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
    connect(ui->menuCommunication, SIGNAL(triggered(QAction*)), this, SLOT(ComPortSelected(QAction*)));

    connect(&grbl, SIGNAL(ToolChangeRequest()), this, SLOT(ToolChangeRequest()));
    connect(ui->leGFileName, SIGNAL(textChanged(QString)), this, SLOT(UpdateUIState()));

    if(QSerialPortInfo::availablePorts().isEmpty())
    {
        ui->menuCommunication->addAction("There is no available devices!")->setDisabled(true);
    }else{
        foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
        {
        QAction *a = ui->menuCommunication->addAction(info.portName()+" - "+info.description());
        a->setCheckable(true);
        a->setToolTip(info.portName());
        }
    }

    UpdateUIState();
    startTimer(100);
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
    if(m_eCurrentState == stSendingGFile)
    {
        if(!GFileSendChunk())
        {
            m_eCurrentState = stIdle;
            UpdateUIState();
        }
    }
    else if(m_eCurrentState == stCheckingGFile)
    {
        if(!GFileSendChunk())
        {
            m_eCurrentState = stIdle;
            UpdateUIState();
        }
    }
}

void MainWindow::CommandError(QString response)
{
    qDebug() << response;
}

void MainWindow::CommandSent(QString )
{

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
        nextidx = line.indexOf(",", idx);
        ui->lblWPX->setText(line.mid(idx, nextidx - idx));

        idx = nextidx+1;
        nextidx = line.indexOf(",", idx);
        ui->lblWPY->setText(line.mid(idx, nextidx - idx));

        idx = nextidx+1;
        nextidx = line.indexOf(",", idx);
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
        nextidx = line.indexOf(",", idx);
        ui->lblMPX->setText(line.mid(idx, nextidx - idx));

        idx = nextidx+1;
        nextidx = line.indexOf(",", idx);
        ui->lblMPY->setText(line.mid(idx, nextidx - idx));

        idx = nextidx+1;
        nextidx = line.indexOf(",", idx);
        ui->lblMPZ->setText(line.mid(idx, nextidx - idx));
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
    ui->btnHold->setChecked(true);
    ui->btnHold->setEnabled(false);
    ui->btnToolChangeAccept->setEnabled(true);
    grbl.SendAsyncCommand("!", false);


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
    else if(ui->btnHome->isDown()) cmd = "G0 Z0\nG0 X0 Y0";

    grbl.EnqueueCommand(cmd);
}

void MainWindow::timerEvent(QTimerEvent *)
{
    ui->progressBar->setValue(grbl.getBufferFill());
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

    grbl.setCapturingResponse(true);
    grbl.SendAsyncCommand("?", false);
    this->setDisabled(grbl.isResetInProgress());
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
    grbl.SendAsyncCommand("~", false);

}

void MainWindow::ComPortSelected(QAction* action)
{
    grbl.OpenPort(action->toolTip(), 115200);
}
