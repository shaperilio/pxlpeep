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

ImageWindow::ImageWindow(Colormapper &map, int ID)
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
    windowFont.setPixelSize(12);
#ifdef Q_OS_MACOS
    // If we do a titleless window on Mac, you can't resize the window.
    // Tool windows have a smaller titlebar but they are always on top of the main window.
//    setWindowFlags(Qt::Tool);
//    setAttribute(Qt::WA_MacAlwaysShowToolWindow); // so it doesn't disappear when we lose focus.
#else
    setWindowFlags(Qt::CustomizeWindowHint); //sizeable border without title bar.
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
    }
    scene = nullptr;
    colormap = &map;
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
        cout << "Image buffer slot " << i << ": " << sourceImageBufferFilenames[i].toStdString() << endl;
    }
}

bool ImageWindow::readImage(QString filename)
{
    cout << endl;
    if (filename == "")
        return false;

    QFileInfo fileInfo(filename);
    filename = fileInfo.canonicalFilePath();

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
        }
        cout << "Image `" << filename.toStdString() << "` not buffered; loading from disk." << endl;
        currentImageBufferIndex = sourceImageBufferIndex;
    } else {
        // This image is in our buffer; we're done!
        currentImageBufferIndex = idx;
        cout << "Image `" << filename.toStdString() << "` found at buffer position " << idx << "." << endl;
        curFilename = filename;
        sourceImageBufferFilenames[currentImageBufferIndex] = curFilename;

        int lastSlash = curFilename.lastIndexOf('/');
        curDirectory = curFilename.left(lastSlash + 1);
        return translateImage();
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

    int lastSlash = curFilename.lastIndexOf('/');
    curDirectory = curFilename.left(lastSlash + 1);

    reportBufferContents();

    return translateImage();
}

bool ImageWindow::setColormap(ColormapPalette newColormap)
{
    if (colormap == nullptr) return false;
    if (newColormap == getColormap()) return true;
    if (!colormap->setColormap(newColormap)) return false;
    return translateImage();
}

bool ImageWindow::setImageFunction(ImageWindowFunction newFunction)
{
    if (newFunction == getImageFunction()) return true;
    function = newFunction;
//    if (function == ImageWindowFunction::Log10 && getScaleMode() != ImageWindowScaling::Fit)
//        return setScaleMode(ImageWindowScaling::Fit);
    return translateImage();
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
    if (colormap == nullptr)
    {
        cerr << "Attempt to get translation parameters when colormap is null!" << endl;
        return false;
    }

    if (!OKToTranslate)
    {
        cerr << "Attempt to get translation parameters when image is not OK to translate!" << endl;
        return false;
    }

    params = "_";
    QString temp;
    params += temp.sprintf("s%d_", scaling);
    params += temp.sprintf("f%d_", function);
    params += temp.sprintf("m%d_", getColormap());
    params += temp.sprintf("r%d", rotation);
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
    if (colormap == nullptr)
        return false;

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
                    dstIdx = (r * w + c) * 4;
                    break;
                case ImageWindowRotation::CCW90:
                    dstIdx = (c * h + (h - 1 - r)) * 4;
                    dstIdx = (h * w - 1) * 4 - dstIdx;
                    break;
                case ImageWindowRotation::CCW180:
                    dstIdx = (r * w + c) * 4;
                    dstIdx = (h * w - 1) * 4 - dstIdx;
                    break;
                case ImageWindowRotation::CCW270:
                    dstIdx = (c * h + (h - 1 - r)) * 4;
                    break;
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
                    dstIdx = (r * w + c) * 4;
                    break;
                case ImageWindowRotation::CCW90:
                    dstIdx = (c * h + (h - 1 - r)) * 4;
                    dstIdx = (h * w - 1) * 4 - dstIdx;
                    break;
                case ImageWindowRotation::CCW180:
                    dstIdx = (r * w + c) * 4;
                    dstIdx = (h * w - 1) * 4 - dstIdx;
                    break;
                case ImageWindowRotation::CCW270:
                    dstIdx = (c * h + (h - 1 - r)) * 4;
                    break;
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

void ImageWindow::handleMousePressEvent(QMouseEvent *event, bool forwarded)
{
    setShowHelp(false);

    if(event->buttons() == Qt::MiddleButton)
    {
        panRef = event->globalPos();
        setCursor(Qt::ClosedHandCursor);
    }
    else if (event->buttons() == Qt::RightButton)
        mouseDragRef = event->globalPos() - frameGeometry().topLeft();
    else if (event->buttons() == Qt::LeftButton)
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

void ImageWindow::handleMouseReleaseEvent(QMouseEvent *event, bool forwarded)
{
    setCursor(Qt::CrossCursor);

    //Note the use of button and not buttons!
    if (event->button() == Qt::LeftButton)
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
    else if (event->buttons() == Qt::MiddleButton)
    {
        //Pan the image.
        QPoint panDelta = event->globalPos() - panRef;
        curMousePos += panDelta; //or the info box will show coordinates changing.
        horizontalScrollBar()->setValue(horizontalScrollBar()->value() - panDelta.x());
        verticalScrollBar()->setValue  (verticalScrollBar()->value()   - panDelta.y());
        panRef = event->globalPos();
    }
    else if (event->buttons() == Qt::LeftButton)
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
            viewport()->update(infoRect);
        }
    }

    event->accept();
}

