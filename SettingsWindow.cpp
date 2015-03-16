#include "SettingsWindow.h"
#include "ui_SettingsWindow.h"

#include <QDebug>

SettingsWindow::SettingsWindow(CGRBLController *grbl, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingsWindow)
{
    m_grbl = grbl;
    ui->setupUi(this);

    ReadGrblConfig();
    connect(ui->lineEdit_21, SIGNAL(textChanged(QString)), this, SLOT(ValidateParamData(QString)));
    connect(ui->lineEdit_22, SIGNAL(textChanged(QString)), this, SLOT(ValidateParamData(QString)));
    connect(ui->lineEdit_23, SIGNAL(textChanged(QString)), this, SLOT(ValidateParamData(QString)));
    connect(ui->lineEdit_24, SIGNAL(textChanged(QString)), this, SLOT(ValidateParamData(QString)));
}

SettingsWindow::~SettingsWindow()
{
    delete ui;
}

void SettingsWindow::ReadGrblConfig()
{
    connect(m_grbl, SIGNAL(ParamsRetreieved(QStringList)), this, SLOT(ParamsRetreieved(QStringList)));
    m_grbl->RetrieveParams();
}

void SettingsWindow::ParamsRetreieved(QStringList params)
{
    bool ok;

    disconnect(m_grbl, SIGNAL(ParamsRetreieved(QStringList)), this, SLOT(ParamsRetreieved(QStringList)));
    foreach(QString param, params)
    {
        int eqIdx = param.indexOf('=');
        int spaceIdx = param.indexOf(' ', eqIdx);
        QString paramId = param.left(eqIdx);
        QString value = param.mid(eqIdx + 1, spaceIdx - eqIdx - 1);

        QList<QCheckBox *> allcb = findChildren<QCheckBox *>();
        foreach(QCheckBox *cb, allcb)
        {
            if(cb->statusTip().contains(paramId)) cb->setChecked(value.toInt());
        }

        QList<QLineEdit *> allle = findChildren<QLineEdit *>();
        foreach(QLineEdit *le, allle)
        {
            if(le->statusTip().contains(paramId))
            {
                if(le->statusTip().contains("mask"))
                {
                    quint8 val = value.toInt(&ok);
                    if(ok)
                    {
                        QString s = QString::number(val, 2);
                        while(s.count() < 8) s.prepend('0');
                        le->setText(s);
                    }
                }
            }
        }

        QList<QDoubleSpinBox *> alldsb = findChildren<QDoubleSpinBox *>();
        foreach(QDoubleSpinBox *dsb, alldsb)
        {
            if(dsb->statusTip().contains(paramId))
            {
                if(dsb->statusTip().contains("int"))
                {
                    int val = value.toInt(&ok);
                    if(ok) dsb->setValue(val);
                    else dsb->setValue(0);
                }
                else if(dsb->statusTip().contains("float"))
                {
                    double val = value.toFloat(&ok);
                    if(ok) dsb->setValue(val);
                    else dsb->setValue(0);
                }
            }
        }
    }

    ValidateParamData();
}

void SettingsWindow::ValidateParamData(QString)
{
    QList<QLineEdit *> allle = findChildren<QLineEdit *>();
    foreach(QLineEdit *le, allle)
    {
        if(le->statusTip().contains("mask"))
        {
            bool valid = le->text().count() == 8;

            for(int i = 0; i < le->text().count(); i++) if(le->text().at(i) != '0' && le->text().at(i) != '1') valid = false;
            if(valid) le->setStyleSheet("");
            else le->setStyleSheet("background-color: rgb(255, 64, 64);");
        }
    }
}


void SettingsWindow::on_buttonBox_accepted()
{
    bool ok;

    QList<QCheckBox *> allcb = findChildren<QCheckBox *>();
    foreach(QCheckBox *cb, allcb)
    {
        int idx = cb->statusTip().indexOf(',');
        if(idx >= 0)
        {
            QString param = cb->statusTip().left(idx) + "=";
            param.append(cb->isChecked() ? "1" : "0");
            m_grbl->SendAsyncCommand(param);
        }
    }

    QList<QLineEdit *> allle = findChildren<QLineEdit *>();
    foreach(QLineEdit *le, allle)
    {
        int idx = le->statusTip().indexOf(',');
        if(idx >= 0)
        {
            QString param = le->statusTip().left(idx) + "=";
            param.append(QString::number(le->text().toInt(&ok, 2)));
            m_grbl->SendAsyncCommand(param);
        }
    }

    QList<QDoubleSpinBox *> alldsb = findChildren<QDoubleSpinBox *>();
    foreach(QDoubleSpinBox *dsb, alldsb)
    {
        int idx = dsb->statusTip().indexOf(',');
        if(idx >= 0)
        {
            QString param = dsb->statusTip().left(idx) + "=";
            param.append(QString::number(dsb->value()));
            m_grbl->SendAsyncCommand(param);
        }
    }
}
