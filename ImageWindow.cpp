#include "definitions.h"
#include "ImageWindow.h"
#include <math.h>
#include <QWheelEvent>
#include <QDebug>
#include <QScrollBar>
#include <QFontDatabase>
#include <QDesktopWidget>
#include <QDir>
#include <iostream>
#include <QCollator>

using namespace std;

#define NOT_LOADED "not loaded"
#define DEFAULT_DATE QDateTime::fromString("1980-01-16 08:00:00", "yyyy-MM-dd hh:mm:ss")

ImageWindow::ImageWindow(int ID)
{
    this->ID = ID;
    holdAll();
    setDefaultOptions();
    this->resize(640, 480);

    scaleMin = 0;
    scaleMax = 255;
    offset = 0;
    scale = 1;

    resetWheelAccumulator();
    zoomFactor = 1;
    curMousePos = QPoint(0,0);
    ROIisValid = false;
    currentSnap = None;


    //Must set mouse tracking on the viewport, not "this", to get mouse move events without buttons!
    viewport()->setMouseTracking(true);

    //Load the font from the resource file.
    int id = QFontDatabase::addApplicationFont(":/fonts/ProFontWindows.ttf");
    QString family = QFontDatabase::applicationFontFamilies(id).at(0);
    windowFont = QFont(family);
#if defined(Q_OS_MACOS)
    // If we do a titleless window on Mac, you can't resize the window.
    // Tool windows have a smaller titlebar but they are always on top of the main window.
//    setWindowFlags(Qt::Tool);
//    setAttribute(Qt::WA_MacAlwaysShowToolWindow); // so it doesn't disappear when we lose focus.
    windowFont.setPointSize(11);
#elif defined(Q_OS_LINUX)
    // Ubuntu 20.04 LTS out of the box...
//    setWindowFlags(Qt::Tool); // sizeable, stays on top of main window, but has title bar.
    // Looks like there's no way to get a titleless, sizable window in 20.04 LTS anymore...
    setWindowFlags(Qt::CustomizeWindowHint); //no title bar, but not sizable.
    windowFont.setPointSize(8);
#else
    windowFont.setPointSize(8);
    setWindowFlags(Qt::CustomizeWindowHint); //sizeable border without title bar
#endif
    //The update mode is important because we will draw on top of the image things that
    //we don't want to scroll with the image (color bar, info box, etc.).
    //If you don't set this, you can get smearing of our hand-drawn items in the image.
    setViewportUpdateMode(QGraphicsView::FullViewportUpdate);

    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    setBackgroundBrush(QBrush(Qt::darkGreen));

    setCursor(Qt::CrossCursor);

    for (int i = 0; i < IMAGE_BUFFER_LENGTH; i++) {
        sourceImageBuffer[i] = nullptr;
        sourceImageBufferFilenames[i] = NOT_LOADED;
        sourceImageBufferModifiedDate[i] = DEFAULT_DATE;
    }
    scene = nullptr;
    colormap = new Colormapper();
    dipFactor = 1;
 }

void ImageWindow::resetWheelAccumulator()
{
    wheelAccumulator = 0;
}

void ImageWindow::closeEvent(QCloseEvent *event)
{
    if (parent != nullptr) parent->imageWindowClosing(ID);
    if (scene != nullptr) delete scene;
    for (int i = 0; i < IMAGE_BUFFER_LENGTH; i++)
        if (sourceImageBuffer[i] != nullptr) delete sourceImageBuffer[i];
    event->accept();
}

void ImageWindow::reportBufferContents()
{
    for (int i = 0; i < IMAGE_BUFFER_LENGTH; i++) {
        if (sourceImageBufferFilenames[i] == NOT_LOADED) continue;
        cout << "Image buffer slot " << i << ": " << sourceImageBufferFilenames[i].toStdString() <<
                " modified at " << sourceImageBufferModifiedDate[i].toString("yyyy-MM-dd hh:mm:ss").toStdString() << endl;
    }
}

void ImageWindow::setTitle(QString title)
{
    setWindowTitle(title);
    parent->setButtonText(myButtonNo, title);
}

bool ImageWindow::readImage(QString filename)
{
    cout << endl;
    if (filename == "")
        return false;

    filename = QFileInfo(filename).canonicalFilePath();

    reportBufferContents();

    int idx = -1;
    for (int i = 0; i < IMAGE_BUFFER_LENGTH; i++) {
        if (filename == sourceImageBufferFilenames[i]) {
            idx = i;
            break;
        }
    }

    if (idx == -1) {
        // This image is not in our buffer. Find a slot for it.
        sourceImageBufferIndex = (sourceImageBufferIndex + 1) % IMAGE_BUFFER_LENGTH;
        if (sourceImageBuffer[sourceImageBufferIndex] != nullptr) {
            delete sourceImageBuffer[sourceImageBufferIndex];
            sourceImageBuffer[sourceImageBufferIndex] = nullptr;
            sourceImageBufferFilenames[sourceImageBufferIndex] = NOT_LOADED;
            sourceImageBufferModifiedDate[sourceImageBufferIndex] = DEFAULT_DATE;
        }
        cout << "Image `" << filename.toStdString() << "` not buffered; loading from disk." << endl;
        currentImageBufferIndex = sourceImageBufferIndex;
    } else {
        // This image is in our buffer; we're done!
        currentImageBufferIndex = idx;
        cout << "Image `" << filename.toStdString() << "` found at buffer position " << idx << "." << endl;
        // But has it been modified?
        if (QFileInfo(filename).lastModified().toMSecsSinceEpoch() != sourceImageBufferModifiedDate[idx].toMSecsSinceEpoch()) {
            cout << "Image `" << filename.toStdString() << "` has been modified on disk. Reloading." << endl;
            currentImageBufferIndex = idx;
        }
        else {
            // Repeat all the code from below which executes after loading the image from disk.
            curFilename = filename;
            sourceImageBufferFilenames[currentImageBufferIndex] = curFilename;
            sourceImageBufferModifiedDate[currentImageBufferIndex] = QFileInfo(curFilename).lastModified();

            int lastSlash = curFilename.lastIndexOf('/');
            curDirectory = curFilename.left(lastSlash + 1);
            syncWithFolder();
            setTitle(filename);
            return translateImage();
        }
    }

    curFilename = "";

    cout << "Using buffer slot " << currentImageBufferIndex << endl;
    auto sourceImage = &(sourceImageBuffer[currentImageBufferIndex]);

    if((*sourceImage) == nullptr)
        (*sourceImage) = new ImageData();

    (*sourceImage)->painter = this;
    bool r = (*sourceImage)->readImage(filename);
    if (!r)
    {
        delete (*sourceImage);
        (*sourceImage) = nullptr;
        return r;
    }

    scene = new QGraphicsScene(this);
    setScene(scene);

    curFilename = filename;
    sourceImageBufferFilenames[currentImageBufferIndex] = curFilename;
    sourceImageBufferModifiedDate[currentImageBufferIndex] = QFileInfo(curFilename).lastModified();

    int lastSlash = curFilename.lastIndexOf('/');
    curDirectory = curFilename.left(lastSlash + 1);
    syncWithFolder();
    setTitle(filename);

    reportBufferContents();

    return translateImage();
}

bool ImageWindow::setColormap(ColormapPalette newColormap)
{
    if (newColormap == getColormap()) return true;
    if (!colormap->setColormap(newColormap)) return false;
    return translateImage();
}

bool ImageWindow::setImageFunction(ImageWindowFunction newFunction)
{
    if (newFunction == getImageFunction()) return true;
    function = newFunction;
    return translateImage();
}

bool ImageWindow::setDipFactor(double dipFactor)
{
    this->dipFactor = dipFactor;
    auto f = this->getImageFunction();
    if (f == ImageWindowFunction::BrightenDark || f == ImageWindowFunction::DarkenLight ||
        f == ImageWindowFunction::Log10BrightenDark || f == ImageWindowFunction::Log10DarkenLight)
        return translateImage();
    return true;
}

bool ImageWindow::setScaleMode(ImageWindowScaling newMode)
{
    if (newMode == getScaleMode()) return true;
    scaling = newMode;
    return translateImage();
}

bool ImageWindow::setUserMin(double newMin)
{
    if (newMin == getUserMin()) return true;
    userMin = newMin;
    if (getScaleMode() == ImageWindowScaling::User)
        return translateImage();

    return true;
}

bool ImageWindow::setUserMax(double newMax)
{
    if (newMax == getUserMax()) return true;
    userMax = newMax;
    if (getScaleMode() == ImageWindowScaling::User)
        return translateImage();

    return true;
}

bool ImageWindow::setImageRotation(ImageWindowRotation newRotation)
{
    if (newRotation == getImageRotation()) return true;
    rotation = newRotation;
    ROIisValid = false; //or else we need to recalculate it based on the previous and new orientation...
    return translateImage();
}

bool ImageWindow::setImageFlip(bool horizontal, bool vertical)
{
    if (horizontal == getImageFlipHorizontal() && vertical == getImageFlipVertical())
        return true;
    flipHorizontal = horizontal;
    flipVertical   = vertical;
    cout << "Flip H: " << flipHorizontal << "; flip V: " << flipVertical << endl;
    return translateImage();
}

int ImageWindow::getImageWidth()
{
    auto sourceImage = sourceImageBuffer[currentImageBufferIndex];
    if (sourceImage == nullptr)
        return 0;
    if (rotation == ImageWindowRotation::Zero || rotation == ImageWindowRotation::CCW180)
        return sourceImage->getWidth();
    else
        return sourceImage->getHeight();
}

int ImageWindow::getImageHeight()
{
    auto sourceImage = sourceImageBuffer[currentImageBufferIndex];
    if (sourceImage == nullptr)
        return 0;
    if (rotation == ImageWindowRotation::Zero || rotation == ImageWindowRotation::CCW180)
        return sourceImage->getHeight();
    else
        return sourceImage->getWidth();
}

bool ImageWindow::getTranslationParamsString(QString &params)
{
    if (!OKToTranslate)
    {
        cerr << "Attempt to get translation parameters when image is not OK to translate!" << endl;
        return false;
    }

    params = "_";
    params += QString::asprintf("s%d_", scaling);
    params += QString::asprintf("f%d_", function);
    params += QString::asprintf("m%d_", getColormap());
    params += QString::asprintf("r%d", rotation);
    auto sourceImage = sourceImageBuffer[currentImageBufferIndex];
    if (sourceImage->getNumChannels() == 1)
        return true;

    params += "_";
    if (activeChannels & chanR)
        params += "R";
    else
        params += "-";
    if (activeChannels & chanG)
        params += "G";
    else
        params += "-";
    if (activeChannels & chanB)
        params += "B";
    else
        params += "-";

    return true;
}

