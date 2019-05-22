#include "MainDialog.h"
#include "ui_MainDialog.h"
#include "ImageWindow.h"
#include <QDesktopWidget>
#include <QKeyEvent>
#include <QDebug>
#include <QFileDialog>
#include <QSettings>
#include "definitions.h"
#include <iostream>

using namespace std;

MainDialog::MainDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MainDialog)
{
    this->setWindowFlags(Qt::Dialog |
                         Qt::CustomizeWindowHint |
                         Qt::WindowTitleHint |
                         Qt::WindowMinimizeButtonHint);

    setAcceptDrops(true);

    ui->setupUi(this);

    setWindowTitle(QString("pxlpeep ") + QString(VERSION_STRING));

    imgButtons[0] = ui->btnImage0;
    imgButtons[1] = ui->btnImage1;
    imgButtons[2] = ui->btnImage2;
    imgButtons[3] = ui->btnImage3;
    imgButtons[4] = ui->btnImage4;
    imgButtons[5] = ui->btnImage5;
    imgButtons[6] = ui->btnImage6;
    imgButtons[7] = ui->btnImage7;
    imgButtons[8] = ui->btnImage8;
    imgButtons[9] = ui->btnImage9;

    nextButton = 0;
    for (int i = 0; i < MAX_BUTTONS; i++)
    {
        imgWindows[i] = nullptr;
        connect(imgButtons[i], &QPushButton::clicked, this, &MainDialog::slot_btnImage_clicked);
        resetButton(i);
    }
}

MainDialog::~MainDialog()
{
    this->on_btnCloseAll_clicked();

    delete ui;
}

void MainDialog::on_btnExit_clicked()
{
    this->close();
}

void MainDialog::on_btnCloseAll_clicked()
{
    for (int i = 0; i < MAX_BUTTONS; i++)
        if (imgWindows[i])
            imgWindows[i]->close();
}

// Don't start this with "on_" because we connect them manually in the constructor, and
// Qt will try to connect this automatically just by looking at the name. See
// https://linux.m2osw.com/qtwarning-qmetaobjectconnectslotsbyname-no-matching-signal-onsomethingevent
void MainDialog::slot_btnImage_clicked()
{
    QObject *obj = sender();
    QPushButton *button = static_cast<QPushButton *>(obj);
    int i;
    for ( i = 0; i < MAX_BUTTONS; i++)
        if (button == imgButtons[i])
            break;

    if (imgWindows[i] == nullptr)
        openAndShow(-1, ""); // Open a new window.
    else {
        imgWindows[i]->raise(); // Bring it up.
        imgWindows[i]->activateWindow(); // and make it live.
    }
}

void MainDialog::on_chkSyncWindows_stateChanged(int arg1)
{
    syncWindows = static_cast<bool>(arg1 == 2);
    if (syncWindows)
    {
        for (int i = 0; i < MAX_BUTTONS; i++)
        {
            if (imgWindows[i] == nullptr) continue;
            imgWindows[i]->resetWheelAccumulator();
        }
    }
}

void MainDialog::wheelEventInImageWindow(QWheelEvent *event, int windowID)
{
    if (! syncWindows) return;
    // Now emit the signal to all the windows that don't have this ID.
    for (int i = 0; i < MAX_BUTTONS; i++)
    {
        if (imgWindows[i] == nullptr) continue;
        if (i == windowID) continue;
        imgWindows[i]->handleWheelEvent(event, true);
    }
}

void MainDialog::mousePressEventInImageWindow(QMouseEvent *event, int windowID)
{
    if (! syncWindows) return;
    // Now emit the signal to all the windows that don't have this ID.
    for (int i = 0; i < MAX_BUTTONS; i++)
    {
        if (imgWindows[i] == nullptr) continue;
        if (i == windowID) continue;
        imgWindows[i]->handleMousePressEvent(event, true);
    }
}

void MainDialog::mouseMoveEventInImageWindow(QMouseEvent *event, int windowID)
{
    if (! syncWindows) return;
    // Now emit the signal to all the windows that don't have this ID.
    for (int i = 0; i < MAX_BUTTONS; i++)
    {
        if (imgWindows[i] == nullptr) continue;
        if (i == windowID) continue;
        imgWindows[i]->handleMouseMoveEvent(event, true);
    }
}

