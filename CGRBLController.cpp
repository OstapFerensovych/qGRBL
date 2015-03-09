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
    m_ResetInProgress = true;
}

void CGRBLController::RetrieveParams()
{
    if(!m_port.isOpen() || m_ResetInProgress) return;

    m_port.write("$$\n");
}

bool CGRBLController::SendAsyncCommand(QString cmd, bool appendCR)
{
    if(!m_port.isOpen() || m_ResetInProgress) return false;

    QByteArray ba;
    ba.clear();
    ba.append(cmd.trimmed());
    if(appendCR) ba.append('\n');

    m_port.write(ba);

    return true;
}

bool CGRBLController::EnqueueCommand(QString cmd)
{
    if(!m_port.isOpen() || m_ResetInProgress) return false;

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

        return true;
    } else return false;
}

void CGRBLController::setCapturingResponse(bool on)
{
    if(on) m_CmdResponse.clear();
    m_CapturingResponse = on;
}

void CGRBLController::serialReadyRead()
{
    while(m_port.canReadLine())
    {
        QString resp = m_port.readLine().simplified();
        qDebug()<<resp;

        if(resp.indexOf("Grbl") == 0)
        {
            m_port.clear();
            m_ResetInProgress = false;
            return;
        }

        if(resp.isEmpty() || m_ResetInProgress) continue;

        if(resp == "ok")
        {
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
        else if(m_CapturingResponse)
        {
            m_CmdResponse.append(resp);
            emit ResponseLineReceieved(resp);
        }
        else m_CmdResponse.clear();
    }
}