bool ImageWindow::translateImage()
{
    if (!OKToTranslate)
        return true;

    if (!(translatedImage.width() == getImageWidth() && translatedImage.height() == getImageHeight()))
    {
        scene->clear();
        translatedImage = QImage(getImageWidth(), getImageHeight(), QImage::Format_ARGB32);
        setSceneRect(-10000, -10000, getImageWidth() + 20000, getImageHeight() + 20000); //arbitrarily ridiculous limits to allow panning beyond image edges.
    }

    maxDisp = colormap->getMaxPaletteValue();

    auto sourceImage = sourceImageBuffer[currentImageBufferIndex];
    double maxVal = sourceImage->getMaxValue();
    double minVal = sourceImage->getMinValue();
    switch(scaling)
    {
    case ImageWindowScaling::Centered:
        double range;
        if (maxVal > minVal)
            range = applyImageFunction(maxVal);
        else
            range = applyImageFunction(minVal);

        if (range < 0) range *= -1;

        scaleMin = -range;
        scaleMax = range;

        scale  =  (double)maxDisp / 2 / range;
        offset = -(double)maxDisp / 2 / scale; //offset happens after scaling.
        break;

    case ImageWindowScaling::Fit:
        offset = applyImageFunction(minVal);

        if(maxVal == minVal)
            scale = 1;
        else
            scale = (double)maxDisp /
                    (applyImageFunction(maxVal) - applyImageFunction(minVal));

        scaleMin = minVal;
        scaleMax = maxVal;
        break;

    default:
        if (userMin == userMax)
            userMax += 255;
        offset = applyImageFunction(userMin);
        scale = (double)maxDisp /
                (applyImageFunction(userMax) - applyImageFunction(userMin));

        scaleMin = userMin;
        scaleMax = userMax;
        break;
     }

    int w = sourceImage->getWidth();
    int h = sourceImage->getHeight();
    if (sourceImage->getNumChannels() == 1 ||
        (activeChannels == chanR || activeChannels == chanG || activeChannels == chanB))
    {
        int channel;
        if (sourceImage->getNumChannels() == 1)
            channel = 0;
        else {
            if (activeChannels == chanR)
                channel = 0;
            else if (activeChannels == chanG)
                channel = 1;
            else channel = 2;
        }
        //Parallelizing this gets you a significant gain
        #pragma omp parallel for
        for (int r = 0; r < h; r++) {
            for (int c = 0; c < w; c++)
            {
                int srcIdx = r * w + c; //source pixel index
                int dstIdx; //destination pixel index; depends on rotation

                switch(rotation)
                {
                    case ImageWindowRotation::Zero:
                    {
                        int c2 = c;
                        int r2 = r;
                        if (flipHorizontal)
                            c2 = w - 1 - c;
                        if (flipVertical)
                            r2 = h - 1 - r;
                        dstIdx = (r2 * w + c2) * 4;
                        break;
                    }
                    case ImageWindowRotation::CCW90:
                    {
                        int c2 = c;
                        int r2 = (h - 1 - r);
                        if (flipHorizontal)
                            r2 = r;
                        if (flipVertical)
                            c2 = w - 1 - c;
                        dstIdx = (c2 * h + r2) * 4;
                        dstIdx = (h * w - 1) * 4 - dstIdx;
                        break;
                    }
                    case ImageWindowRotation::CCW180:
                    {
                        int c2 = c;
                        int r2 = r;
                        if (flipHorizontal)
                            c2 = w - 1 - c;
                        if (flipVertical)
                            r2 = h - 1 - r;
                        dstIdx = (r2 * w + c2) * 4;
                        dstIdx = (h * w - 1) * 4 - dstIdx;
                        break;
                    }
                    case ImageWindowRotation::CCW270:
                    {
                        int c2 = c;
                        int r2 = (h - 1 - r);
                        if (flipHorizontal)
                            r2 = r;
                        if (flipVertical)
                            c2 = w - 1 - c;
                        dstIdx = (c2 * h + r2) * 4;
                        break;
                    }
                }
                double srcPixel = (applyImageFunction(sourceImage->getPixel(srcIdx, channel)) - offset) * scale;
                const uchar *dstPixel = translatedImage.constBits() + dstIdx;
                colormap->translatePixel((uchar *)dstPixel, srcPixel);
            }
        }
    }
    else
    {
        #pragma omp parallel for
        for (int r = 0; r < h; r++) {
            for (int c = 0; c < w; c++)
            {
                int srcIdx = r * w + c; //source pixel index
                int dstIdx; //destination pixel index; depends on rotation

                switch(rotation)
                {
                    case ImageWindowRotation::Zero:
                    {
                        int c2 = c;
                        int r2 = r;
                        if (flipHorizontal)
                            c2 = w - 1 - c;
                        if (flipVertical)
                            r2 = h - 1 - r;
                        dstIdx = (r2 * w + c2) * 4;
                        break;
                    }
                    case ImageWindowRotation::CCW90:
                    {
                        int c2 = c;
                        int r2 = (h - 1 - r);
                        if (flipHorizontal)
                            r2 = r;
                        if (flipVertical)
                            c2 = w - 1 - c;
                        dstIdx = (c2 * h + r2) * 4;
                        dstIdx = (h * w - 1) * 4 - dstIdx;
                        break;
                    }
                    case ImageWindowRotation::CCW180:
                    {
                        int c2 = c;
                        int r2 = r;
                        if (flipHorizontal)
                            c2 = w - 1 - c;
                        if (flipVertical)
                            r2 = h - 1 - r;
                        dstIdx = (r2 * w + c2) * 4;
                        dstIdx = (h * w - 1) * 4 - dstIdx;
                        break;
                    }
                    case ImageWindowRotation::CCW270:
                    {
                        int c2 = c;
                        int r2 = (h - 1 - r);
                        if (flipHorizontal)
                            r2 = r;
                        if (flipVertical)
                            c2 = w - 1 - c;
                        dstIdx = (c2 * h + r2) * 4;
                        break;
                    }
                }

                double srcPixel0 = (activeChannels & chanR ? (applyImageFunction(sourceImage->getPixel(srcIdx, 0)) - offset) * scale : 0);
                double srcPixel1 = (activeChannels & chanG ? (applyImageFunction(sourceImage->getPixel(srcIdx, 1)) - offset) * scale : 0);
                double srcPixel2 = (activeChannels & chanB ? (applyImageFunction(sourceImage->getPixel(srcIdx, 2)) - offset) * scale : 0);

                const uchar *dstPixel = translatedImage.constBits() + dstIdx;
                colormap->translatePixelMultiChan((uchar *)dstPixel, srcPixel0, srcPixel1, srcPixel2);
            }
        }
    }

    translationIsFresh = true;

    translatedPixmap = QPixmap::fromImage(translatedImage);
    scene->addPixmap(translatedPixmap);
    viewport()->update();

    return true;
}

void ImageWindow::checkROIpoint(QPoint &point)
{
    if(point.x() < 0)
        point.setX(0);
    if(point.y() < 0)
        point.setY(0);
    auto sourceImage = sourceImageBuffer[currentImageBufferIndex];
    if(point.x() > sourceImage->getWidth())
        point.setX(sourceImage->getWidth());
    if(point.y() > sourceImage->getHeight())
        point.setY(sourceImage->getHeight());
}

void ImageWindow::handleMousePressEvent(QMouseEvent *event, bool /*forwarded*/)
{
    setShowHelp(false);

    if(event->buttons() == Qt::LeftButton && event->modifiers() != Qt::ShiftModifier)
    {
        panRef = event->globalPos();
        setCursor(Qt::ClosedHandCursor);
    }
    else if (event->buttons() == Qt::RightButton)
        mouseDragRef = event->globalPos() - frameGeometry().topLeft();
    else if (event->buttons() == Qt::LeftButton && event->modifiers() == Qt::ShiftModifier)
    {
        ROI1 = QPoint(floor(mapToScene(event->pos()).x() + 0.5), floor(mapToScene(event->pos()).y() + 0.5));
        checkROIpoint(ROI1);
    }

    event->accept();
}

void ImageWindow::mousePressEvent(QMouseEvent *event)
{
    emit signalMousePressEvent(event, this->ID);

    handleMousePressEvent(event, false);
}

void ImageWindow::handleMouseReleaseEvent(QMouseEvent *event, bool /*forwarded*/)
{
    setCursor(Qt::CrossCursor);

    //Note the use of button and not buttons!
    if (event->button() == Qt::LeftButton && event->modifiers() == Qt::ShiftModifier)
    {
        //Select an ROI. This is here so that a single click will clear the ROI.
        ROI2 = QPoint(floor(mapToScene(event->pos()).x() + 0.5), floor(mapToScene(event->pos()).y() + 0.5));
        checkROIpoint(ROI2);
        ROIisValid = (bool)(abs(ROI1.x() - ROI2.x()) >= 1 && abs(ROI1.y() - ROI2.y()) >= 1);
        viewport()->update();
    }
    event->accept();
}

void ImageWindow::mouseReleaseEvent(QMouseEvent *event)
{
    emit signalMouseReleaseEvent(event, this->ID);

    handleMouseReleaseEvent(event, false);
}

void ImageWindow::handleMouseMoveEvent(QMouseEvent *event, bool forwarded)
{
    if (event->buttons() == Qt::RightButton && !forwarded)
    {
        //Move the window.
        currentSnap = None;
        QPoint delta = event->globalPos() - mouseDragRef;
        move(delta);
    }
    else if (event->buttons() == Qt::LeftButton && event->modifiers() != Qt::ShiftModifier)
    {
        //Pan the image.
        QPoint panDelta = event->globalPos() - panRef;
        curMousePos += panDelta; //or the info box will show coordinates changing.
        horizontalScrollBar()->setValue(horizontalScrollBar()->value() - panDelta.x());
        verticalScrollBar()->setValue  (verticalScrollBar()->value()   - panDelta.y());
        panRef = event->globalPos();
    }
    else if (event->buttons() == Qt::LeftButton && event->modifiers() == Qt::ShiftModifier)
    {
        //Select an ROI.
        ROI2 = QPoint(floor(mapToScene(event->pos()).x() + 0.5), floor(mapToScene(event->pos()).y() + 0.5));
        checkROIpoint(ROI2);
        ROIisValid = (bool)(abs(ROI1.x() - ROI2.x()) >= 1 && abs(ROI1.y() - ROI2.y()) >= 1);
        viewport()->update();
    }
    else
    {
        if (! forwarded)
        {
            curMousePos = QPoint(event->pos().x(), event->pos().y());
            //Must update the viewport, not the window!!!!!
            //http://stackoverflow.com/a/3318205/149506

            // 2020-12-13: I think there's a bug in Qt?
            // I'm calculating infoRect and cusorInfoRect when I draw them - and drawing a filled
            // rectangle with those exact dimensions.
            // If I request an update with those very same rectangles, I see a line along the bottom and the
            // right that are not repainted (Ubuntu 20.04 LTS straight ouf the box here).
            // So I make the update region slightly larger.
            QRect infoUpdate(infoUpdateRegion.topLeft(), QSize(infoUpdateRegion.width() + 1, infoUpdateRegion.height() + 1));
            viewport()->update(infoUpdate);
//            cout << "viewport()->update for " << infoRect.left() << ", " << infoRect.top() << " to "
//                 << infoRect.right() << ", " << infoRect.bottom() << endl;

            if (showCursorInfoBox) {
                QRect cursorInfoUpdate(cursorInfoUpdateRegion.topLeft(), QSize(cursorInfoUpdateRegion.width() + 1, cursorInfoUpdateRegion.height() + 1));
                viewport()->update(cursorInfoUpdate);
//                cout << "viewport()->update for " << cursorInfoRect.left() << ", " << cursorInfoRect.top() << " to "
//                     << cursorInfoRect.right() << ", " << cursorInfoRect.bottom() << endl;
            }
        }
    }

    event->accept();
}

