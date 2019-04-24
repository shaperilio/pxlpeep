#ifndef MAINDIALOG_H
#define MAINDIALOG_H

#include <QDialog>
#include "colormapper.h"

class ImageWindow;

namespace Ui {
class MainDialog;
}

class MainDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MainDialog(QWidget *parent = nullptr);
    ~MainDialog() Q_DECL_OVERRIDE;
    void imageWindowClosing(int ID);
    QStringList filesToOpenAtStartup;

private slots:
    void on_btnClose_clicked();
    void slot_btnImage_clicked();

    void on_chkSyncWindows_stateChanged(int arg1);

    void wheelEventInImageWindow(QWheelEvent *event, int windowID);
    void mousePressEventInImageWindow(QMouseEvent *event, int windowID);
    void mouseMoveEventInImageWindow(QMouseEvent *event, int windowID);
    void mouseReleaseEventInImageWindow(QMouseEvent *event, int windowID);
    void keyPressedInImageWindow(QKeyEvent *event, int windowID);
    void keyReleasedInImageWindow(QKeyEvent *event, int windowID);

public slots:
    int exec() Q_DECL_OVERRIDE; //must override this to get the right frameGeometry.
    void imageWindowDeletedAFile(QString &trashLocation);

protected:
    void keyPressEvent(QKeyEvent *e) Q_DECL_OVERRIDE; //to prevent ESC from closing the window.
    void dragEnterEvent(QDragEnterEvent *e) Q_DECL_OVERRIDE;
    void dropEvent(QDropEvent *e) Q_DECL_OVERRIDE;

    int nextButton = 0;                 //keeps track of the next available button in the main window.
    static const int MAX_BUTTONS = 10;  //put this many buttons in the dialog box.
    QPushButton *imgButtons[MAX_BUTTONS];
    ImageWindow *imgWindows[MAX_BUTTONS];

    Colormapper colormap; //one colormapper for all the windows to share.

    //These functions set and get the last used directory and type using the registry.
    const QString KEY_LAST_USED_DIR = "last_used_dir";
    void     storeLastOpenFrom(QString filepath);
    QString  retrieveLastOpenFrom();

    const QString KEY_LAST_FILE_TYPE = "last_file_type";
    void     storeLastFileType(int lastTypeIndex);
    QString retrieveLastFileType();

    void openAndShow(int windowID, QString filename);

    void resetButton(int ID);

    bool syncWindows = false;
    void connectSignals(int ID); //Connect signals to and from imageWindows.

    QStringList deletedFilesList;
    void closeEvent(QCloseEvent *event) Q_DECL_OVERRIDE;

private:
    Ui::MainDialog *ui;
};

#endif // MAINDIALOG_H