void ImageWindow::mouseMoveEvent(QMouseEvent *event)
{
    emit signalMouseMoveEvent(event, this->ID);

    handleMouseMoveEvent(event, false);
}

void ImageWindow::handleWheelEvent(QWheelEvent *event, bool forwarded)
{
    showHelp = false;

    wheelAccumulator += event->delta();

    zoomCtr = event->pos();

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
    setMatrix(scaleMatrix);
    showCentered();
}

void ImageWindow::showCentered()
{
    // This will only properly center the image if it has been translated.
    horizontalScrollBar()->setValue((horizontalScrollBar()->maximum() + horizontalScrollBar()->minimum()) / 2);
    verticalScrollBar()->  setValue((verticalScrollBar()->  maximum() + verticalScrollBar()->  minimum()) / 2);
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
    QMatrix scaleMatrix;
    scaleMatrix.scale(zoomFactor, zoomFactor);
    setMatrix(scaleMatrix);

    //Now move the scroll bars so that zooming is centered, but only if zooming in!
    //Does not apply for 1:1 zoom.
    if (zoomIncrement != 0 && zoomIncrement > 0)
    {
        QPoint windowCtr(geometry().width()/2, geometry().height()/2);
        QPoint delta(zoomCtr - windowCtr);
        horizontalScrollBar()->setValue(horizontalScrollBar()->value() + delta.x());
        verticalScrollBar()  ->setValue(verticalScrollBar()  ->value() + delta.y());
    }
}

void ImageWindow::paintEvent(QPaintEvent *event)
{
    if (!OKToDraw) return;

    QGraphicsView::paintEvent(event);

    drawRulers();
    drawInfoBox();
    drawHelp();
    drawColorbar();
    drawROI();

    event->accept();
}

inline double ImageWindow::applyImageFunction(double value)
{
    if (function == ImageWindowFunction::Log10)
    {
        if (value > 0) return log10(value); else return 0;
    }
    return value;
}

