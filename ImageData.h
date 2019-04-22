#ifndef IMAGEDATA_H
#define IMAGEDATA_H

#include <FreeImage.h>
#include <QString>
#include <QRect>

//Declare it here instead of adding the include
class ImageWindow;

class ImageData
{
public:
    ImageData() {initClass();}

    bool reallocate(int width, int height, int numChannels, int BPP);

    virtual ~ImageData() {freeData();}
    bool readImage(QString filename);

    int getWidth()       {return width; }
    int getHeight()      {return height;}
    int getArea()        {return width * height;}
    int getNumChannels() {return numChannels;}
    int getBPP()         {return BPP;}

    ushort *data;

    double getMinValue() {return minValue;}
    double getMaxValue() {return maxValue;}
    int getMinIndex()    {return minIndex;}
    int getMaxIndex()    {return maxIndex;}

    #define MULTI_CHANNEL_INDEX ((x + y * width) * numChannels + channel)
    inline double getPixel(int x, int y, int channel) {return data[MULTI_CHANNEL_INDEX];}
    inline double getPixel(int index, int channel) {return data[index * numChannels + channel];}

    void feedData(FIBITMAP *image, FREE_IMAGE_TYPE imgType);
    void feedData(ushort *image);
    void feedDataReverse(ushort *image);

    void recalcMinMax();

    bool doWhiteBalance(QRect &Area);

    bool getEXIFMake(QString &make) {make = EXIFMake; return haveEXIFMake;}
    bool getEXIFFirmware(QString &firmware) {firmware = EXIFFirmware; return haveEXIFFirmware;}
    bool getEXIFISO(int &ISO) {ISO = EXIFISO; return haveEXIFISO;}
    bool getEXIFShutter(double &shutterms) {shutterms = EXIFShutter; return haveEXIFShutter;}
    bool getEXIFAperture(double &aperture) {aperture = EXIFAperture; return haveEXIFAperture;}
    bool getEXIFEV(double &EV);
    bool getEXIFTemperatures(double &sensor, double &dsp, double &bat, double &pmic);

    //This is a pointer to the ImageWindow responsible for displaying this image.
    //If you don't want auto-reallocate and auto-repaint, then set it to nullptr.
    //You can call ImageWindow::SignalNewData to do this manually.
    ImageWindow *painter;

protected:

    int width, height;
    int numChannels;
    int BPP;

    long memSizeBytes;

    double minValue, maxValue;
    int minIndex, maxIndex;
    void checkMinMax(ushort pixValue, int index);

    void initClass();
    void resetMinMax() {minIndex = 0; maxIndex = 0; minValue = 65535; maxValue = 0;}

    inline void setPixel(int index, ushort src) {data[index] = src;}
    inline void setPixel(int x, int y, int channel, ushort src) {data[MULTI_CHANNEL_INDEX] = src;}

    virtual void allocateData();
    virtual void freeData();

    bool readRAW(QString filename);

    //For grey scale images, we assume that we have a bayer array and thus we are looking
    //to equate the grey values within a 2x2 pixel area.
    bool doWhiteBalanceGrey(QRect &greyArea);

    //For color, we just calculate gains for each color.
    bool doWhiteBalanceColor(QRect &colorArea);

    void retrieveEXIFTags(FIBITMAP *img);
    void resetEXIFStatus();
    bool haveEXIFMake;
    QString EXIFMake;
    bool haveEXIFFirmware;
    QString EXIFFirmware;
    bool haveEXIFISO;
    int EXIFISO;
    bool haveEXIFShutter;
    double EXIFShutter;
    bool haveEXIFAperture;
    double EXIFAperture;
    bool haveEXIFTemperatures;
    double EXIFSensorTemp, EXIFDSPTemp, EXIFBatTemp, EXIFPMICTemp;
};

#endif // IMAGEDATA_H