void MainDialog::mouseReleaseEventInImageWindow(QMouseEvent *event, int windowID)
{
    if (! syncWindows) return;
    // Now emit the signal to all the windows that don't have this ID.
    for (int i = 0; i < MAX_BUTTONS; i++)
    {
        if (imgWindows[i] == nullptr) continue;
        if (i == windowID) continue;
        imgWindows[i]->handleMouseReleaseEvent(event, true);
    }
}

void MainDialog::keyPressedInImageWindow(QKeyEvent *event, int windowID)
{
    if (! syncWindows) return;
    // Now emit the signal to all the windows that don't have this ID.
    for (int i = 0; i < MAX_BUTTONS; i++)
    {
        if (imgWindows[i] == nullptr) continue;
        if (i == windowID) continue;
        imgWindows[i]->handleKeyPress(event, true);
    }
}

void MainDialog::keyReleasedInImageWindow(QKeyEvent *event, int windowID)
{
    if (! syncWindows) return;
    // Now emit the signal to all the windows that don't have this ID.
    for (int i = 0; i < MAX_BUTTONS; i++)
    {
        if (imgWindows[i] == nullptr) continue;
        if (i == windowID) continue;
        imgWindows[i]->handleKeyRelease(event, true);
    }
}

void MainDialog::connectSignals(int ID)
{
    connect(imgWindows[ID], &ImageWindow::signalWheelEvent,        this, &MainDialog::wheelEventInImageWindow);
    connect(imgWindows[ID], &ImageWindow::signalMousePressEvent,   this, &MainDialog::mousePressEventInImageWindow);
    connect(imgWindows[ID], &ImageWindow::signalMouseMoveEvent,    this, &MainDialog::mouseMoveEventInImageWindow);
    connect(imgWindows[ID], &ImageWindow::signalMouseReleaseEvent, this, &MainDialog::mouseReleaseEventInImageWindow);
    connect(imgWindows[ID], &ImageWindow::signalKeyPressed,        this, &MainDialog::keyPressedInImageWindow);
    connect(imgWindows[ID], &ImageWindow::signalKeyReleased,       this, &MainDialog::keyReleasedInImageWindow);
    connect(imgWindows[ID], &ImageWindow::signalFileDeleted,       this, &MainDialog::imageWindowDeletedAFile);
}

void MainDialog::resetButton(int ID)
{
    imgButtons[ID]->setText("Open image...");
    imgButtons[ID]->setToolTip("");
}

void MainDialog::imageWindowClosing(int ID)
{
    imgWindows[ID] = nullptr;
    resetButton(ID);
}

void MainDialog::storeLastOpenFrom(QString filepath)
{
    //http://stackoverflow.com/questions/3597900/qsettings-file-chooser-should-remember-the-last-directory
    QSettings s;
    const QString key(KEY_LAST_USED_DIR);
    QDir d;
    s.setValue(key, d.absoluteFilePath(filepath));
}

QString MainDialog::retrieveLastOpenFrom()
{
    QSettings s;
    const QString key(KEY_LAST_USED_DIR);
    return s.value(key).toString();
}

void MainDialog::storeLastFileType(QString lastType)
{
    QSettings s;
    const QString key(KEY_LAST_FILE_TYPE);
    s.setValue(key, lastType);
}

QString MainDialog::retrieveLastFileType()
{
    QSettings s;
    const QString key(KEY_LAST_FILE_TYPE);
    return s.value(key).toString();
}