void ImageWindow::mouseMoveEvent(QMouseEvent *event)
{
    emit signalMouseMoveEvent(event, this->ID);

    handleMouseMoveEvent(event, false);
}

void ImageWindow::handleWheelEvent(QWheelEvent *event, bool /*forwarded*/)
{
    showHelp = false;

    wheelAccumulator += event->angleDelta().y();
    zoomCtr = QPoint(event->position().x(), event->position().y());

    if(wheelAccumulator < -deltaThresh)
    {
        wheelAccumulator = 0;
        zoom(-1);
    }
    else if (wheelAccumulator > deltaThresh)
    {
        wheelAccumulator = 0;
        zoom(+1);
    }

    event->accept();
}

void ImageWindow::wheelEvent(QWheelEvent *event)
{
    emit signalWheelEvent(event, this->ID);

    handleWheelEvent(event, false);
}

void ImageWindow::zoomFit()
{
    double wLevel, hLevel;

    wLevel = log((double)frameGeometry().width()  / (double)getImageWidth())  / log(zoomStep);
    hLevel = log((double)frameGeometry().height() / (double)getImageHeight()) / log(zoomStep);

    zoomLevel = (int)floor(qMin(wLevel, hLevel));
    zoomFactor = pow(zoomStep, zoomLevel);
    QMatrix scaleMatrix;
    scaleMatrix.scale(zoomFactor, zoomFactor);
    setTransform(QTransform(scaleMatrix));
    showCentered();
}

void ImageWindow::showCentered()
{
    showCorner(ImageCorner::Center);
}

void ImageWindow::showCorner(ImageCorner corner)
{
    // This will only properly move the image if it has been translated.
    auto hScroll = horizontalScrollBar();
    auto vScroll = verticalScrollBar();

    // We go 5% of the minimum window dimension beyond the edges of images to accentuate the fact you're at a corner.
    double edgeMargin = 0.05 * min(width(), height());

    if (corner == ImageCorner::Center) {
        hScroll->setValue((hScroll->maximum() + hScroll->minimum()) / 2);
        vScroll->setValue((vScroll->maximum() + vScroll->minimum()) / 2);
        return;
    }
    if (corner == ImageCorner::TopLeft) {
        hScroll->setValue(-edgeMargin);
        vScroll->setValue(-edgeMargin);
        return;
    }
    if (corner == ImageCorner::BottomLeft) {
        hScroll->setValue(-edgeMargin);
        vScroll->setValue(getImageHeight() * zoomFactor + edgeMargin - height());
        return;
    }
    if (corner == ImageCorner::TopRight) {
        hScroll->setValue(getImageWidth() * zoomFactor + edgeMargin - width());
        vScroll->setValue(-edgeMargin);
        return;
    }
    if (corner == ImageCorner::BottomRight) {
        hScroll->setValue(getImageWidth()  * zoomFactor + edgeMargin - width() );
        vScroll->setValue(getImageHeight() * zoomFactor + edgeMargin - height());
        return;
    }
}

void ImageWindow::zoom(int zoomIncrement)
{
    if(zoomIncrement == 0) //if we send in zero, we reset zoom to 1:1.
    {
        zoomLevel = 0;
        zoomFactor = 1;
    }
    else
    {
        zoomLevel += zoomIncrement;
        if(zoomLevel > ImageWindow::maxZoomLevel)
        {
            zoomLevel = ImageWindow::maxZoomLevel;
            return;
        }
        if(zoomLevel < ImageWindow::minZoomLevel)
        {
            zoomLevel = ImageWindow::minZoomLevel;
            return;
        }

        zoomFactor = pow(zoomStep, zoomLevel);
    }
    auto zoomLocationOnImageBefore = mapToScene(zoomCtr);
    QMatrix scaleMatrix;
    scaleMatrix.scale(zoomFactor, zoomFactor);
    setTransform(QTransform(scaleMatrix));
    auto zoomCtrAfter = mapFromScene(zoomLocationOnImageBefore);
    auto zoomLocationDelta = zoomCtrAfter - zoomCtr;
    horizontalScrollBar()->setValue(horizontalScrollBar()->value() + zoomLocationDelta.x());
    verticalScrollBar()  ->setValue(verticalScrollBar()  ->value() + zoomLocationDelta.y());
}

void ImageWindow::paintEvent(QPaintEvent *event)
{
    if (!OKToDraw) return;

    QGraphicsView::paintEvent(event);

//    auto r = event->rect();
//    cout << "paintEvent for " << r.left() << ", " << r.top() << " to " << r.right() << ", " << r.bottom() << endl;

    drawRulers();
    drawInfoBox();
    drawCursorInfoBox();
    drawHelp();
    drawColorbar();
    drawROI();

    event->accept();
}

inline double ImageWindow::applyImageFunction(double value)
{
    switch(this->function)
    {
    case ImageWindowFunction::Log10BrightenDark :
        return imageFunctionLog10BrightenDark(value);
    case ImageWindowFunction::Log10DarkenLight :
        return imageFunctionLog10DarkenLight(value);
    case ImageWindowFunction::BrightenDark :
        return imageFunctionBrightenDark(value);
    case ImageWindowFunction::DarkenLight :
        return imageFunctionDarkenLight(value);
    default :
        return imageFunctionNone(value);
    }
}

inline double ImageWindow::imageFunctionLog10BrightenDark(double value)
{
    if (value > 0) {
        double val = log10(value * dipFactor * dipFactor);
        if (val > 0) return val;
    }
    return 0;
}

inline double ImageWindow::imageFunctionLog10DarkenLight(double value)
{
    if (value > 0) {
        double val = log10(value / dipFactor / dipFactor);
        if (val > 0) return val;
    }
    return 0;
}

inline double ImageWindow::imageFunctionNone(double value)
{
    return value;
}

inline double parabolicResponse(double input, double minVal, double maxVal, double dipFactor)
{
    // Parabolic profile. o = ai^2 + bi + c
    // When i = minVal, o = minVal.
    // when i = maxVal, o = maxVal.
    // when i = (maxVal + minVal) / 2, o = (maxVal + minVal) / 2 * dipFactor.
    const double a = 2 * (maxVal + minVal) * (1 - dipFactor) / (maxVal - minVal) / (maxVal - minVal);
    const double b = 1 - a * (maxVal + minVal);
    const double c = (1 - b) / (maxVal + minVal) * (maxVal + minVal) * (maxVal + minVal) / 4 + (b - dipFactor) * (maxVal + minVal) / 2;
    return a * input * input + b * input + c;
}

inline double ImageWindow::imageFunctionBrightenDark(double value)
{
    auto sourceImage = sourceImageBuffer[currentImageBufferIndex];
    double maxVal = sourceImage->getMaxValue();
    double minVal = sourceImage->getMinValue();
    double val = parabolicResponse(value, minVal, maxVal, dipFactor);
    if (val > maxVal) return maxVal;
    if (val < minVal) return minVal;
    return val;
}

inline double ImageWindow::imageFunctionDarkenLight(double value)
{
    auto sourceImage = sourceImageBuffer[currentImageBufferIndex];
    double maxVal = sourceImage->getMaxValue();
    double minVal = sourceImage->getMinValue();
    double val = parabolicResponse(value, minVal, maxVal, 1/dipFactor);
    if (val > maxVal) return maxVal;
    if (val < minVal) return minVal;
    return val;
}

