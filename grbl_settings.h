#ifndef GRBL_SETTINGS_H
#define GRBL_SETTINGS_H

#include <QDialog>

namespace Ui {
class grbl_settings;
}

class grbl_settings : public QDialog
{
    Q_OBJECT

public:
    explicit grbl_settings(QWidget *parent = 0);
    ~grbl_settings();

private slots:
    void on_buttonBox_accepted();

private:
    Ui::grbl_settings *ui;
};

#endif // GRBL_SETTINGS_H
