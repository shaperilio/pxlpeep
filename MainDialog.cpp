#include "MainDialog.h"
#include "ui_MainDialog.h"
#include "ImageWindow.h"
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
        connect(imgButtons[i], &QPushButton::clicked, this, &MainDialog::imageButtonClicked);
        resetButton(i);
    }

    connect(ui->btnExit, &QPushButton::clicked, this, &MainDialog::exitApp);
    connect(ui->btnCloseAll, &QPushButton::clicked, this, &MainDialog::closeAllImageWindows);
    connect(ui->chkSyncWindows, &QCheckBox::stateChanged, this, &MainDialog::toggleSyncWindows);
    connect(ui->btnFillScreen, &QPushButton::clicked, this, &MainDialog::fillScreenWithOpenWindows);
}

MainDialog::~MainDialog()
{
    this->closeAllImageWindows();

    delete ui;
}

void MainDialog::exitApp()
{
    this->close();
}

void MainDialog::closeAllImageWindows()
{
    for (int i = 0; i < MAX_BUTTONS; i++)
        if (imgWindows[i])
            imgWindows[i]->close();
}

int MainDialog::getNumImageWindows()
{
    return this->getImageWindowList().size();
}

QList<int> MainDialog::getImageWindowList()
{
    QList<int> windowIndices;

    for (int i = 0; i < MAX_BUTTONS; i++)
        if (imgWindows[i])
            windowIndices.push_back(i);

    return windowIndices;
}

void MainDialog::imageButtonClicked()
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

void MainDialog::toggleSyncWindows(int arg1)
{
    syncWindows = arg1 == Qt::CheckState::Checked;
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

int MainDialog::setNextWindowButton()
{
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

    return nextButton;
}

void MainDialog::showImageWindow(ImageWindow *imgWindow)
{
    imgWindow->holdAll();
    imgWindow->setColormap(ColormapPalette::Grey);
    imgWindow->setImageFunction(ImageWindowFunction::OneToOne);
    imgWindow->releaseAll(); // Release before you zoom fit, or the image won't be centered.
    imgWindow->zoomFit();
    imgWindow->showWindow();
}

void MainDialog::openAndShow(int windowID, QString filename)
{
    ImageWindow *imgWindow;

    //We take this chance to find the next available window.
    setNextWindowButton();

    int ID;
    if (windowID == -1)
        ID = nextButton;
    else
        ID = windowID;

    if (imgWindows[ID] == nullptr) imgWindows[ID] = new ImageWindow(ID);

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
                    QFileDialog::Options() //default
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
    showImageWindow(imgWindow);
}

void MainDialog::fillScreenWithOpenWindows()
{
    auto indices = this->getImageWindowList();
    int numWindows = indices.size();
    if (numWindows == 1) {
        this->imgWindows[indices[0]]->snapWindowTo(ImageWindow::snapType::Max);
    }
    else if (numWindows == 2) {
        this->imgWindows[indices[0]]->snapWindowTo(ImageWindow::snapType::Left);
        this->imgWindows[indices[1]]->snapWindowTo(ImageWindow::snapType::Right);
    }
    else if (numWindows == 3 || numWindows == 4) {
        this->imgWindows[indices[0]]->snapWindowTo(ImageWindow::snapType::TopLeft);
        this->imgWindows[indices[1]]->snapWindowTo(ImageWindow::snapType::TopRight);
        this->imgWindows[indices[2]]->snapWindowTo(ImageWindow::snapType::BottomLeft);
        if (numWindows == 4)
            this->imgWindows[indices[3]]->snapWindowTo(ImageWindow::snapType::BottomRight);
    }
    else {
        cout << numWindows << " windows are open; don't know how to fill the screen." << endl;
    }
}

void MainDialog::openAndPaste(int windowID)
{
    ImageWindow *imgWindow;

    setNextWindowButton();

    int ID;
    if (windowID == -1)
        ID = nextButton;
    else
        ID = windowID;

    if (imgWindows[ID] == nullptr) imgWindows[ID] = new ImageWindow(ID);

    imgWindow = imgWindows[ID];
    imgWindow->myButtonNo = ID;
    imgWindow->parent = this;
    connectSignals(ID);

    if (!imgWindow->pasteFromClipboard())
    {
        imgWindow->close();
        return;
    }
    showImageWindow(imgWindow);
}

void MainDialog::setButtonText(int ID, QString text)
{
    imgButtons[ID]->setText(text);
    imgButtons[ID]->setToolTip(text);
}

void MainDialog::keyPressEvent(QKeyEvent *e)
{
    Qt::KeyboardModifiers mods = QApplication::keyboardModifiers();

    if(e->key() == Qt::Key_V && mods == Qt::ControlModifier)
    {
        openAndPaste(-1);
        return;
    }
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
        for (QUrl const &url : urls)
        {
            QString filename = url.toLocalFile();
            cout << filename.toStdString() << endl;
            openAndShow(-1, filename);
        }
        e->acceptProposedAction();
    }
}

#include <QScreen>
int MainDialog::exec()
{
    this->move(-50000, -50000); //make sure the window isn't visible.
    QDialog::show();            //show the window, so that frameGeometry will be correct.
    QRect r = QGuiApplication::primaryScreen()->geometry();
    this->move(r.width() - this->frameGeometry().width(), 0);
    for (QString const &image : filesToOpenAtStartup)
        openAndShow(-1, image);
    if (this->fillScreenAtStartup) {
        this->fillScreenWithOpenWindows();
        for (int idx : this->getImageWindowList())
            this->imgWindows[idx]->zoomFit();
    }
    if (this->syncOpenWindowsAtStartup) {
        this->ui->chkSyncWindows->setChecked(true);
    }
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
            for ( QString const &fileToDelete : deletedFilesList)
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
