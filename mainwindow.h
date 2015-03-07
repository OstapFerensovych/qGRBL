#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSerialPort>
#include <QFile>

#include "CGRBLController.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void QueuedCommandDone();
    void CommandError(QString response);
    void CommandSent(QString cmd);
    void ResponseLineReceieved(QString line);
    void JoggingBtnPressed();
    void on_btnHold_pressed();
    void on_btnReset_pressed();
    void on_btnOpenGFile_clicked();

    void UpdateUIState(void);
    void on_btnStartGFile_clicked();
    void on_btnUnlock_clicked();
    void on_btnCheckGFile_clicked();


private:
    Ui::MainWindow *ui;
    QFile gfile;
    CGRBLController grbl;

    void timerEvent(QTimerEvent *);

    bool GFileSendChunk();

    enum
    {
        stIdle,
        stCheckingGFile,
        stSendingGFile
    } m_eCurrentState;


/* ---------------- Manual Control ---------------- */
private:
    int m_MCPointer;
    QStringList m_MCHistory;
    bool eventFilter(QObject* obj, QEvent *event);
    void _ManualControlCreate();
    void _ManualControlDelete();
private slots:
    void on_lineEdit_returnPressed();
/* ------------------------------------------------ */

};

#endif // MAINWINDOW_H
