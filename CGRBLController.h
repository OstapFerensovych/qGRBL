#ifndef CGRBLCONTROLLER_H
#define CGRBLCONTROLLER_H

#include <QObject>
#include <QSerialPort>
#include <QStringList>

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
    QStringList getCapturedResponse() { return m_CmdResponse; }
    quint32 getBufferFill() { return m_BufFill; }
    bool isResetInProgress() { return m_ResetInProgress; }
    void setFeedRateMultiplier(double mult);
    double getLastFeedRate();

private:
    QSerialPort           m_port;
    QSerialPort::BaudRate m_baud;
    QList<QByteArray>     m_CmdQueue;
    quint32               m_BufFill;
    QStringList           m_CmdResponse;
    QStringList           m_GrblParams;
    bool                  m_ResetInProgress;
    bool                  m_CapturingResponse;
    bool                  m_RetrievingParams;
    bool                  m_NeedFeedRateUpdate;
    double                m_LastFeedRate;
    double                m_FeedRateMultiplier;

    QString UpdateFeedRateMultiplier(QString cmd, double factor);

private slots:
    void serialReadyRead();

signals:
    void QueuedCommandDone();
    void CommandError(QString response);
    void ResponseLineReceieved(QString line);
    void CommandSent(QString cmd);
    void ParamsRetreieved(QStringList params);
    void ToolChangeRequest();
    void SetZProbe();

public slots:
};

#endif // CGRBLCONTROLLER_H