void ImageWindow::drawInfoBox()
{
    if (!showInfoBox) return;

    QFontMetrics fm(windowFont);

    //What we will show:
    //Filename
    QString line1 = curFilename;

    //Modified date
    QString line2 = "Last modified " + sourceImageBufferModifiedDate[currentImageBufferIndex].toString("yyyy-MM-dd hh:mm:ss");

    //Width, height, zoom, and rotation
    auto sourceImage = sourceImageBuffer[currentImageBufferIndex];
    QString line3 = QString::asprintf("W = %d, H = %d pix (%.2fX)",
                                      sourceImage->getWidth(), sourceImage->getHeight(), zoomFactor);

    QString rot;
    int w, h;
    switch (rotation)
    {
    case ImageWindowRotation::Zero:
        rot = "";
        w = sourceImage->getWidth();
        h = sourceImage->getHeight();
        break;
    case ImageWindowRotation::CCW90:
        rot = " 90deg";
        w = sourceImage->getHeight();
        h = sourceImage->getWidth();
        break;
    case ImageWindowRotation::CCW180:
        rot = " 180deg";
        w = sourceImage->getWidth();
        h = sourceImage->getHeight();
        break;
    case ImageWindowRotation::CCW270:
        rot = " 270deg";
        w = sourceImage->getHeight();
        h = sourceImage->getWidth();
        break;
    }
    line2.append(rot);
    if (flipHorizontal && !flipVertical)
        line2.append(" H flip");
    if (!flipHorizontal && flipVertical)
        line2.append(" V flip");
    if (flipHorizontal && flipVertical)
        line2.append(" H+V flip");

    QString lineExposure;
    bool lineExposureValid = false;
    int ISO;
    if (sourceImage->getEXIFISO(ISO))
    {
        lineExposureValid = true;
        double shutter, EV;
        sourceImage->getEXIFShutter(shutter);
        sourceImage->getEXIFEV(EV);

        lineExposure = QString::asprintf("ISO = %d, shutter = %.2f ms, EV = %.2f", ISO, shutter, EV);
    }

    QString lineDate;
    bool lineDateValid = false;
    QString date;
    if (sourceImage->getEXIFDate(date))
    {
        lineDateValid = true;
        QString device, firmware;
        sourceImage->getEXIFMake(device);
        sourceImage->getEXIFFirmware(firmware);
        lineDate = "Created " + date + ", " + device + " (" + firmware + ")";
    }

    //Pixel coordinates
    QPointF zoomedPos = mapToScene(curMousePos);
    QString line5;

    double curX, curY;

    curY = zoomedPos.y();
    if (imgYOriginIsBottom)
        curY = getImageHeight() - curY;
    curY = curY + (imgIsZeroIndexed ? 0 : 1) - 0.5;

    curX = zoomedPos.x() + (imgIsZeroIndexed ? 0 : 1) - 0.5;

    double rX = curX - (imgIsZeroIndexed ? 0 : 1) - static_cast<double>(w)/2 + 0.5;
    double rY = curY - (imgIsZeroIndexed ? 0 : 1) - static_cast<double>(h)/2 + 0.5;
    double curR = sqrtf(rX*rX + rY*rY);
    double curTheta = atan2(rY, rX) * 180 / M_PI;

    line5 = QString::asprintf("X = %.2f, Y = %.2f (R = %.2f, θ = %.2f)", curX, curY, curR, curTheta);

    QString colorVal;
    if (zoomedPos.x() > 0 && zoomedPos.x() < sourceImage->getWidth() &&
        zoomedPos.y() > 0 && zoomedPos.y() < sourceImage->getHeight())
    {
        int x, y;
        switch (rotation)
        {
        case ImageWindowRotation::Zero:
            x = (int)zoomedPos.x();
            y = (int)zoomedPos.y();
            break;
        case ImageWindowRotation::CCW90:
            x = sourceImage->getHeight() - 1 - (int)zoomedPos.y();
            y = (int)zoomedPos.x();
            break;
        case ImageWindowRotation::CCW180:
            x = (int)zoomedPos.x();
            y = (int)zoomedPos.y();
            x = sourceImage->getWidth()  - 1 - x;
            y = sourceImage->getHeight() - 1 - y;
            break;
        case ImageWindowRotation::CCW270:
            x = (int)zoomedPos.y();
            y = sourceImage->getWidth() - 1 - (int)zoomedPos.x();
            break;
        }

        char fmt[5];
        if (sourceImage->getBPP() <= 8)
            sprintf(fmt, "%%03d");
        else
            sprintf(fmt, "%%05d");

        if (sourceImage->getNumChannels() == 1)
        {
            colorVal = QString::asprintf(fmt, (int)sourceImage->getPixel(x, y, 0));
            colorVal = " -> " + colorVal;
        }
        else
        {
            QString R, G, B;
            if (activeChannels & chanR)
                R = QString::asprintf(fmt, (int)sourceImage->getPixel(x, y, 0));
            else
                R = "OFF";

            if (activeChannels & chanG)
                G = QString::asprintf(fmt, (int)sourceImage->getPixel(x, y, 1));
            else
                G = "OFF";

            if (activeChannels & chanB)
                B = QString::asprintf(fmt, (int)sourceImage->getPixel(x, y, 2));
            else
                B = "OFF";

            colorVal = " -> " + R + ", " + G + ", " + B;
        }
        line5.append(colorVal);
    }

    int maxW = fm.horizontalAdvance(line1);
    if(fm.horizontalAdvance(line2) > maxW) maxW = fm.horizontalAdvance(line2);
    if(lineExposureValid && fm.horizontalAdvance(lineExposure) > maxW) maxW = fm.horizontalAdvance(lineExposure);
    if(lineDateValid && fm.horizontalAdvance(lineDate) > maxW) maxW = fm.horizontalAdvance(lineDate);
    if(fm.horizontalAdvance(line5) > maxW) maxW = fm.horizontalAdvance(line5);

    int lineH = fm.height();
    int lineVspace = 0;

    int infoW = maxW + 2 * boxMargin;
    int numLines = 4 + (lineExposureValid ? 1 : 0) + (lineDateValid ? 1 : 0);
    int infoH = lineH * numLines + lineVspace * (numLines - 1) + 2 * boxMargin;

    QRect winRect = geometry();

    infoUpdateRegion.setLeft(winRect.width() - 1 - boxPad - infoW);
    infoUpdateRegion.setTop(boxPad);
    infoUpdateRegion.setWidth(infoW);
    infoUpdateRegion.setHeight(infoH);

    //Now start drawing stuff.
    QPainter p(viewport());
    p.setBrush(Qt::black);
    p.drawRect(infoUpdateRegion);

    p.setPen(Qt::white);
    p.setFont(windowFont);
    QRect lineRect(infoUpdateRegion.x(), infoUpdateRegion.y(), infoUpdateRegion.width()*2, infoUpdateRegion.height()*2);

    lineRect.setX(infoUpdateRegion.x() + infoUpdateRegion.width() - fm.horizontalAdvance(line1) - boxMargin);
    lineRect.setY(lineRect.y() + boxMargin);
    p.drawText(lineRect, line1);

    lineRect.setX(infoUpdateRegion.x() + infoUpdateRegion.width() - fm.horizontalAdvance(line2) - boxMargin);
    lineRect.setY(lineRect.y() + lineVspace + lineH);
    p.drawText(lineRect, line2);

    lineRect.setX(infoUpdateRegion.x() + infoUpdateRegion.width() - fm.horizontalAdvance(line3) - boxMargin);
    lineRect.setY(lineRect.y() + lineVspace + lineH);
    p.drawText(lineRect, line3);

    if (lineExposureValid)
    {
        lineRect.setX(infoUpdateRegion.x() + infoUpdateRegion.width() - fm.horizontalAdvance(lineExposure) - boxMargin);
        lineRect.setY(lineRect.y() + lineVspace + lineH);
        p.drawText(lineRect, lineExposure);
    }

    if (lineDateValid)
    {
        lineRect.setX(infoUpdateRegion.x() + infoUpdateRegion.width() - fm.horizontalAdvance(lineDate) - boxMargin);
        lineRect.setY(lineRect.y() + lineVspace + lineH);
        p.drawText(lineRect, lineDate);
    }

    lineRect.setX(infoUpdateRegion.x() + infoUpdateRegion.width() - fm.horizontalAdvance(line5) - boxMargin);
    lineRect.setY(lineRect.y() + lineVspace + lineH - 2);
    p.drawText(lineRect, line5);
}

void ImageWindow::drawCursorInfoBox()
{
    if (!showCursorInfoBox) return;

    QPainter p(viewport());
    // If the zoom level is high enough, we will draw a marker that is locked to half-pixel units, and the info box will be
    // locked to that. Otherwise, we just follow the mouse cursor with the info box.
    QPoint infoBoxRef; // reference coordinates for the inf box.
    double const markerZoomLevel = 64;
    QRect pixelMarkerRect;

    //Pixel coordinates
    QPointF zoomedPos = mapToScene(curMousePos);

    if (zoomFactor >= markerZoomLevel) {
        // Round them to the nearest 0.5 pixels.
        zoomedPos.setX(floor(zoomedPos.x() * 2 + 0.5) / 2.0);
        zoomedPos.setY(floor(zoomedPos.y() * 2 + 0.5) / 2.0);
        QPoint pixelPosition = mapFromScene(zoomedPos);
        infoBoxRef = pixelPosition;

        // Draw a marker there.
        int third = 3;
        int half = third * 3;
        int pixelMarkerSize = half * 2;
        pixelMarkerRect = QRect(pixelPosition.x() - half, pixelPosition.y() - half, pixelMarkerSize, pixelMarkerSize);
        QRect markerBox(pixelMarkerRect);
        p.setBrush(Qt::black);
        p.drawRect(markerBox);
        markerBox.adjust(third, third, -third, -third);
        p.setBrush(Qt::white);
        p.drawRect(markerBox);
        markerBox.adjust(third, third, -third, -third);
        p.setBrush(Qt::black);
        p.drawRect(markerBox);
    } else {
        infoBoxRef = curMousePos;
    }

    // Compute the pixel coordinates like in drawInfoBox
    auto sourceImage = sourceImageBuffer[currentImageBufferIndex];
    int w, h;
    if (rotation == ImageWindowRotation::Zero || rotation == ImageWindowRotation::CCW180) {
        w = sourceImage->getWidth();
        h = sourceImage->getHeight();
    } else {
        w = sourceImage->getHeight();
        h = sourceImage->getWidth();
    }

    double curX, curY;

    curY = zoomedPos.y();
    if (imgYOriginIsBottom)
        curY = getImageHeight() - curY;
    curY = curY + (imgIsZeroIndexed ? 0 : 1) - 0.5;

    curX = zoomedPos.x() + (imgIsZeroIndexed ? 0 : 1) - 0.5;

    double rX = curX - (imgIsZeroIndexed ? 0 : 1) - static_cast<double>(w)/2 + 0.5;
    double rY = curY - (imgIsZeroIndexed ? 0 : 1) - static_cast<double>(h)/2 + 0.5;
    double curR = sqrtf(rX*rX + rY*rY);
    double curTheta = atan2(rY, rX) * 180 / M_PI;

    QString line1 = QString::asprintf("X = %.1f, Y = %.1f", curX, curY);
    QString line2 = QString::asprintf("R = %.1f, θ = %.1f", curR, curTheta);

    // Draw the info box.
    QFontMetrics fm(windowFont);
    int line1Width = fm.horizontalAdvance(line1);
    int line2Width = fm.horizontalAdvance(line2);
    int maxW = line1Width;
    if (line2Width > maxW) maxW = line2Width;
    int infoW = maxW + 2 * boxMargin;

    int lineH = fm.height();
    int infoH = lineH * 2 + 2 * boxMargin;

    int x = infoBoxRef.x() + 10;
    int y = infoBoxRef.y() + 10;

    // Ensure the whole box fits inside the window.
    QRect winRect = geometry();
    int limitW = winRect.width();
    int limitH = winRect.height();

    if (x + infoW > limitW) x = limitW - infoW;
    if (x < 0) x = 0;

    if (y + infoH > limitH) y = limitH - infoH;
    if (y < 0) y = 0;

    QRect cursorInfoBox(x, y, infoW, infoH);

    //Now start drawing stuff.
    p.setBrush(Qt::black);
    p.drawRect(cursorInfoBox);

    p.setPen(Qt::white);
    p.setFont(windowFont);
    QRect lineRect(cursorInfoBox.left() + cursorInfoBox.width() - line1Width - boxMargin, cursorInfoBox.top() + boxMargin,
                   cursorInfoBox.width()*2, cursorInfoBox.height()*2);
    p.drawText(lineRect, line1);
    lineRect.setX(cursorInfoBox.left() + cursorInfoBox.width() - line2Width - boxMargin);
    lineRect.setY(lineRect.top() + lineH);
    p.drawText(lineRect, line2);

    // Set the update region, depending on whether or not we've drawn the pixel marker.
    cursorInfoUpdateRegion = cursorInfoBox.united(pixelMarkerRect);
}

