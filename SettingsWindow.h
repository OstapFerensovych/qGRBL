#ifndef GRBL_SETTINGS_H
#define GRBL_SETTINGS_H

#include <CGRBLController.h>
#include <QDialog>

namespace Ui {
class SettingsWindow;
}

class SettingsWindow : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsWindow(CGRBLController *grbl, QWidget *parent = 0);
    ~SettingsWindow();

private:
    Ui::SettingsWindow *ui;
    CGRBLController *m_grbl;

    void ReadGrblConfig();

private slots:
    void ParamsRetreieved(QStringList params);
    void ValidateParamData(QString s = "");
    void on_buttonBox_accepted();
};

#endif // GRBL_SETTINGS_H
