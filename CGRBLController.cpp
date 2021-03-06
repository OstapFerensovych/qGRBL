#include "CGRBLController.h"
#include <QDebug>
#include <QStringList>
#include <QThread>

CGRBLController::CGRBLController(QObject *parent) : QObject(parent)
{
    m_CmdQueue.clear();
    m_BufFill = 0;
    m_ResetInProgress = false;
    m_CapturingResponse = false;
    m_RetrievingParams = false;
    m_NeedFeedRateUpdate = false;
    m_LastFeedRate = 0;
    m_FeedRateMultiplier = 1.0;
}

CGRBLController::~CGRBLController()
{
    ClosePort();
}

bool CGRBLController::OpenPort(QString port, quint32 baud)
{
    if(m_port.isOpen()) ClosePort();

    m_port.setPortName(port);
    m_port.setBaudRate((QSerialPort::BaudRate)baud);
    connect(&m_port, SIGNAL(readyRead()), this, SLOT(serialReadyRead()));
    bool ok = m_port.open(QSerialPort::ReadWrite);

    if(ok) SendReset();

    return ok;
}

void CGRBLController::ClosePort()
{
    if(m_port.isOpen()) m_port.close();
    disconnect(&m_port, SIGNAL(readyRead()), this, SLOT(serialReadyRead()));
}

void CGRBLController::SendReset()
{
    m_port.write("\r\n\r\n\030");
    m_port.flush();
    m_CmdQueue.clear();
    m_BufFill = 0;
    m_LastFeedRate = 0;
    m_ResetInProgress = true;
}

void CGRBLController::RetrieveParams()
{
    if(!m_port.isOpen() || m_ResetInProgress) return;

    m_RetrievingParams = true;
    m_GrblParams.clear();
    m_port.write("$$\n");
    m_port.flush();
}

bool CGRBLController::SendAsyncCommand(QString cmd, bool appendCR)
{
    if(!m_port.isOpen() || m_ResetInProgress) return false;

    cmd = UpdateFeedRateMultiplier(cmd, m_FeedRateMultiplier);

    QByteArray ba;
    ba.clear();
    ba.append(cmd.trimmed());
    if(appendCR) ba.append('\n');

    m_port.write(ba);
    m_port.flush();

    return true;
}

bool CGRBLController::EnqueueCommand(QString cmd)
{
    if(!m_port.isOpen() || m_ResetInProgress) return false;

    if(m_NeedFeedRateUpdate && (m_LastFeedRate > 0)) cmd = "F" + QString::number(m_LastFeedRate * m_FeedRateMultiplier, 'f', 3);
    else cmd = UpdateFeedRateMultiplier(cmd, m_FeedRateMultiplier);

    cmd = cmd.trimmed();
    if(cmd.contains(QRegExp("[t,T][(0-9)]*")))
    {
        emit ToolChangeRequest();
    }

    if(cmd.isEmpty()) return true;

    if((cmd.count() + m_BufFill) < GRBL_BUFFER_SIZE)
    {
        QStringList sl = cmd.split('\n');
        for(int i = 0; i < sl.count(); i++)
        {
            QByteArray ba;
            ba.clear();
            ba.append(sl.at(i) + "\n");
            m_CmdQueue.append(ba);
            m_BufFill += ba.count();

            if(m_CmdQueue.count() == 1)
            {
                m_port.write(m_CmdQueue.first());
                emit CommandSent(m_CmdQueue.first().trimmed());
            }
        }

        if(m_NeedFeedRateUpdate)
        {
            m_NeedFeedRateUpdate = false;
            return false;
        }

        return true;
    } else return false;
}

void CGRBLController::setCapturingResponse(bool on)
{
    if(on) m_CmdResponse.clear();
    m_CapturingResponse = on;
}

void CGRBLController::setFeedRateMultiplier(double mult)
{
    if(mult < 0.1) mult = 0.1;
    else if(mult > 10.0) mult = 10.0;

    m_FeedRateMultiplier = mult;
    m_NeedFeedRateUpdate = true;
}

double CGRBLController::getLastFeedRate()
{
    return m_LastFeedRate;
}

double CGRBLController::getActFeedRate()
{
    return m_LastFeedRate * m_FeedRateMultiplier;
}

QString CGRBLController::UpdateFeedRateMultiplier(QString cmd, double factor)
{
    QRegExp re("F[+-]?\\d*\\.?\\d+");
    bool ok;

//    qDebug() << ">" << cmd.trimmed();
    if(re.indexIn(cmd) >= 0)
    {
        foreach(QString match, re.capturedTexts())
        {
            double feedVal = match.remove(0, 1).toDouble(&ok);
            if(!ok) continue;
            m_LastFeedRate = feedVal;
            cmd.replace(match, QString::number(feedVal * factor, 'f', 3));
        }
    }
 //   qDebug() << "<" << cmd.trimmed();
 //   qDebug() << "-----------------------------------";

    return cmd;
}

void CGRBLController::serialReadyRead()
{
    while(m_port.canReadLine())
    {
        QString resp = m_port.readLine().simplified();

        if(m_RetrievingParams && resp.at(0) == '$') m_GrblParams.append(resp);

        if(resp.indexOf("Grbl") == 0)
        {
            m_port.clear();
            m_ResetInProgress = false;
            return;
        }

        if(resp.isEmpty() || m_ResetInProgress) continue;

        if(resp == "ok")
        {
            if(m_RetrievingParams)
            {
                m_RetrievingParams = false;
                emit ParamsRetreieved(m_GrblParams);
            }
            if(m_BufFill)
            {
                m_BufFill -= m_CmdQueue.first().count();
                m_CmdQueue.removeFirst();
            }
            if(!m_CmdQueue.isEmpty())
            {
                m_port.write(m_CmdQueue.first());
                emit CommandSent(m_CmdQueue.first().trimmed());
            }

            emit QueuedCommandDone();
        }
        else if(resp.contains("error:") || resp.contains("ALARM:"))
        {
            SendReset();
            emit CommandError(resp);
            return;
        }

        else if(resp.contains("[PRB:"))
        {
            emit SetZProbe();
        }

        else if(m_CapturingResponse)
        {
            m_CmdResponse.append(resp);
            emit ResponseLineReceieved(resp);
        }
        else m_CmdResponse.clear();
    }
}