void ImageWindow::drawRulers()
{
    if (!showRulers) return;

    int minTickSpacingpix = 100;
    int tickLen = 15;

    QFontMetrics fm(windowFont);
    QPainter p(viewport());
    p.setFont(windowFont);

    QRect r = viewport()->geometry();

    QPen blackPen(QBrush(Qt::black), 3);
    QPen whitePen(QBrush(Qt::white), 1);

    QBrush blackBrush(Qt::black);

    int cornerMargin = 25; //pixels to not draw rulers in the window corners so they don't crash into each other.
    //Horizontal ruler
    double xImg = 0.5, xDraw, oldxImg;
    while (1)
    {
        xDraw = mapFromScene(xImg, 0).x(); //convert to viewport coordinates.
        while (xDraw < cornerMargin)
        {
            // If we're within the margin, go to the first image location just outside it.
            int xDrawOld = xDraw;
            while (xDraw == xDrawOld) {
                // This loop handles images that are zoomed out below 1:1.
                xImg += 1;
                xDraw = mapFromScene(xImg, 0).x();
            }
        }
        if (xDraw > viewport()->width() - 1 - cornerMargin || xImg > getImageWidth()) break; //reached the other end.

        //If we get here, draw our ruler tick and label it.
        p.setBrush(blackBrush);
        p.setPen(blackPen);
        p.drawLine(xDraw, r.height() - 1, xDraw, r.height() - 1 - tickLen);
        p.drawLine(xDraw, 0, xDraw, tickLen);
        p.setPen(whitePen);
        p.drawLine(xDraw, r.height() - 1, xDraw, r.height() - 1 - tickLen);
        p.drawLine(xDraw, 0, xDraw, tickLen);

        int xCoord = (int)floor(xImg);
        if (!imgIsZeroIndexed) xCoord++;
        QString coord = QString::asprintf("%d", xCoord);
        QRect coordBox(xDraw + 2, tickLen - fm.height() + 2, fm.horizontalAdvance(coord), fm.height());
        p.fillRect(coordBox, blackBrush);
        p.drawText(coordBox, coord);
        coordBox = QRect(xDraw + 2, r.height() - 1 - tickLen - 1, fm.horizontalAdvance(coord), fm.height());
        p.fillRect(coordBox, blackBrush);
        p.drawText(coordBox, coord);

        //Remember this as the previous image coordinate.
        oldxImg = xImg;

        xDraw += (double)minTickSpacingpix; //increment to the next tick mark, and calculate new image coordinate.
        xImg = floor(mapToScene(xDraw, 0).x()) + 0.5; //the 0.5 is to be at the middle of zoomed-in pixels.
        if (xImg == oldxImg) //can happen if we're zoomed in too much; increment xImg manually.
            xImg += 1;
        if (xImg > getImageWidth()) //done with image.
            break;
    }

    //Vertical ruler
    double yImg = 0.5, yDraw, oldyImg, ySign = 1;
    if (imgYOriginIsBottom)
    {
        yImg = getImageHeight() - 0.5;
        ySign = -1;
    }
    while (1)
    {
        yDraw = mapFromScene(0, yImg).y(); //convert to viewport coordinates.
        if (!imgYOriginIsBottom)
        {
            // Start drawing the ruler at the top.
            while (yDraw < cornerMargin)
            {
                int yDrawOld = yDraw;
                while (yDraw == yDrawOld) {
                    yImg += 1;
                    yDraw = mapFromScene(0, yImg).y();
                }
            }
            if (yDraw > viewport()->height() - 1 - cornerMargin || yImg > getImageHeight()) break; //reached the other end.
        }
        else
        {
            // Since we're starting at the bottom, these conditions have to change.
            while (yDraw > viewport()->height() - 1 - cornerMargin)
            {
                int yDrawOld = yDraw;
                while (yDraw == yDrawOld) {
                    yImg -= 1;
                    yDraw = mapFromScene(0, yImg).y();
                }
            }
            if (yDraw < cornerMargin || yImg < 0) break; //reached the other end.
        }

        //If we get here, draw our ruler tick and label it.
        p.setBrush(blackBrush);
        p.setPen(blackPen);
        p.drawLine(r.width() - 1, yDraw, r.width() - 1 - tickLen, yDraw);
        p.drawLine(0, yDraw, tickLen, yDraw);
        p.setPen(whitePen);
        p.drawLine(r.width() - 1, yDraw, r.width() - 1 - tickLen, yDraw);
        p.drawLine(0, yDraw, tickLen, yDraw);

        int yCoord = (int)floor(yImg);
        if (!imgIsZeroIndexed)
            if (imgYOriginIsBottom)
                yCoord = getImageHeight() - yCoord;
            else
                yCoord++;
        else
            if (imgYOriginIsBottom)
                yCoord = getImageHeight() - 1 - yCoord;

        QString coord = QString::asprintf("%d", yCoord);
        QRect coordBox(0, yDraw + 2, fm.horizontalAdvance(coord) + 1, fm.height());
        p.fillRect(coordBox, blackBrush);
        coordBox.setX(coordBox.x() + 1);
        p.drawText(coordBox, coord);
        coordBox = QRect(r.width() - 1 - fm.horizontalAdvance(coord), yDraw + 2, fm.horizontalAdvance(coord) + 1, fm.height());
        p.fillRect(coordBox, blackBrush);
        coordBox.setX(coordBox.x() + 1);
        p.drawText(coordBox, coord);

        //Remember this as the previous image coordinate.
        oldyImg = yImg;

        yDraw += ySign * (double)minTickSpacingpix; //increment to the next tick mark, and calculate new image coordinate.
        yImg = floor(mapToScene(0, yDraw).y()) + 0.5; //the 0.5 is to be at the middle of zoomed-in pixels.
        if (yImg == oldyImg) //can happen if we're zoomed in too much; increment xImg manually.
            yImg += ySign * 1;
        if (yImg > getImageHeight() || yImg < 0) //done with image.
            break;
    }
}

void ImageWindow::drawColorbar()
{
    if (!showColorbar)
        return;

    QString title;
    title = getColormapName();
    QString dipFactorString = QString::asprintf("%0.03f", dipFactor);
    if (getImageFunction() == ImageWindowFunction::Log10DarkenLight)
        title.append(" log darken (" + dipFactorString + ")");
    else if (getImageFunction() == ImageWindowFunction::Log10BrightenDark)
        title.append(" log brighten (" + dipFactorString + ")");
    else if (getImageFunction() == ImageWindowFunction::DarkenLight)
        title.append(" parabolic darken (" + dipFactorString + ")");
    else if (getImageFunction() == ImageWindowFunction::BrightenDark)
        title.append(" parabolic brighten (" + dipFactorString + ")");
    if (getScaleMode() == ImageWindowScaling::Fit)
        title.append(" fit");

    QString minTxt = QString::asprintf("%.1f", scaleMin);

    QString maxTxt = QString::asprintf("%.1f", scaleMax);

    QFontMetrics fm(windowFont);
    int textH = fm.height();
    int barWidth = 255;
    while (fm.horizontalAdvance(title) + fm.horizontalAdvance(minTxt) + fm.horizontalAdvance(maxTxt) + 50 > barWidth)
        barWidth += 256;
    int barHeight = 10;

    QRect colorbarRect(0, 0, barWidth + 2 * boxMargin, barHeight + textH + 5 + 2 * boxMargin);

    if (colorbarImage.isNull())
        translationIsFresh = true;

    if (translationIsFresh)
    {
        colorbarImage = QImage(colorbarRect.width(), colorbarRect.height(), QImage::Format_ARGB32);

        QPainter p(&colorbarImage);
        p.setFont(windowFont);
        p.setBrush(Qt::black);
        p.drawRect(colorbarRect);

        double drawMin = applyImageFunction(scaleMin);
        double drawMax = applyImageFunction(scaleMax);

        for (int x = 0; x < barWidth; x++)
        {
            double val;
            if (x == 0)
                val = drawMin;
            else if (x == barWidth)
                val = drawMax;
            else
                val = applyImageFunction(x / (double)(barWidth - 1) * (scaleMax - scaleMin) + scaleMin);

            double srcPixel = (val - offset) * scale;

            for (int y = 0; y < barHeight; y++)
            {
                const uchar *dst = colorbarImage.constScanLine(y + boxMargin + textH + 5) + (x + boxMargin) * 4;
                colormap->translatePixel((uchar *)dst, srcPixel);
             }
        }

        p.setPen(Qt::white);

        p.drawText(boxMargin, boxMargin + textH, minTxt);
        p.drawText(colorbarImage.width() - boxMargin - fm.horizontalAdvance(maxTxt), boxMargin + textH, maxTxt);

        p.drawText(colorbarImage.width() / 2 - fm.horizontalAdvance(title) / 2, boxMargin + textH, title);

        translationIsFresh = false;
    }

    QPainter p(viewport());

    QRect winRect = geometry();

    QRectF target;
    target.setLeft  (winRect.width()  - boxPad - colorbarRect.width() );
    target.setTop   (winRect.height() - boxPad - colorbarRect.height());
    target.setWidth (colorbarRect.width() );
    target.setHeight(colorbarRect.height());

    QRectF source(colorbarRect);

    p.drawImage(target, colorbarImage, source);
}

void ImageWindow::drawROI()
{
    // There's a bug in drawing the ROI at what appear to be multiples of 8 in zoom level.
    // I can't see how it's me; must be in Qt.
    if (!ROIisValid)
     return;

    int xSign, ySign;
    //Calculate the ROI rectangle.
    if (ROI1.x() < ROI2.x())
    {
        ROI.setX(ROI1.x());
        ROI.setWidth((ROI2 - ROI1).x());
        xSign = +1;
    }
    else
    {
        ROI.setX(ROI2.x());
        ROI.setWidth((ROI1 - ROI2).x());
        xSign = -1;
    }

    if (ROI1.y() < ROI2.y())
    {
        ROI.setY(ROI1.y());
        ROI.setHeight((ROI2 - ROI1).y());
        ySign = +1;
    }
    else
    {
        ROI.setY(ROI2.y());
        ROI.setHeight((ROI1 - ROI2).y());
        ySign = -1;
    }

    //Draw it on-screen.
    QPen blackPen(QBrush(Qt::black), 3);
    QPen whitePen(QBrush(Qt::white), 1);

    QPainter p(viewport());

    //Draw the diagonal.
    QPoint ROI1Screen, ROI2Screen;
    ROI1Screen = mapFromScene(ROI1) + QPoint(xSign, ySign);
    ROI2Screen = mapFromScene(ROI2) - QPoint(xSign, ySign);
    p.setPen(blackPen);
    p.drawLine(ROI1Screen, ROI2Screen);
    p.setPen(whitePen);
    p.drawLine(ROI1Screen, ROI2Screen);

    //Now draw the rectangle.
    QRect ROIScreen;
    ROIScreen.setTopLeft(ROI1Screen);
    ROIScreen.setBottomRight(ROI2Screen);

    p.setPen(blackPen);
    p.drawRect(ROIScreen);
    p.setPen(whitePen);
    p.drawRect(ROIScreen);

    QBrush blackBrush(Qt::black);
    p.setFont(windowFont);

    QString coords = QString::asprintf("dx = %d, dy = %d -> h = %.2f", ROI.width(), ROI.height(),
                                       sqrt((double)(ROI.width() * ROI.width() + ROI.height() * ROI.height())));
    QFontMetrics fm(windowFont);
    QRect coordBox((ROI1Screen.x() + ROI2Screen.x() - fm.horizontalAdvance(coords)) / 2,
                (ROI1Screen.y() + ROI2Screen.y() - fm.height()) / 2,
                fm.horizontalAdvance(coords) + 1, fm.height());
    p.fillRect(coordBox, blackBrush);
    coordBox.setX(coordBox.x() + 1);
    p.drawText(coordBox, coords);
}

