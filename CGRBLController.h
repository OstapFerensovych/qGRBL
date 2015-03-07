#ifndef CGRBLCONTROLLER_H
#define CGRBLCONTROLLER_H

#include <QObject>
#include <QSerialPort>

#define GRBL_BUFFER_SIZE      (120)

class CGRBLController : public QObject
{
    Q_OBJECT
public:
    explicit CGRBLController(QObject *parent = 0);
    ~CGRBLController();

public:
    bool OpenPort(QString port, quint32 baud);
    void ClosePort();

    void SendReset();
    void RetrieveParams();

    bool SendAsyncCommand(QString cmd, bool appendCR = true);
    bool EnqueueCommand(QString cmd);

    void setCapturingResponse(bool on);
    QList<QString> getCapturedResponse() { return m_CmdResponse; }
    quint32 getBufferFill() { return m_BufFill; }
    bool isResetInProgress() { return m_ResetInProgress; }

private:
    QSerialPort           m_port;
    QSerialPort::BaudRate m_baud;
    QList<QByteArray>     m_CmdQueue;
    quint32               m_BufFill;
    QList<QString>        m_CmdResponse;
    bool                  m_ResetInProgress;
    bool                  m_CapturingResponse;

private slots:
    void serialReadyRead();

signals:
    void QueuedCommandDone();
    void CommandError(QString response);
    void ResponseLineReceieved(QString line);
    void CommandSent(QString cmd);

public slots:
};

#endif // CGRBLCONTROLLER_H
