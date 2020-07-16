#ifndef IMAGEWINDOW_H
#define IMAGEWINDOW_H

#include <QGraphicsView>
#include <QDateTime>
#include "ImageData.h"
#include "colormapper.h"
#include "MainDialog.h"

//Scaling governs how bitmap values are linearly remapped to display values.
enum ImageWindowScaling : int
{
    Fit,		//normalized according to min/max.
    Centered,	//normalized, but with zero at middle of the scale.
    User		//min/max set by user.
};

//Function governs whether the scaling is linear or logarithmic.
enum ImageWindowFunction : int
{
    OneToOne,
    Log10BrightenDark,
    Log10DarkenLight,
    BrightenDark,
    DarkenLight,
    NUM_WINDOFUNCTIONS
};

enum ImageWindowRotation : int
{
    Zero,
    CCW90,
    CCW180,
    CCW270
};

class ImageWindow : public QGraphicsView
{
    Q_OBJECT

public:
    ImageWindow(Colormapper &map, int ID);

    bool readImage(QString filename);

    void setDefaultOptions()
    {
        imgYOriginIsBottom = false;
        imgIsZeroIndexed   = true;

        userMin = 0;
        userMax = 255;
        scaling = ImageWindowScaling::User;
        function = ImageWindowFunction::OneToOne;
        rotation = ImageWindowRotation::Zero;
        flipHorizontal = false;
        flipVertical   = false;

        activeChannels = chanR | chanG | chanB;

        showInfoBox = true;
        showRulers  = true;
        showColorbar = true;
        showHelp = true;
    }

    void resetWheelAccumulator(); // For synchronizing windows.

    //Display options
    bool imgYOriginIsBottom; //default is false; image origin is top left corner.
    bool imgIsZeroIndexed;   //default is true; first pixel is pixel zero.

    // These are not in use.
//    bool setSourceImage(ImageData *image) {sourceImage = image; return translateImage();}
//    ImageData * getSourceImage() {return sourceImage;}

    double getUserMin() {return userMin;}
    double getUserMax() {return userMax;}
    ImageWindowScaling  getScaleMode()     {return scaling; }
    ImageWindowFunction getImageFunction() {return function;}
    ImageWindowRotation getImageRotation() {return rotation;}
    bool getImageFlipHorizontal() {return flipHorizontal;}
    bool getImageFlipVertical()   {return flipVertical;}

    bool setUserMin(double newMin);
    bool setUserMax(double newMax);
    bool setScaleMode(ImageWindowScaling newMode);
    bool setImageFunction(ImageWindowFunction newFunction);
    bool setImageRotation(ImageWindowRotation newRotation);
    bool setImageFlip(bool horizontal, bool vertical);
    int  getImageWidth();
    int  getImageHeight(); //these return the dimensions according to rotation.

    ColormapPalette getColormap()     {return colormap->getColormap();}
    QString         getColormapName() {return colormap->getColormapName();}
    bool setColormap(ColormapPalette newColormap);

    void holdTranslation()    {OKToTranslate = false;}
    bool releaseTranslation() {OKToTranslate = true; return translateImage();}

    void holdDraw()    {OKToDraw = false;}
    void releaseDraw() {OKToDraw = true; viewport()->update();}

    void holdAll()    {holdTranslation(); holdDraw();}
    bool releaseAll() {bool r = releaseTranslation(); releaseDraw(); return r;}

    void zoomFit();
    void zoom1To1() {zoom(0);}
    void showCentered();

    void syncWithFolder();
    bool readNextImage();
    bool readPrevImage();

    int myButtonNo;

    void showWindow();

    //Parent dialog
    MainDialog *parent;
    //	HWND parenthWnd;

    void setShowRulers(bool val) {if (showRulers == val) return; showRulers = val; viewport()->update();}
    bool getShowRulers() {return showRulers;}

    void setShowColorbar(bool val) {if (showColorbar == val) return; showColorbar = val; viewport()->update();}
    bool getShowColorbar() {return showColorbar;}

    void setShowInfoBox(bool val) {if (showInfoBox == val) return; showInfoBox = val; viewport()->update();}
    bool getShowInfoBox() {return showInfoBox;}

    void setShowHelp(bool val) {if (showHelp == val) return; showHelp = val; viewport()->update();}
    bool getShowHelp() {return showHelp;}

    bool signalNewData() {bool r = translateImage(); if (r) viewport()->update(); return r;}

    bool pasteFromClipboard();

protected:
    int ID; // ID for the main window, to communicate back when we're done.
    QImage translatedImage;   //need an image to access pixels directly, but...
    QPixmap translatedPixmap; //...need a pixmap for display.

    static constexpr int IMAGE_BUFFER_LENGTH = 10;
    ImageData *sourceImageBuffer[IMAGE_BUFFER_LENGTH];
    QString sourceImageBufferFilenames[IMAGE_BUFFER_LENGTH];
    QDateTime sourceImageBufferModifiedDate[IMAGE_BUFFER_LENGTH];
    int sourceImageBufferIndex = 0;  // Stores the next available slot in the buffer.
    int currentImageBufferIndex = 0; // Stores the slot that we're currently looking at.
    void reportBufferContents();

