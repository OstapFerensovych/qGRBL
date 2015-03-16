#ifndef GRBL_SETTINGS_H
#define GRBL_SETTINGS_H

#include <QDialog>

namespace Ui {
class SettingsWindow;
}

class SettingsWindow : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsWindow(QWidget *parent = 0);
    ~SettingsWindow();

private slots:
    void on_buttonBox_accepted();

private:
    Ui::SettingsWindow *ui;
};

#endif // GRBL_SETTINGS_H