#include <QApplication>
#include <QScreen>
void ImageWindow::snapWindowTo(snapType snap, QScreen * screen)
{
    int halfBorder = (frameGeometry().width()  - geometry().width() )/2; // thickness of window border.

    //This does not directly support vertical screen layout, i.e. one monitor above another.

    //Note - the way this gets the number of screens and screen dimensions is deprecated in QT so it
    //will look like these function calls are all wrong if you look at the docs.

    // QT can't correctly get the geometry of Ubuntu's Unity desktop (16.04), so we just pad it.
    int buggyMargin = 10;
    halfBorder += buggyMargin;

    //We need to establish on our own the screen layout for left-to-right, because Qt doesn't
    //seem to handle that very well.
    QRect screenDims;
    QList<QScreen *> screens = QGuiApplication::screens();
    int numScreens = screens.size();
    QScreen * screensL2R[numScreens];
    memset(screensL2R, 0, numScreens * sizeof(QScreen *));
    for (int j = 0; j < numScreens; j++)
    {
        int curMin = std::numeric_limits<int>::max();
        for (int i = 0; i < numScreens; i++)
        {
            bool skip = false;
            for (int k = 0; k < j; k++)
                if (screens[i] == screensL2R[k])
                {
                    skip = true;
                    break;
                }
            if (skip) continue; //screen already accounted for.
            if (screens[i]->geometry().left() < curMin)
            {
                screensL2R[j] = screens[i];
                curMin = screens[i]->geometry().left();
            }
        }
    }

    switch(snap)
    {
    case Left:
        //First, check if we're already snapped "left", e.g. the user is trying
        //to snap "right" on a monitor that is to the left.
        if (currentSnap == Left)
        {
            screen = QGuiApplication::screenAt(geometry().topLeft());
            //Find out if there's a screen to the left of this one.
            int i;
            for (i = 0; i < numScreens; i++)
                if (screen == screensL2R[i]) break;
            if (i == 0)
                break; //do nothing; no more monitors available.
            snapWindowTo(Right, screensL2R[i-1]); //snap right on the monitor to the left.
            break;
        }
        currentSnap = Left;
        if (screen == 0)
            screen = QGuiApplication::screenAt(geometry().topLeft());
        screenDims = screen->geometry();
        move(screenDims.left(), screenDims.top());
        resize(screenDims.width()/2 - 2*halfBorder, screenDims.height() - 2*halfBorder);
        break;
    case TopLeft:
        if (currentSnap == TopLeft)
        {
            screen = QGuiApplication::screenAt(geometry().topLeft());
            int i;
            for (i = 0; i < numScreens; i++)
                if (screen == screensL2R[i]) break;
            if (i == 0)
                break;
            snapWindowTo(TopRight, screensL2R[i-1]);
            break;
        }
        currentSnap = TopLeft;
        if (screen == 0)
            screen = QGuiApplication::screenAt(geometry().topLeft());
        screenDims = screen->geometry();
        move(screenDims.left(), screenDims.top());
        resize(screenDims.width()/2 - 2*halfBorder, screenDims.height()/2 - 2*halfBorder);
        break;
    case BottomLeft:
        if (currentSnap == BottomLeft)
        {
            screen = QGuiApplication::screenAt(geometry().topLeft());
            int i;
            for (i = 0; i < numScreens; i++)
                if (screen == screensL2R[i]) break;
            if (i == 0)
                break;
            snapWindowTo(BottomRight, screensL2R[i-1]);
            break;
        }
        currentSnap = BottomLeft;
        if (screen == 0)
            screen = QGuiApplication::screenAt(geometry().topLeft());
        screenDims = screen->geometry();
        move(screenDims.left(), screenDims.height()/2 + screenDims.top());
        resize(screenDims.width()/2 - 2*halfBorder, screenDims.height()/2 - 2*halfBorder);
        break;
    case Right:
        if (currentSnap == Right)
        {
            screen = QGuiApplication::screenAt(geometry().topLeft());
            //Find out if there's a screen to the right of this one.
            int i;
            for (i = 0; i < numScreens; i++)
                if (screen == screensL2R[i]) break;
            if (i == numScreens - 1)
                break; //do nothing; no more monitors available.
            snapWindowTo(Left, screensL2R[i+1]); //snap right on the monitor to the left.
            break;
        }
        currentSnap = Right;
        if (screen == 0)
            screen = QGuiApplication::screenAt(geometry().topRight());
        screenDims = screen->geometry();
        move(screenDims.width()/2 + screenDims.left(), screenDims.top());
        resize(screenDims.width()/2 - 2*halfBorder, screenDims.height() - 2*halfBorder);
        break;
    case TopRight:
        if (currentSnap == TopRight)
        {
            screen = QGuiApplication::screenAt(geometry().topLeft());
            int i;
            for (i = 0; i < numScreens; i++)
                if (screen == screensL2R[i]) break;
            if (i == numScreens - 1)
                break;
            snapWindowTo(TopLeft, screensL2R[i+1]);
            break;
        }
        currentSnap = TopRight;
        if (screen == 0)
            screen = QGuiApplication::screenAt(geometry().topRight());
        screenDims = screen->geometry();
        move(screenDims.width()/2 + screenDims.left(), screenDims.top());
        resize(screenDims.width()/2 - 2*halfBorder, screenDims.height()/2 - 2*halfBorder);
        break;
    case BottomRight:
        if (currentSnap == BottomRight)
        {
            screen = QGuiApplication::screenAt(geometry().topLeft());
            int i;
            for (i = 0; i < numScreens; i++)
                if (screen == screensL2R[i]) break;
            if (i == numScreens - 1)
                break;
            snapWindowTo(BottomLeft, screensL2R[i+1]);
            break;
        }
        currentSnap = BottomRight;
        if (screen == 0)
            screen = QGuiApplication::screenAt(geometry().topRight());
        screenDims = screen->geometry();
        move(screenDims.width()/2 + screenDims.left(), screenDims.height()/2 + screenDims.top());
        resize(screenDims.width()/2 - 2*halfBorder, screenDims.height()/2 - 2*halfBorder);
        break;
    case Max:
        currentSnap = Max;
        if (screen == 0)
            screen = QGuiApplication::screenAt(QPoint((geometry().left() + geometry().right() )/2,
                                                      (geometry().top()  + geometry().bottom())/2));
        screenDims = screen->geometry();
        move(screenDims.left(), screenDims.top());
        resize(screenDims.width() - 2*halfBorder, screenDims.height() - 2*halfBorder);
        break;
    case Restore:
        currentSnap = Restore;
        if (screen == 0)
            screen = QGuiApplication::screenAt(QPoint((geometry().left() + geometry().right() )/2,
                                                      (geometry().top()  + geometry().bottom())/2));
        screenDims = screen->geometry();
        move(screenDims.left() + screenDims.width() / 4, screenDims.top() + screenDims.height() / 4);
        resize(screenDims.width() / 2 - 2*halfBorder, screenDims.height() / 2 - 2*halfBorder);
        break;
    case None:
        break;
    }
}

#include <QApplication>
#include <QClipboard>
void sendToClipboard(QImage &image)
{
    QClipboard *clipboard = QApplication::clipboard();
    if (clipboard == nullptr)
    {
        cerr << "Clibpard is null!" << endl;
        return;
    }
    clipboard->setImage(image, QClipboard::Clipboard);
    cout << "Copied image measuring " << image.width() << " x " << image.height() << " to clipboard." << endl;
}

void ImageWindow::copyImageToClipboard()
{
    if (!OKToTranslate)
    {
        cerr << "Image is not OK to translate!" << endl;
        return;
    }
    sendToClipboard(translatedImage);
}

QString insertBeforeExtension(QString originalPath, QString suffixToInsert)
{
    QString filename, extension;
    QFileInfo fi(originalPath);
    filename = QDir(fi.absoluteDir()).filePath(fi.baseName());
    extension = fi.completeSuffix();

    filename += suffixToInsert;
    if (extension.length() > 0)
    {
        filename +=  ".";
        filename += extension;
    }

    return filename;
}
void ImageWindow::saveImageToFile()
{
    if (!OKToTranslate)
    {
        cerr << "Image is not OK to translate!" << endl;
        return;
    }

    // Append some translation info into the filename.
    QString suffix;
    if (!getTranslationParamsString(suffix))
        return;

    QString filename;
    // force PNG
    QFileInfo fi(curFilename);
    filename = insertBeforeExtension(fi.absoluteDir().filePath(fi.baseName()), suffix);
    filename +=  ".png";
    if(translatedImage.save(filename))
        cout << "Saved translated image to " << filename.toStdString() << endl;
    else
        cerr << "Failed to save translated image to " << filename.toStdString() << endl;


}

#include <QScreen>
#include <QWindow>
#include <QPixmap>
#include <QSysInfo>
bool ImageWindow::takeScreenshot(QImage &screenshot)
{
    if (!OKToDraw)
    {
        cerr << "Window is not OK to draw!" << endl;
        return false;
    }
    // Mostly from here: https://doc.qt.io/qt-5/qtwidgets-desktop-screenshot-example.html
    const QWindow *window = windowHandle();
    if (window == nullptr)
    {
        cerr << "Window is null!" << endl;
        return false;
    }

    QScreen *screen = window->screen();
    if (screen == nullptr)
    {
        cerr << "Screen is null!" << endl;
        return false;
    }

    int x = 0, y = 0, w = -1, h = -1;
    QString os = QSysInfo::productType();
    cout << "Taking a screenshot on " << os.toStdString() << endl;
    if (os == "macos" || os == "osx")
    {
        // For mac, we have to define the coordinates of the screenshot.
        x = this->geometry().x();
        y = this->geometry().y();
        w = this->geometry().width();
        h = this->geometry().height();
        cout << "OS X screenshot at " << x << ", " << y << " measuring " << w << " x " << h << "." << endl;
    }

    QPixmap pixmap = screen->grabWindow(window->winId(), x, y, w, h);
    screenshot = pixmap.toImage();
    return true;
}

void ImageWindow::copyScreenshotToClipboard()
{
    QImage image;
    if (!takeScreenshot(image))
        return;

    sendToClipboard(image);
}

#include <QDateTime>
bool ImageWindow::pasteFromClipboard()
{
    QClipboard *clipboard = QApplication::clipboard();
    if (clipboard == nullptr)
    {
        cerr << "Clibpard is null!" << endl;
        return false;
    }
    QImage cbImage = clipboard->image();
    if (cbImage.isNull())
    {
        cerr << "Clibpard does not contain an image!" << endl;
        return false;
    }
    QString filename = QDir::tempPath() + "/temp_" + QString::number(QDateTime::currentMSecsSinceEpoch()) + ".tif";
    if(cbImage.save(filename))
    {
        cout << "Saved clipboard image to " << filename.toStdString() << endl;
        return this->readImage(filename);
    }
    cerr << "Failed to save clipboard image to " << filename.toStdString() << endl;
    return false;
}