    QGraphicsScene *scene;
    Colormapper *colormap;

    double userMin, userMax;
    ImageWindowScaling scaling;
    double scaleMin, scaleMax;
    double scale, offset;
    int maxDisp;
    ImageWindowFunction function;
    inline double applyImageFunction(double value);
    inline double imageFunctionLog10BrightenDark(double value);
    inline double imageFunctionLog10DarkenLight(double value);
    double dipFactor;
    bool setDipFactor(double dipFactor);
    inline double imageFunctionBrightenDark(double value);
    inline double imageFunctionDarkenLight(double value);
    inline double imageFunctionNone(double value);
    ImageWindowRotation rotation;

    bool flipHorizontal, flipVertical;

    static const uchar chanR = 1;
    static const uchar chanG = 2;
    static const uchar chanB = 4;
    uchar activeChannels; //bitfield; R = bit 0, G = bit 1, B = bit 2;
    bool OKToTranslate;
    bool translateImage();
    bool translationIsFresh;
    bool getTranslationParamsString(QString &params);

    bool OKToDraw;

    QFont windowFont;
    QString curFilename, curDirectory;

    void closeEvent(QCloseEvent *event) Q_DECL_OVERRIDE;

    //Mouse wheel does zoom.
    QPoint zoomCtr;
    int wheelAccumulator;
    void wheelEvent(QWheelEvent *event) Q_DECL_OVERRIDE;
    int zoomLevel;
    double zoomFactor;
    static constexpr double zoomStep = 1.41421356237;//sqrt(2.0);
    static const int maxZoomLevel = 16;
    static const int minZoomLevel = -16;
    static const int deltaThresh = 360; //mouse wheel will zoom in/out once delta exceeds this.
    void zoom(int zoomIncrement);

    void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    QPoint mouseDragRef, panRef;
    void checkROIpoint(QPoint &point);
    void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseReleaseEvent(QMouseEvent *event) Q_DECL_OVERRIDE;

    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;

    static const int boxMargin = 5;
    static const int boxPad    = 25;

    QPoint curMousePos;
    QRect infoRect;
    bool showInfoBox;
    void drawInfoBox();

    bool showRulers;
    void drawRulers();

    bool showColorbar;
    void drawColorbar();
    QImage colorbarImage;

    QPoint ROI1, ROI2; //these are stored in image coordinates.
    QRect  ROI;        //this is calculated during drawROI.
    bool ROIisValid;
    void drawROI();

    void keyPressEvent(QKeyEvent *event) Q_DECL_OVERRIDE;
    void keyReleaseEvent(QKeyEvent *event) Q_DECL_OVERRIDE;

    bool showHelp;
    void drawHelp();

    enum snapType
    {
        None,
        Left,
        Right,
        TopLeft,
        TopRight,
        BottomLeft,
        BottomRight,
        Max,
        Restore,
    };

    snapType currentSnap;
    void snapWindowTo(snapType snap, int screenNum);

    QList<QString> fileList;
    QString nextFile, prevFile;
    int fileListPos;
    bool deleteCurrentFile();

    bool copyCurrentFileToBucket(int bucket);

    void copyImageToClipboard();
    void saveImageToFile();
    bool takeScreenshot(QImage &screenshot);
    void copyScreenshotToClipboard();
    void saveScreenshotToFile();

    void setTitle(QString title);
signals:
    // These are connected to slots in MainDialog for inter-window syncing.
    // When something happens in an ImageWindow, these signals notify the main window.
    // The way we chose to do this, there's one signal per event handler.
    void signalWheelEvent(QWheelEvent *event, int windowID);
    void signalMousePressEvent(QMouseEvent *event, int windowID);
    void signalMouseMoveEvent(QMouseEvent *event, int windowID);
    void signalMouseReleaseEvent(QMouseEvent *event, int windowID);
    void signalKeyPressed(QKeyEvent *event, int windowID);
    void signalKeyReleased(QKeyEvent *event, int windowID);
    void signalFileDeleted(QString &trashLocation);


public:
    // The slots that the signals are connected to above will in turn call these functions.
    // When something happens in an ImageWindow, these slots are used to replicate the behavior in all the other windows.
    // These actually contain the event handling code, so they get called by the window itself as well.
    // forwarded is true when the main window is calling these.
    void handleWheelEvent(QWheelEvent *event, bool forwarded);
    void handleMousePressEvent(QMouseEvent *event, bool forwarded);
    void handleMouseMoveEvent(QMouseEvent *event, bool forwarded);
    void handleMouseReleaseEvent(QMouseEvent *event, bool forwarded);
    void handleKeyPress(QKeyEvent *event, bool forwarded);
    void handleKeyRelease(QKeyEvent *event, bool forwarded);
};

#endif // IMAGEWINDOW_H
