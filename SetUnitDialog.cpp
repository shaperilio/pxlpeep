#include "SetUnitDialog.h"
#include "ui_SetUnitDialog.h"
#include "ImageWindow.h"
#include <QDoubleValidator>

SetUnitDialog::SetUnitDialog(ImageWindow *image_wnd, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SetUnitDialog)
{
    ui->setupUi(this);
    this->image_wnd = image_wnd;
    this->setWindowTitle("Set custom unit");
}

void SetUnitDialog::addPixVal(double val) {
    ui->pixVal->addItem(QString::asprintf("%.2f", val));
}

SetUnitDialog::~SetUnitDialog()
{
    delete ui;
}

void SetUnitDialog::on_setButton_clicked()
{
    const double n_pix = ui->pixVal->currentText().toDouble();
    const double n_unit = ui->unitVal->text().toDouble();
    const double unitPerPix = n_unit / n_pix;
    QString unitName = ui->unitName->text();
    this->image_wnd->setCustomUnit(unitName, unitPerPix);
    this->close();
}

void SetUnitDialog::on_pixVal_currentTextChanged(const QString &arg1)
{
    QString newVal = arg1;
    int cursorPos = ui->pixVal->currentIndex();
    QDoubleValidator validator(0.01, 1000*1000, 2);
    if (validator.validate(newVal, cursorPos) == QValidator::Invalid) {
        this->ui->setButton->setEnabled(false);
    } else {
        this->ui->setButton->setEnabled(true);
    }
}
