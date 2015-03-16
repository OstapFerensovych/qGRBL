#include "MainWindow.h"
#include "ui_MainWindow.h"

#include <QKeyEvent>
#include <QSettings>

void MainWindow::on_lineEdit_returnPressed()
{
    QString cmd = ui->lineEdit->text().simplified();

    if(!m_MCHistory.contains(cmd))
    {
        m_MCHistory.append(cmd);
        m_MCPointer = m_MCHistory.count();
    }

    ui->lineEdit->clear();
    grbl.EnqueueCommand(cmd);
}

bool MainWindow::eventFilter(QObject* obj, QEvent *event)
{
    if (obj == ui->lineEdit)
    {
        if (event->type() == QEvent::KeyPress)
        {
            QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
            if (keyEvent->key() == Qt::Key_Up)
            {
                m_MCPointer--;
                if(m_MCPointer < 0) m_MCPointer = 0;
                else ui->lineEdit->setText(m_MCHistory[m_MCPointer]);

                return true;
            }
            else if(keyEvent->key() == Qt::Key_Down)
            {
                m_MCPointer++;
                if(m_MCPointer > m_MCHistory.count() - 1) m_MCPointer = m_MCHistory.count() - 1;
                else ui->lineEdit->setText(m_MCHistory[m_MCPointer]);
                return true;
            }
        }
        return false;
    }
    return QMainWindow::eventFilter(obj, event);
}

void MainWindow::_ManualControlCreate()
{
    ui->lineEdit->installEventFilter(this);

    QSettings s;
    m_MCHistory = s.value("MCHistory_Data").toStringList();
    m_MCPointer = m_MCHistory.count();
}

void MainWindow::_ManualControlDelete()
{
    QSettings s;

    int maxCount = s.value("MCHistory_MaxCount", 100).toInt();
    while(m_MCHistory.count() > maxCount) m_MCHistory.removeFirst();
    s.setValue("MCHistory_Data", m_MCHistory);
    s.setValue("MCHistory_MaxCount", maxCount);
}