void MainDialog::openAndShow(int windowID, QString filename)
{
    ImageWindow *imgWindow;

    //We take this chance to find the next available window.
    int i;
    for (i = 0; i < MAX_BUTTONS; i++)
    {
        if (imgWindows[i] == nullptr)
        {
            nextButton = i;
            break;
        }
    }
    if (i == MAX_BUTTONS) //no unused windows; mark the next one for reuse.
        nextButton = (nextButton + 1) % MAX_BUTTONS;

    int ID;
    if (windowID == -1)
        ID = nextButton;
    else
        ID = windowID;

    if (imgWindows[ID] == nullptr) imgWindows[ID] = new ImageWindow(colormap, ID);

    imgWindow = imgWindows[ID];
    imgWindow->myButtonNo = ID;
    imgWindow->parent = this;
    connectSignals(ID);

    bool imgResult = false;
    imgResult = imgWindow->readImage(filename);

    QString lastFileType = retrieveLastFileType();
    QString lastFileLocation = retrieveLastOpenFrom();
    while(!imgResult)
    {
        filename = QFileDialog::getOpenFileName(
                    this,
                    "Open image",
                    lastFileLocation,
                    "All files (*.*);;JPEG (*.jpg *.jpeg);;TIFF (*.tif *.tiff);;PNG (*.png);;BMP (*.bmp);;Photoshop (*.PSD)", //filter
                    &lastFileType,
                    nullptr //options
                    );
        if (filename.isNull()) {
            // User canceled.
            if (windowID == -1) {
                // If we made a new window, delete it.
                delete imgWindows[ID];
                imgWindows[ID] = nullptr;
            }
            return;
        }
        imgResult = imgWindow->readImage(filename);
        if (!imgResult)
            cerr << "Failed to open image " << filename.toStdString() << endl;
    }
    storeLastOpenFrom(filename);
    storeLastFileType(lastFileType);
    imgWindow->holdAll();
    imgWindow->setColormap(ColormapPalette::Grey);
    imgWindow->setImageFunction(ImageWindowFunction::OneToOne);
    imgWindow->setWindowTitle(filename);
    imgButtons[ID]->setText(filename);
    imgButtons[ID]->setToolTip(filename);
    imgResult = imgWindow->releaseAll(); // Release before you zoom fit, or the image won't be centered.
    imgWindow->zoomFit();
    imgWindow->showWindow();
}

void MainDialog::keyPressEvent(QKeyEvent *e)
{
    if(e->key() != Qt::Key_Escape)
        QDialog::keyPressEvent(e);
}

#include <QMimeData>
// To enable dragging and dropping:
// 1. Accept the proposed action on enter, then
// 2. Handle actually doing something on drop.
void MainDialog::dragEnterEvent(QDragEnterEvent *e)
{
    const QMimeData *m = e->mimeData();
    if (m->hasUrls())
        e->acceptProposedAction();
}

void MainDialog::dropEvent(QDropEvent *e)
{
    const QMimeData *m = e->mimeData();
    if (m->hasUrls())
    {
        QList<QUrl> urls = m->urls();
        cout << "Drag drop event with " << urls.length() << " URL(s):" << endl;
        for (QUrl url : urls)
        {
            QString filename = url.toLocalFile();
            cout << filename.toStdString() << endl;
            openAndShow(-1, filename);
        }
        e->acceptProposedAction();
    }
}

int MainDialog::exec()
{
    this->move(-50000, -50000); //make sure the window isn't visible.
    QDialog::show();            //show the window, so that frameGeometry will be correct.
    QRect r = QApplication::desktop()->screenGeometry();
    this->move(r.width() - this->frameGeometry().width(), 0);
    for (QString image : filesToOpenAtStartup)
        openAndShow(-1, image);
    return QDialog::exec();     //make it modal.
}

void MainDialog::imageWindowDeletedAFile(QString &trashLocation)
{
    deletedFilesList.append(trashLocation);
    cout << deletedFilesList.length() << " files marked for deletion so far this session." << endl;
}

#include <QMessageBox>
void MainDialog::closeEvent(QCloseEvent *event)
{
    if(deletedFilesList.length() > 0)
    {
        QMessageBox msgBox;
        msgBox.setText("There are " + QString::number(deletedFilesList.length())
                       + " file(s) in temporary trash");
        msgBox.setInformativeText("Do you want to delete them permanently?");
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setDefaultButton(QMessageBox::Yes);
        int ret = msgBox.exec();

        if (ret == QMessageBox::Yes)
        {
            for ( QString fileToDelete : deletedFilesList)
            {
                QFile file(fileToDelete);
                if (file.remove())
                    cout << "Deleted " << fileToDelete.toStdString() << " permanently!" << endl;
                else
                    cerr << "Failed to delete " << fileToDelete.toStdString() << " permanently!" << endl;

                // See if the trash folder is empty.
                QDir trashFolder(QFileInfo(fileToDelete).absoluteDir());
                if (trashFolder.isEmpty())
                {
                    if (QDir().rmdir(trashFolder.path()))
                        cout << "Removed empty trash folder " << trashFolder.path().toStdString() << endl;
                    else
                        cerr << "Failed to remove empty trash folder " << trashFolder.path().toStdString() << endl;
                }
            }
        }
    }

    event->accept();
}