void ImageWindow::drawInfoBox()
{
    if (!showInfoBox) return;

    QFontMetrics fm(windowFont);

    //What we will show:
    //Filename
    QString line1 = curFilename;

    //Width, height, zoom, and rotation
    auto sourceImage = sourceImageBuffer[currentImageBufferIndex];
    QString line2;
    line2.sprintf("W = %d, H = %d pix (%.2fX)",
                  sourceImage->getWidth(), sourceImage->getHeight(), zoomFactor);

    QString rot;
    switch (rotation)
    {
    case ImageWindowRotation::Zero:
        rot = ""; break;
    case ImageWindowRotation::CCW90:
        rot = " 90deg"; break;
    case ImageWindowRotation::CCW180:
        rot = " 180deg"; break;
    case ImageWindowRotation::CCW270:
        rot = " 270deg"; break;
    }
    line2.append(rot);

    QString line3;
    bool line3Valid = false;
    int ISO;
    if (sourceImage->getEXIFISO(ISO))
    {
        line3Valid = true;
        double shutter, EV;
        sourceImage->getEXIFShutter(shutter);
        sourceImage->getEXIFEV(EV);
        line3.sprintf("ISO = %d, shutter = %.2f ms, EV = %.2f", ISO, shutter, EV);
    }

    //Pixel coordinates
    QPointF zoomedPos = mapToScene(curMousePos);
    QString line4;

    double curX, curY;

    curY = zoomedPos.y();
    if (imgYOriginIsBottom)
        curY = getImageHeight() - curY;
    curY = curY + (imgIsZeroIndexed ? 0 : 1) - 0.5;

    curX = zoomedPos.x() + (imgIsZeroIndexed ? 0 : 1) - 0.5;

    line4.sprintf("X = %.1f, Y = %.1f", curX, curY);

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
            colorVal.sprintf(fmt, (int)sourceImage->getPixel(x, y, 0));
            colorVal = " -> " + colorVal;
        }
        else
        {
            QString R, G, B;
            if (activeChannels & chanR)
                R.sprintf(fmt, (int)sourceImage->getPixel(x, y, 0));
            else
                R = "OFF";

            if (activeChannels & chanG)
                G.sprintf(fmt, (int)sourceImage->getPixel(x, y, 1));
            else
                G = "OFF";

            if (activeChannels & chanB)
                B.sprintf(fmt, (int)sourceImage->getPixel(x, y, 2));
            else
                B = "OFF";

            colorVal = " -> " + R + ", " + G + ", " + B;
        }
        line4.append(colorVal);
    }

    int maxW = fm.width(line1);
    if(fm.width(line2) > maxW) maxW = fm.width(line2);
    if(line3Valid && fm.width(line3) > maxW) maxW = fm.width(line3);
    if(fm.width(line4) > maxW) maxW = fm.width(line4);

    int lineH = fm.height();
    int lineVspace = 0;

    int infoW = maxW + 2 * boxMargin;
    int numLines = (line3Valid ? 4 : 3);
    int infoH = lineH * numLines + lineVspace * (numLines - 1) + 2 * boxMargin;

    QRect winRect = geometry();

    infoRect.setLeft(winRect.width() - 1 - boxPad - infoW);
    infoRect.setTop(boxPad);
    infoRect.setWidth(infoW);
    infoRect.setHeight(infoH);

    //Now start drawing stuff.
    QPainter p(viewport());
    p.setBrush(Qt::black);
    p.drawRect(infoRect);

    p.setPen(Qt::white);
    p.setFont(windowFont);
    QRect lineRect = infoRect;
    lineRect.setX(lineRect.x() + boxMargin);

    lineRect.setY(lineRect.y() + boxMargin);
    p.drawText(lineRect, line1);

    lineRect.setY(lineRect.y() + lineVspace + lineH);
    p.drawText(lineRect, line2);

    if (line3Valid)
    {
        lineRect.setY(lineRect.y() + lineVspace + lineH);
        p.drawText(lineRect, line3);
    }

    lineRect.setY(lineRect.y() + lineVspace + lineH);
    p.drawText(lineRect, line4);
}

void ImageWindow::drawRulers()
{
    if (!showRulers) return;

    int minTickSpacingpix = 50;
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
            xImg += 1;
            xDraw = mapFromScene(xImg, 0).x(); //convert to viewport coordinates.
            if (xImg > getImageWidth())
                break;
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
        QString coord;
        coord.sprintf("%d", xCoord);
        QRect coordBox(xDraw + 2, tickLen - fm.height() + 2, fm.width(coord), fm.height());
        p.fillRect(coordBox, blackBrush);
        p.drawText(coordBox, coord);
        coordBox = QRect(xDraw + 2, r.height() - 1 - tickLen - 1, fm.width(coord), fm.height());
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
    double yImg = 0.5, yDraw, oldyImg;
    while (1)
    {
        yDraw = mapFromScene(0, yImg).y(); //convert to viewport coordinates.
        while (yDraw < cornerMargin)
        {
            yImg += 1;
            yDraw = mapFromScene(0, yImg).y(); //convert to viewport coordinates.
            if (yImg > getImageHeight())
                break;
        }
        if (yDraw > viewport()->height() - 1 - cornerMargin || yImg > getImageHeight()) break; //reached the other end.

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

        QString coord;
        coord.sprintf("%d", yCoord);
        QRect coordBox(0, yDraw + 2, fm.width(coord) + 1, fm.height());
        p.fillRect(coordBox, blackBrush);
        coordBox.setX(coordBox.x() + 1);
        p.drawText(coordBox, coord);
        coordBox = QRect(r.width() - 1 - fm.width(coord), yDraw + 2, fm.width(coord) + 1, fm.height());
        p.fillRect(coordBox, blackBrush);
        coordBox.setX(coordBox.x() + 1);
        p.drawText(coordBox, coord);

        //Remember this as the previous image coordinate.
        oldyImg = yImg;

        yDraw += (double)minTickSpacingpix; //increment to the next tick mark, and calculate new image coordinate.
        yImg = floor(mapToScene(0, yDraw).y()) + 0.5; //the 0.5 is to be at the middle of zoomed-in pixels.
        if (yImg == oldyImg) //can happen if we're zoomed in too much; increment xImg manually.
            yImg += 1;
        if (yImg > getImageHeight()) //done with image.
            break;
    }
}