void ImageWindow::saveScreenshotToFile()
{
    QImage image;
    if (!takeScreenshot(image))
        return;

    QString suffix = QString("_screenshot_") + QString::number(QDateTime::currentMSecsSinceEpoch()) + QString(".png");
    QString filename = curFilename + suffix;
    if(image.save(filename))
        cout << "Saved screenshot image to " << filename.toStdString() << endl;
    else
        cerr << "Failed to save screenshot image to " << filename.toStdString() << endl;

}

void ImageWindow::handleKeyPress(QKeyEvent *event, bool forwarded)
{
    Qt::KeyboardModifiers mods = QApplication::keyboardModifiers();

//    cout << std::hex;
//    cout << "Key press: 0x" << event->key() << " with QApplication::keyboardModifiers() = 0x" << mods << " and event->modifiers = 0x" << event->modifiers() << endl;
//    cout << std::dec;

    auto sourceImage = sourceImageBuffer[currentImageBufferIndex];

    switch(event->key())
    {
        // The digits 0-9 are used for sorting images into "bucket folders".
        // We let all these cases continue on to the end, where they're handled
        // in the case statement for Key_2.
        case Qt::Key_3:
        {
            if (mods == Qt::ControlModifier) {
                showCentered();
                break;
            }
        }
        case Qt::Key_4:
        {
            if (mods == Qt::ControlModifier) {
                showCorner(ImageCorner::TopLeft);
                break;
            }
        }
            case Qt::Key_5:
        {
            if (mods == Qt::ControlModifier) {
                showCorner(ImageCorner::TopRight);
                break;
            }
        }
        case Qt::Key_6:
        {
            if (mods == Qt::ControlModifier) {
                showCorner(ImageCorner::BottomLeft);
                break;
            }
        }
        case Qt::Key_7:
        {
            if (mods == Qt::ControlModifier) {
                showCorner(ImageCorner::BottomRight);
                break;
            }
        }
        case Qt::Key_8:
        case Qt::Key_9:
        case Qt::Key_0:
        {
            if (mods == Qt::NoModifier) {
                imgIsZeroIndexed = !imgIsZeroIndexed;
                drawRulers();
                viewport()->update();
                break;
            }
        }
        // For 1 and 2, we also have to check the key in the case statement, because
        // the numeric keys above "trickle down" into these two to avoid putting the bucket
        // code into multiple cases.
        case Qt::Key_1:
        {
            if (event->key() == Qt::Key_1 && mods == Qt::ControlModifier) {
                zoomFit();
                break;
            }
        }
        case Qt::Key_2:
        {
            if (event->key() == Qt::Key_2 && mods == Qt::ControlModifier) {
                zoom1To1();
                break;
            }

            if (mods == Qt::MetaModifier || mods ==Qt::AltModifier) {
                int bucket = static_cast<int>(event->key()) - 0x30;
                copyCurrentFileToBucket(bucket);
            }
            break;
        }
        case Qt::Key_M:
        {
            if (mods == Qt::NoModifier && !forwarded) {
                if (parent != nullptr) {
                    parent->raise();
                }
            }
            break;
        }
        case Qt::Key_Left:
        {
            if (!forwarded)
            {
                if (mods == Qt::ControlModifier ||
                    mods == (Qt::ControlModifier | Qt::KeypadModifier))
                {
                    snapWindowTo(Left);
                    break;
                }
                if (mods == (Qt::ControlModifier | Qt::ShiftModifier) ||
                    mods == (Qt::ControlModifier | Qt::ShiftModifier | Qt::KeypadModifier))
                {
                    snapWindowTo(TopLeft);
                    break;
                }
                if (mods == (Qt::AltModifier | Qt::ShiftModifier) ||
                    mods == (Qt::AltModifier | Qt::ShiftModifier | Qt::KeypadModifier))
                {
                    snapWindowTo(BottomLeft);
                    break;
                }
                if (mods == Qt::NoModifier ||
                    mods == Qt::KeypadModifier)
                {
                    readPrevImage();
                    break;
                }
            }
            if (mods == Qt::NoModifier || mods == Qt::KeypadModifier)
                readPrevImage();
            break;
        }
        case Qt::Key_Right:
        {
            if (!forwarded)
            {
                if (mods == Qt::ControlModifier ||
                    mods == (Qt::ControlModifier | Qt::KeypadModifier))
                {
                    snapWindowTo(Right);
                    break;
                }
                if (mods == (Qt::ControlModifier | Qt::ShiftModifier) ||
                    mods == (Qt::ControlModifier | Qt::ShiftModifier | Qt::KeypadModifier))
                {
                    snapWindowTo(TopRight);
                    break;
                }
                if (mods == (Qt::AltModifier | Qt::ShiftModifier) ||
                    mods == (Qt::AltModifier | Qt::ShiftModifier | Qt::KeypadModifier))
                {
                    snapWindowTo(BottomRight);
                    break;
                }
                if (mods == Qt::NoModifier || mods == Qt::KeypadModifier)
                {
                    readNextImage();
                    break;
                }
            }
            if (mods == Qt::NoModifier || mods == Qt::KeypadModifier)
                readNextImage();
            break;
        }
        case Qt::Key_Up:
        {
            if (!forwarded)
                if (mods == Qt::ControlModifier || mods == (Qt::ControlModifier | Qt::KeypadModifier))
                    snapWindowTo(Max);
            break;
        }
        case Qt::Key_Down:
        {
            if (!forwarded)
                if (mods == Qt::ControlModifier || mods == (Qt::ControlModifier | Qt::KeypadModifier))
                    snapWindowTo(Restore);
            break;
        }
        case Qt::Key_F5:
        {
            if (mods == Qt::NoModifier && !forwarded)
                readImage(curFilename);
            break;
        }
        case Qt::Key_Delete:
        {
            if (mods != Qt::NoModifier) break;

            if (forwarded)
                break;
            deleteCurrentFile();
            break;
        }
        case Qt::Key_I:
        {
            if (mods != Qt::NoModifier) break;

            showInfoBox = !showInfoBox;
            viewport()->update();
            break;
        }
        case Qt::Key_Space:
        {
            if (mods != Qt::NoModifier) break;

            showCursorInfoBox = !showCursorInfoBox;
            viewport()->update();
            break;
        }
        case Qt::Key_C:
        {
            if (mods == Qt::ControlModifier && !forwarded)
            {
                copyImageToClipboard();
                break;
            }
            if (mods == (Qt::ControlModifier | Qt::AltModifier) && !forwarded)
            {
                saveImageToFile();
                break;
            }
            if (mods == (Qt::ControlModifier | Qt::ShiftModifier) && !forwarded)
            {
                copyScreenshotToClipboard();
                break;
            }
            if (mods == (Qt::ControlModifier | Qt::ShiftModifier | Qt::AltModifier) && !forwarded)
            {
                saveScreenshotToFile();
                break;
            }

            if (mods != Qt::NoModifier) break;

            showColorbar = !showColorbar;
            viewport()->update();
            break;
        }
        case Qt::Key_X:
        {
            if (mods != Qt::NoModifier) break;

            showRulers = !showRulers;
            viewport()->update();
            break;
        }
        case Qt::Key_V:
        {
            if (!forwarded && mods == Qt::ControlModifier)
            {
                pasteFromClipboard();
                break;
            }

            int map = (int)getColormap();
            if (mods == Qt::NoModifier)
                map--;
            else if (mods == Qt::ShiftModifier)
                map++;
            if (map == (int)ColormapPalette::NUM_PALETTES)
                map = 0;
            else if (map == -1)
                map = (int)ColormapPalette::NUM_PALETTES - 1;

            setColormap((ColormapPalette)map);
            break;
        }
        case Qt::Key_Equal:
        {
            if (mods != Qt::NoModifier) break;

            setDipFactor(dipFactor * 1.25);
            break;
        }
        case Qt::Key_Minus:
        {
            if (mods != Qt::NoModifier) break;

            setDipFactor(dipFactor / 1.25);
            break;
        }
        case Qt::Key_F:
        {   
            int wf = (int)getImageFunction();
            if (mods == Qt::NoModifier)
                wf--;
            else if (mods == Qt::ShiftModifier)
                wf++;
            if (wf == (int)ImageWindowFunction::NUM_WINDOFUNCTIONS)
                wf = 0;
            else if (wf == -1)
                wf = (int)ImageWindowFunction::NUM_WINDOFUNCTIONS - 1;

            setImageFunction((ImageWindowFunction)wf);
            break;
        }
        case Qt::Key_S:
        {
            if (mods != Qt::NoModifier) break;

            ImageWindowScaling s = getScaleMode();
            if (s == ImageWindowScaling::Fit)
            {
                holdAll();
                setUserMin(0);
                setUserMax(pow(2, sourceImage->getBPP()) - 1);
                setScaleMode(ImageWindowScaling::User);
                releaseAll();
            }
            else
                setScaleMode(ImageWindowScaling::Fit);

            break;
        }
        case Qt::Key_F4:
        {
            if (mods == Qt::ControlModifier) {
                this->close();
                break;
            }
            break;
        }
        case Qt::Key_W:
        {
            if (forwarded)
                break;

            if (mods == Qt::ControlModifier) {
                this->close();
                break;
            }

            if (mods == Qt::ShiftModifier && sourceImage != nullptr && !forwarded)
            {
                sourceImage->resetWhiteBalance();
                break;
            }

            if (mods == Qt::NoModifier && ROIisValid && sourceImage != nullptr && !forwarded)
                sourceImage->doWhiteBalance(ROI);
            break;
        }
        case Qt::Key_A:
        {
            int angle = (int)getImageRotation();
            if (mods == Qt::ShiftModifier)
                angle--;
            else if (mods == Qt::NoModifier)
                angle++;

            if (angle > (int)ImageWindowRotation::CCW270)
                angle = 0;
            else if (angle == -1)
                angle = (int)ImageWindowRotation::CCW270;

            setImageRotation((ImageWindowRotation)angle);

            break;
        }
        case Qt::Key_L:
        {
            if (mods != Qt::NoModifier) break;
            bool h = !getImageFlipHorizontal();
            bool v = getImageFlipVertical();
            setImageFlip(h, v);
            break;
        }
        case Qt::Key_T:
        {
            if (mods != Qt::NoModifier) break;
            bool h = getImageFlipHorizontal();
            bool v = !getImageFlipVertical();
            setImageFlip(h, v);
            break;
        }
        case Qt::Key_R:
        {
            if (mods == Qt::ControlModifier && !forwarded)
            {
                readImage(curFilename);
                break;
            }

            if (mods == Qt::ShiftModifier)
                activeChannels = chanR;
            else if (mods == Qt::NoModifier)
                activeChannels ^= chanR;
            translateImage();
            break;
        }
        case Qt::Key_G:
        {
            if (mods == Qt::ShiftModifier)
                activeChannels = chanG;
            else if (mods == Qt::NoModifier)
                activeChannels ^= chanG;
            translateImage();
            break;
        }
        case Qt::Key_B:
        {
            if (mods == Qt::ShiftModifier)
                activeChannels = chanB;
            else if (mods == Qt::NoModifier)
                activeChannels ^= chanB;
            translateImage();
            break;
        }
        case Qt::Key_Y:
        {
            if (mods != Qt::NoModifier) break;

            imgYOriginIsBottom = !imgYOriginIsBottom;
            viewport()->update();
            break;
        }
        case Qt::Key_Shift:
        case Qt::Key_Control:
        case Qt::Key_Alt:
        case Qt::Key_Meta: // control key on Mac
            break; //don't show help for all the aux keys by themselves
        default:
        {
            cout << "Default key press case." << endl;
            showHelp = true;
            viewport()->update();
            QGraphicsView::keyPressEvent(event);
        }
    }
}

