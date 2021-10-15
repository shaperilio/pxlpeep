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
    bool fillScreenAtStartup;
    bool syncOpenWindowsAtStartup;
    void setButtonText(int ID, QString text);

private slots:
    void exitApp();
    void closeAllImageWindows();
    void imageButtonClicked();

    void toggleSyncWindows(int arg1);

    void wheelEventInImageWindow(QWheelEvent *event, int windowID);
    void mousePressEventInImageWindow(QMouseEvent *event, int windowID);
    void mouseMoveEventInImageWindow(QMouseEvent *event, int windowID);
    void mouseReleaseEventInImageWindow(QMouseEvent *event, int windowID);
    void keyPressedInImageWindow(QKeyEvent *event, int windowID);
    void keyReleasedInImageWindow(QKeyEvent *event, int windowID);
    void fillScreenWithOpenWindows();

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

    int getNumImageWindows();  //returns the number of open images.
    QList<int> getImageWindowList();  //returns a list with the indices of the open windows

    //These functions set and get the last used directory and type using the registry.
    const QString KEY_LAST_USED_DIR = "last_used_dir";
    void     storeLastOpenFrom(QString filepath);
    QString  retrieveLastOpenFrom();

    const QString KEY_LAST_FILE_TYPE = "last_file_type";
    void     storeLastFileType(QString lastType);
    QString retrieveLastFileType();

    int setNextWindowButton();
    void showImageWindow(ImageWindow *imgWindow);
    void openAndShow(int windowID, QString filename);
    void openAndPaste(int windowID);

    void resetButton(int ID);

    bool syncWindows = false;
    void connectSignals(int ID); //Connect signals to and from imageWindows.

    QStringList deletedFilesList;
    void closeEvent(QCloseEvent *event) Q_DECL_OVERRIDE;

private:
    Ui::MainDialog *ui;
};

#endif // MAINDIALOG_H