void ImageWindow::drawColorbar()
{
    if (!showColorbar)
        return;

    QString title;
    title = getColormapName();
    if (getImageFunction() == ImageWindowFunction::Log10)
        title.append(" log");
    if (getScaleMode() == ImageWindowScaling::Fit)
        title.append(" fit");

    QString minTxt;
    minTxt.sprintf("%.1f", scaleMin);

    QString maxTxt;
    maxTxt.sprintf("%.1f", scaleMax);

    QFontMetrics fm(windowFont);
    int textH = fm.height();
    int barWidth = 255;
    while (fm.width(title) + fm.width(minTxt) + fm.width(maxTxt) + 50 > barWidth)
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
        p.drawText(colorbarImage.width() - boxMargin - fm.width(maxTxt), boxMargin + textH, maxTxt);

        p.drawText(colorbarImage.width() / 2 - fm.width(title) / 2, boxMargin + textH, title);

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

    QString coords;
    coords.sprintf("dx = %d, dy = %d -> h = %.2f", ROI.width(), ROI.height(),
                sqrt((double)(ROI.width() * ROI.width() + ROI.height() * ROI.height())));
    QFontMetrics fm(windowFont);
    QRect coordBox((ROI1Screen.x() + ROI2Screen.x() - fm.width(coords)) / 2,
                (ROI1Screen.y() + ROI2Screen.y() - fm.height()) / 2,
                fm.width(coords) + 1, fm.height());
    p.fillRect(coordBox, blackBrush);
    coordBox.setX(coordBox.x() + 1);
    p.drawText(coordBox, coords);
}