void ImageWindow::keyPressEvent(QKeyEvent *event)
{
    emit signalKeyPressed(event, this->ID);

    handleKeyPress(event, false);
}

void ImageWindow::handleKeyRelease(QKeyEvent *event, bool /*forwarded*/)
{
    if(showHelp)
    {
        showHelp = false;
        viewport()->update();
    }

    QGraphicsView::keyReleaseEvent(event);
}

void ImageWindow::keyReleaseEvent(QKeyEvent *event)
{
    emit signalKeyReleased(event, this->ID);

    handleKeyRelease(event, false);
}

void ImageWindow::drawHelp()
{
    if (!showHelp)
        return;

    QPainter p(viewport());
    QBrush blackBrush(Qt::black);
    p.setFont(windowFont);
    QFontMetrics fm(windowFont);
    QRect r = viewport()->geometry();
    QString menu = "";

    menu.append("pxlpeep version ");
    menu.append(VERSION_STRING);
    int width = fm.horizontalAdvance("Left button                        select ROI"); // example line
    menu.append("\n");
    int numLines = 1;
    menu.append("\n");                                   numLines++;
    menu.append("--Mouse--\n");                          numLines++;
    menu.append("Left button                               pan\n"); numLines++;
    menu.append("SHFIT+Left button                  select ROI\n"); numLines++;
    menu.append("Right button                      move window\n"); numLines++;
    menu.append("Wheel                                    zoom\n"); numLines++;
    menu.append("\n");                                   numLines++;
    menu.append("--Keyboard--\n");                       numLines++;
    menu.append("CTRL+up arrow                        maximize\n"); numLines++;
    menu.append("CTRL+rt arrow                  1/2-size right\n"); numLines++;
    menu.append("CTRL+SHIFT+rt arrow        1/4-size top right\n"); numLines++;
    menu.append("ALT+SHIFT+rt arrow      1/4-size bottom right\n"); numLines++;
    menu.append("CTRL+lt arrow                   1/2-size left\n"); numLines++;
    menu.append("CTRL+SHIFT+lt arrow         1/4-size top left\n"); numLines++;
    menu.append("ALT+SHIFT+lt arrow       1/4-size bottom left\n"); numLines++;
    menu.append("CTRL+dn arrow               1/4-size centered\n"); numLines++;
    menu.append("CTRL+W (or F4)                   close window\n"); numLines++;
    menu.append("M                           raise main window\n"); numLines++;
    menu.append("\n"); numLines++;
    menu.append("CTRL+V             paste image from clibpoard\n"); numLines++;
    menu.append("CTRL+C                copy image to clibpoard\n"); numLines++;
    menu.append("CTRL+SHIFT+C     copy screenshot to clibpoard\n"); numLines++;
    menu.append("CTRL+ALT+C                 save image to file\n"); numLines++;
    menu.append("CTRL+ALT+SHIFT+C      save screenshot to file\n"); numLines++;
    menu.append("\n"); numLines++;
    menu.append("lt arrow                       previous image\n"); numLines++;
    menu.append("rt arrow                           next image\n"); numLines++;
    menu.append("F5 or CTRL+R                     reload image\n"); numLines++;
    menu.append("Del                    delete image from disk\n"); numLines++;
    menu.append("ALT+[0-9]          move image to bucket [0-9]\n"); numLines++;
    menu.append("\n"); numLines++;
    menu.append("CTRL+1                            zoom to fit\n"); numLines++;
    menu.append("CTRL+2                               zoom 1:1\n"); numLines++;
    menu.append("CTRL+3-7     center / corners at current zoom\n"); numLines++;
    menu.append("\n"); numLines++;
    menu.append("I                             toggle info box\n"); numLines++;
    menu.append("space                  toggle cursor info box\n"); numLines++;
    menu.append("C                            toggle color bar\n"); numLines++;
    menu.append("X                               toggle rulers\n"); numLines++;
    menu.append("\n"); numLines++;
    menu.append("V / SHIFT+V                   cycle colormaps\n"); numLines++;
    menu.append("F / SHIFT+F       toggle pixel value function\n"); numLines++;
    menu.append("= / -    alter dip factor for brighten/darken\n"); numLines++;
    menu.append("S                    toggle pixel value scale\n"); numLines++;
    menu.append("W                   compute ROI white balance\n"); numLines++;
    menu.append("SHIFT+W                   reset white balance\n"); numLines++;
    menu.append("A / SHIFT+A               rotate image 90 deg\n"); numLines++;
    menu.append("L                      toggle flip horizontal\n"); numLines++;
    menu.append("T                        toggle flip vertical\n"); numLines++;
    menu.append("R                          toggle red channel\n"); numLines++;
    menu.append("SHIFT+R                      red channel only\n"); numLines++;
    menu.append("G                        toggle greem channel\n"); numLines++;
    menu.append("SHIFT+G                    green channel only\n"); numLines++;
    menu.append("B                         toggle blue channel\n"); numLines++;
    menu.append("SHIFT+B                     blue channel only\n"); numLines++;
    menu.append("0                        start is zero or one\n"); numLines++;
    menu.append("Y                  origin is at top or bottom\n"); numLines++;

    QRect menuRect(r.width() - 1 - (width + 2 * boxMargin) - boxPad,
                   infoUpdateRegion.top() + infoUpdateRegion.height() + boxPad,
                   width + 2 * boxMargin,
                   fm.height() * numLines + 2 * boxMargin);

    p.fillRect(menuRect, blackBrush);
    QPen whitePen(QBrush(Qt::white), 1);
    p.setPen(whitePen);
    p.drawText(menuRect, Qt::AlignCenter | Qt::AlignVCenter, menu);
}

void ImageWindow::syncWithFolder()
{
    if (curFilename == "") return;

    // This is guaranteed to work once we've loaded an image.
    QStringList chunks = curFilename.split(".");
    QString extension = chunks[chunks.count() - 1];

    QDir dir(curDirectory);
    dir.setFilter(QDir::Files);
    dir.setSorting(QDir::NoSort);
    dir.setNameFilters(QStringList() << ("*." + extension));

    // How to sort numbers correctly:
    // https://stackoverflow.com/a/36018397/149506
    fileList = dir.entryList();
    QCollator collator;
    collator.setNumericMode(true);
    std::sort(fileList.begin(), fileList.end(), collator);

    fileListPos = -1;

    cout << "syncWithFolder";
    for (int i = 0; i < fileList.length(); i++)
    {
        QString listFilename = curDirectory + fileList.at(i);
        if (curFilename == listFilename)
        {
            fileListPos = i;
            cout << " found " << curFilename.toStdString() << " at position " << fileListPos << endl;

            // Now store the filenames for the next and previous images.
            int idx;

            idx = fileListPos;
            if (idx == fileList.length() - 1)
                idx = 0;
            else
                idx++;
            nextFile = fileList.at(idx);

            idx = fileListPos;
            if (idx == 0)
                idx = fileList.length() - 1;
            else
                idx--;

            prevFile = fileList.at(idx);
            cout << " --> " << nextFile.toStdString() << endl;
            cout << " <-- " << prevFile.toStdString() << endl;

            return;
        }
    }
    cout << " could not find " << curFilename.toStdString() << endl;
    cout << " --> " << nextFile.toStdString() << endl;
    cout << " <-- " << prevFile.toStdString() << endl;
    // The current image being viewed has been deleted.
    // leave previous / next as it was in readImage
    // This will still leave a bad prevoius/next if both the current image
    // and previous or next were deleted.
    // TODO: move the open file dialog logic from MainDialog.cpp into this class,
    // so when we fail to open a file, show the dialog.
    // (should also enable a hotkey for opening files).
}

bool ImageWindow::readNextImage()
{
    syncWithFolder(); // to make sure we update the folder just before we attempt to go to the next image.
    // TODO: remove these list length checks when we add the open file dialog box;
    // at that point the user will be shown a dialog box and see what's there.
    if (fileList.length() == 0) return false;
    if (fileList.length() == 1) return true;

    QString toRead = curDirectory + nextFile;
    bool result = readImage(toRead);
    return result;
}

bool ImageWindow::readPrevImage()
{
    syncWithFolder();
    if (fileList.length() == 0) return false;
    if (fileList.length() == 1) return true;

    QString toRead = curDirectory + prevFile;
    bool result = readImage(toRead);
    return result;
}

#include <QDir>
bool ImageWindow::deleteCurrentFile()
{
    QString folder, filename;
    QFileInfo fi(curFilename);
    folder = fi.absoluteDir().path();
    filename = fi.fileName();
    folder = QDir(folder).filePath(".pxlpeep_trash");
    QDir curDir(folder);
    if (!curDir.exists())
    {
        if (!QDir().mkdir(folder))
        {
            cerr << "Failed to create temporary trash folder at " << folder.toStdString() << endl;
            return false;
        }
    }
    filename = curDir.filePath(filename);

    QFile file(curFilename);
    if(file.rename(filename)) {
        cout << "Moved " << curFilename.toStdString() << " to " << filename.toStdString() << endl;
        emit signalFileDeleted(filename);
        return true;
    }
    else {
        cerr << "Failed to move " << curFilename.toStdString() << " to " << filename.toStdString() << endl;
        return false;
    }
}

// Copies the file to a sub-folder with the name "pxlpeep_bucket_<bucket>"
bool ImageWindow::copyCurrentFileToBucket(int bucket)
{
    if (bucket < 0 || bucket > 9) {
        cerr << "Invalid bucket '" << bucket << "' for marking image; should be between 0 and 9, inclusive." << endl;
        return false;
    }

    QString folder, filename;
    QFileInfo fi(curFilename);
    folder = fi.absoluteDir().path();
    filename = fi.fileName();
    folder = QDir(folder).filePath(QString("pxlpeep_bucket_%1").arg(bucket));
    QDir curDir(folder);
    if (!curDir.exists())
    {
        if (!QDir().mkdir(folder))
        {
            cerr << "Failed to create sorting bucket folder at " << folder.toStdString() << endl;
            return false;
        }
    }
    filename = curDir.filePath(filename);

    QFile file(curFilename);
    if(file.copy(filename)) {
        cout << "Copied " << curFilename.toStdString() << " to " << filename.toStdString() << endl;
        return true;
    }
    else {
        cerr << "Failed to copy " << curFilename.toStdString() << " to " << filename.toStdString() << endl;
        return false;
    }
}

void ImageWindow::showWindow()
{
    this->showNormal();
}
