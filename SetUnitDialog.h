#ifndef SETUNITDIALOG_H
#define SETUNITDIALOG_H

#include <QDialog>

class ImageWindow;

namespace Ui {
class SetUnitDialog;
}

class SetUnitDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SetUnitDialog(ImageWindow *image_wnd, QWidget *parent = nullptr);
    void addPixVal(double val);
    ImageWindow *image_wnd;
    ~SetUnitDialog();

private slots:
    void on_setButton_clicked();

    void on_pixVal_currentTextChanged(const QString &arg1);

private:
    Ui::SetUnitDialog *ui;

};

#endif // SETUNITDIALOG_H