#include <QApplication>
void ImageWindow::snapWindowTo(snapType snap, int screenNum)
{
    QDesktopWidget * desktop = QApplication::desktop();
    QRect screenDims;
    int halfBorder = (frameGeometry().width()  - geometry().width() )/2; // thickness of window border.

    //This does not directly support vertical screen layout, i.e. one monitor above another.

    //Note - the way this gets the number of screens and screen dimensions is deprecated in QT so it
    //will look like these function calls are all wrong if you look at the docs.

    // QT can't correctly get the geometry of Ubuntu's Unity desktop (16.04), so we just pad it.
    int buggyMargin = 10;
    halfBorder += buggyMargin;

    //We need to establish on our own the screen layout for left-to-right, because Qt doesn't
    //seem to handle that very well.
    int numScreens = desktop->numScreens();
    int screensL2R[numScreens];
    memset(screensL2R, -1, numScreens * sizeof(int));
    for (int j = 0; j < numScreens; j++)
    {
        int curMin = std::numeric_limits<int>::max();
        for (int i = 0; i < numScreens; i++)
        {
            bool skip = false;
            for (int k = 0; k < j; k++)
                if (i == screensL2R[k])
                {
                    skip = true;
                    break;
                }
            if (skip) continue; //screen already accounted for.
            if (desktop->screenGeometry(i).left() < curMin)
            {
                screensL2R[j] = i;
                curMin = desktop->screenGeometry(i).left();
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
            screenNum = desktop->screenNumber(geometry().topLeft());
            //Find out if there's a screen to the left of this one.
            int i;
            for (i = 0; i < numScreens; i++)
                if (screenNum == screensL2R[i]) break;
            if (i == 0)
                break; //do nothing; no more monitors available.
            snapWindowTo(Right, screensL2R[i-1]); //snap right on the monitor to the left.
            break;
        }
        currentSnap = Left;
        if (screenNum == -1)
            screenNum = desktop->screenNumber(geometry().topLeft());
        screenDims = desktop->availableGeometry(screenNum);
        move(screenDims.left(), screenDims.top());
        resize(screenDims.width()/2 - 2*halfBorder, screenDims.height() - 2*halfBorder);
        break;
    case TopLeft:
        if (currentSnap == TopLeft)
        {
            screenNum = desktop->screenNumber(geometry().topLeft());
            int i;
            for (i = 0; i < numScreens; i++)
                if (screenNum == screensL2R[i]) break;
            if (i == 0)
                break;
            snapWindowTo(TopRight, screensL2R[i-1]);
            break;
        }
        currentSnap = TopLeft;
        if (screenNum == -1)
            screenNum = desktop->screenNumber(geometry().topLeft());
        screenDims = desktop->availableGeometry(screenNum);
        move(screenDims.left(), screenDims.top());
        resize(screenDims.width()/2 - 2*halfBorder, screenDims.height()/2 - 2*halfBorder);
        break;
    case BottomLeft:
        if (currentSnap == BottomLeft)
        {
            screenNum = desktop->screenNumber(geometry().topLeft());
            int i;
            for (i = 0; i < numScreens; i++)
                if (screenNum == screensL2R[i]) break;
            if (i == 0)
                break;
            snapWindowTo(BottomRight, screensL2R[i-1]);
            break;
        }
        currentSnap = BottomLeft;
        if (screenNum == -1)
            screenNum = desktop->screenNumber(geometry().topLeft());
        screenDims = desktop->availableGeometry(screenNum);
        move(screenDims.left(), screenDims.height()/2 + screenDims.top());
        resize(screenDims.width()/2 - 2*halfBorder, screenDims.height()/2 - 2*halfBorder);
        break;
    case Right:
        if (currentSnap == Right)
        {
            screenNum = desktop->screenNumber(geometry().topLeft());
            //Find out if there's a screen to the right of this one.
            int i;
            for (i = 0; i < numScreens; i++)
                if (screenNum == screensL2R[i]) break;
            if (i == numScreens - 1)
                break; //do nothing; no more monitors available.
            snapWindowTo(Left, screensL2R[i+1]); //snap right on the monitor to the left.
            break;
        }
        currentSnap = Right;
        if (screenNum == -1)
            screenNum = desktop->screenNumber(geometry().topRight());
        screenDims = desktop->availableGeometry(screenNum);
        move(screenDims.width()/2 + screenDims.left(), screenDims.top());
        resize(screenDims.width()/2 - 2*halfBorder, screenDims.height() - 2*halfBorder);
        break;
    case TopRight:
        if (currentSnap == TopRight)
        {
            screenNum = desktop->screenNumber(geometry().topLeft());
            int i;
            for (i = 0; i < numScreens; i++)
                if (screenNum == screensL2R[i]) break;
            if (i == numScreens - 1)
                break;
            snapWindowTo(TopLeft, screensL2R[i+1]);
            break;
        }
        currentSnap = TopRight;
        if (screenNum == -1)
            screenNum = desktop->screenNumber(geometry().topLeft());
        screenDims = desktop->availableGeometry(screenNum);
        move(screenDims.width()/2 + screenDims.left(), screenDims.top());
        resize(screenDims.width()/2 - 2*halfBorder, screenDims.height()/2 - 2*halfBorder);
        break;
    case BottomRight:
        if (currentSnap == BottomRight)
        {
            screenNum = desktop->screenNumber(geometry().topLeft());
            int i;
            for (i = 0; i < numScreens; i++)
                if (screenNum == screensL2R[i]) break;
            if (i == numScreens - 1)
                break;
            snapWindowTo(BottomLeft, screensL2R[i+1]);
            break;
        }
        currentSnap = BottomRight;
        if (screenNum == -1)
            screenNum = desktop->screenNumber(geometry().topLeft());
        screenDims = desktop->availableGeometry(screenNum);
        move(screenDims.width()/2 + screenDims.left(), screenDims.height()/2 + screenDims.top());
        resize(screenDims.width()/2 - 2*halfBorder, screenDims.height()/2 - 2*halfBorder);
        break;
    case Max:
        currentSnap = Max;
        if (screenNum == -1)
            screenNum = desktop->screenNumber( QPoint((geometry().left() + geometry().right() )/2,
                                                     (geometry().top()  + geometry().bottom())/2));
        screenDims = desktop->availableGeometry(screenNum);
        move(screenDims.left(), screenDims.top());
        resize(screenDims.width() - 2*halfBorder, screenDims.height() - 2*halfBorder);
        break;
    case Restore:
        currentSnap = Restore;
        if (screenNum == -1)
            screenNum = desktop->screenNumber( QPoint((geometry().left() + geometry().right() )/2,
                                                     (geometry().top()  + geometry().bottom())/2));
        screenDims = desktop->availableGeometry(screenNum);
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

    QPixmap pixmap = screen->grabWindow(window->winId());
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

    cout << std::hex;
    cout << "Key press: 0x" << event->key() << " with modifier 0x" << mods << endl;
    cout << std::dec;

    auto sourceImage = sourceImageBuffer[currentImageBufferIndex];

    switch(event->key())
    {
        case Qt::Key_QuoteLeft:
        {
            if (mods == Qt::ControlModifier)
                zoomFit();
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
        case Qt::Key_1:
        {
            if (mods == Qt::ControlModifier)
                zoom1To1();
            break;
        }
        case Qt::Key_Left:
        {
            if (!forwarded)
            {
                if (mods == Qt::ControlModifier ||
                    mods == (Qt::ControlModifier | Qt::KeypadModifier))
                {
                    snapWindowTo(Left, -1);
                    break;
                }
                if (mods == (Qt::ControlModifier | Qt::ShiftModifier) ||
                    mods == (Qt::ControlModifier | Qt::ShiftModifier | Qt::KeypadModifier))
                {
                    snapWindowTo(TopLeft, -1);
                    break;
                }
                if (mods == (Qt::AltModifier     | Qt::ShiftModifier) ||
                    mods == (Qt::ControlModifier | Qt::KeypadModifier | Qt::KeypadModifier))
                {
                    snapWindowTo(BottomLeft, -1);
                    break;
                }
                if (mods == Qt::NoModifier || Qt::KeypadModifier)
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
                    snapWindowTo(Right, -1);
                    break;
                }
                if (mods == (Qt::ControlModifier | Qt::ShiftModifier) ||
                    mods == (Qt::ControlModifier | Qt::KeypadModifier | Qt::KeypadModifier))
                {
                    snapWindowTo(TopRight, -1);
                    break;
                }
                if (mods == (Qt::AltModifier     | Qt::ShiftModifier) ||
                    mods == (Qt::ControlModifier | Qt::KeypadModifier | Qt::KeypadModifier))
                {
                    snapWindowTo(BottomRight, -1);
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
                    snapWindowTo(Max, -1);
            break;
        }
        case Qt::Key_Down:
        {
            if (!forwarded)
                if (mods == Qt::ControlModifier || mods == (Qt::ControlModifier | Qt::KeypadModifier))
                    snapWindowTo(Restore, -1);
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
        case Qt::Key_F:
        {
            if (mods != Qt::NoModifier) break;

            ImageWindowFunction f = getImageFunction();
            if (f == ImageWindowFunction::OneToOne)
                setImageFunction(ImageWindowFunction::Log10);
            else
                setImageFunction(ImageWindowFunction::OneToOne);
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
        case Qt::Key_W:
        {
            if (mods != Qt::NoModifier) break;
            if (ROIisValid && sourceImage != nullptr)
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
        case Qt::Key_R:
        {
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
        case Qt::Key_0:
        {
            if (mods != Qt::NoModifier) break;

            imgIsZeroIndexed = !imgIsZeroIndexed;
            drawRulers();
            viewport()->update();
            break;
        }
        case Qt::Key_Y:
        {
            if (mods != Qt::NoModifier) break;

            imgYOriginIsBottom = !imgYOriginIsBottom;
            drawRulers();
            viewport()->update();
            break;
        }
        default:
        {
            if (mods == Qt::ControlModifier ||
                mods == Qt::AltModifier ||
                mods == Qt::ShiftModifier ||
                mods == (Qt::ControlModifier | Qt::ShiftModifier) ||
                mods == (Qt::ControlModifier | Qt::AltModifier) ||
                mods == (Qt::ShiftModifier   | Qt::AltModifier) ||
                mods == (Qt::ControlModifier | Qt::AltModifier | Qt::ShiftModifier)
                )
                break; //don't show help for all the aux keys.
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

void ImageWindow::handleKeyRelease(QKeyEvent *event, bool forwarded)
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
    int width = fm.width("Left button                        select ROI"); // example line
    menu.append("\n");
    int numLines = 1;
    menu.append("\n");                                   numLines++;
    menu.append("--Mouse--\n");                          numLines++;
    menu.append("Left button                        select ROI\n"); numLines++;
    menu.append("Middle button                             pan\n"); numLines++;
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
    menu.append("M                           raise main window\n"); numLines++;
    menu.append("\n"); numLines++;
    menu.append("CTRL+C                copy image to clibpoard\n"); numLines++;
    menu.append("CTRL+SHIFT+C     copy screenshot to clibpoard\n"); numLines++;
    menu.append("CTRL+ALT+C                 save image to file\n"); numLines++;
    menu.append("CTRL+ALT+SHIFT+C      save screenshot to file\n"); numLines++;
    menu.append("\n"); numLines++;
    menu.append("lt arrow                       previous image\n"); numLines++;
    menu.append("rt arrow                           next image\n"); numLines++;
    menu.append("F5                               reload image\n"); numLines++;
    menu.append("Del                    delete image from disk\n"); numLines++;
    menu.append("\n"); numLines++;
    menu.append("CTRL+`                            zoom to fit\n"); numLines++;
    menu.append("CTRL+1                               zoom 1:1\n"); numLines++;
    menu.append("\n"); numLines++;
    menu.append("I                             toggle info box\n"); numLines++;
    menu.append("C                            toggle color bar\n"); numLines++;
    menu.append("X                               toggle rulers\n"); numLines++;
    menu.append("\n"); numLines++;
    menu.append("V / SHIFT+V                   cycle colormaps\n"); numLines++;
    menu.append("F                 toggle pixel value function\n"); numLines++;
    menu.append("S                    toggle pixel value scale\n"); numLines++;
    menu.append("W                   compute ROI white balance\n"); numLines++;
    menu.append("A / SHIFT+A               rotate image 90 deg\n"); numLines++;
    menu.append("R                          toggle red channel\n"); numLines++;
    menu.append("SHIFT+R                      red channel only\n"); numLines++;
    menu.append("G                        toggle greem channel\n"); numLines++;
    menu.append("SHIFT+G                    green channel only\n"); numLines++;
    menu.append("B                         toggle blue channel\n"); numLines++;
    menu.append("SHIFT+B                     blue channel only\n"); numLines++;
    menu.append("0                        start is zero or one\n"); numLines++;
    menu.append("Y                  origin is at top or bottom\n"); numLines++;

    QRect menuRect(r.width() - 1 - (width + 2 * boxMargin) - boxPad,
                   infoRect.top() + infoRect.height() + boxPad,
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
            return;
        }
    }
    cout << " could not find " << curFilename.toStdString() << endl;
}

bool ImageWindow::readNextImage()
{
    syncWithFolder(); // to make sure we update the folder just before we attempt to go to the next image.
    if (fileList.length() == 0) return false;
    if (fileList.length() == 1) return true;

    if (fileListPos == fileList.length() - 1)
        fileListPos = 0;
    else
        fileListPos++;

    QString nextFile = curDirectory + fileList.at(fileListPos);
    bool result = readImage(nextFile);
    syncWithFolder(); // to reset our location in the file list, in case we loaded the image correctly.
    return result;
}

bool ImageWindow::readPrevImage()
{
    syncWithFolder();
    if (fileList.length() == 0) return false;
    if (fileList.length() == 1) return true;

    if (fileListPos == 0)
        fileListPos = fileList.length() - 1;
    else
        fileListPos--;

    QString nextFile = curDirectory + fileList.at(fileListPos);
    bool result = readImage(nextFile);
    syncWithFolder();
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
    if(file.rename(filename))
        cout << "Moved " << curFilename.toStdString() << " to " << filename.toStdString() << endl;
    else
        cerr << "Failed to move " << curFilename.toStdString() << " to " << filename.toStdString() << endl;

    emit signalFileDeleted(filename);
    return true;
}

void ImageWindow::showWindow()
{
    this->showNormal();
}
