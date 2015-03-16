#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSerialPort>
#include <QFile>

#include "CGRBLController.h"
#include "SettingsWindow.h"

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
    void ToolChangeRequest();
    void JoggingBtnPressed();
    void on_btnHold_pressed();
    void on_btnReset_pressed();
    void on_btnOpenGFile_clicked();
    void on_btnToolChangeAccept_clicked();
    void on_btnZeroXY_clicked();
    void on_btnZeroZ_clicked();
    void on_actionGRBL_Settings_triggered();

    void UpdateUIState(void);
    void on_btnStartGFile_clicked();
    void on_btnUnlock_clicked();
    void on_btnCheckGFile_clicked();
    void opengrblSettings();


private:
    Ui::MainWindow *ui;
    SettingsWindow *grblSet;
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


    /* -------------- Comm Port Selection-------------- */
private:
    QString m_CurrentCommPort;
private slots:
    void CommPortSelected(QAction *action);
    void EnumerateCommPorts();
    /* ------------------------------------------------ */

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
