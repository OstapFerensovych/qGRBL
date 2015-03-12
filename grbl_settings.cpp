#include "grbl_settings.h"
#include "ui_grbl_settings.h"

grbl_settings::grbl_settings(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::grbl_settings)
{
    ui->setupUi(this);
}

grbl_settings::~grbl_settings()
{
    delete ui;
}

void grbl_settings::on_buttonBox_accepted()
{

}
