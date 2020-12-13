#include "ImageData.h"
#include <math.h>
#include <QFile>
#include <QStringList>
#include <ImageWindow.h>
#include <iostream>

using namespace std;

void ImageData::initClass()
{
    width = height = 0;
    numChannels = 0;
    memSizeBytes = 0;
    BPP = -1;
    data = nullptr;

    colorGains[0] = 1;
    colorGains[1] = 1;
    colorGains[2] = 1;

    greyGains[0][0] = 1;
    greyGains[1][0] = 1;
    greyGains[0][1] = 1;
    greyGains[1][1] = 1;

    resetMinMax();

    painter = nullptr;
}

bool ImageData::reallocate(int width, int height, int numChannels, int BPP)
{
    if (width * height * numChannels * (long)ceil(BPP / 8.0) <= memSizeBytes)
    {
        //Image fits in current memory.
        this->width = width;
        this->height = height;
        this->numChannels = numChannels;
        this->BPP = BPP;
        return true;
    }

    //Doesn't fit. Need to reallocate.
    if (data != nullptr)
        freeData();

    this->width = width;
    this->height = height;
    this->numChannels = numChannels;
    this->BPP = BPP;

    allocateData();

    if (data == nullptr)
        return false;

    return true;
}

void ImageData::allocateData()
{
    memSizeBytes = getArea() * getNumChannels() * (long)ceil(BPP / 8.0);
    data = new ushort[memSizeBytes];
    if (data == nullptr)
        memSizeBytes = 0;
}

void ImageData::freeData()
{
    if (data != nullptr)
        delete []data;

    data = nullptr;
    memSizeBytes = 0;
}

bool ImageData::readRAW(QString filename)
{
    QFile file(filename);

    if (!file.exists()) return false;

    if (!file.open(QIODevice::ReadOnly))
        return false;

    ushort *rawImage = nullptr;
    // Start with some absurd size -  100 MP @ 16 bits
    rawImage = new ushort[10000 * 10000 * 16/8];
    if (rawImage == nullptr)
    {
        file.close();
        return false;
    }

    int w, h;
    qint64 elems = file.read((char *)rawImage, 10000 * 10000 * 16/8);
    // Now we check some particular sizes that we already know to exist.
    if (elems == 4000 * 3000 * 16/8)
    {
        w = 4000;
        h = 3000;
    }
    else if (elems == 3264 * 2448 * 16/8)
    {
        w = 3264;
        h = 2448;
    }
    else if (elems == 4208 * 3120 * 16/8)
    {
        w = 4208;
        h = 3120;
    }
    else if (elems == 4160 * 3120 * 16/8)
    {
        w = 4160;
        h = 3120;
    }
    else
    {
        file.close();
        delete []rawImage;
        return false;
    }

    file.close();

    if (!reallocate(w, h, 1, 14))
    {
        delete []rawImage;
        return false;
    }

    feedData(rawImage);

    delete []rawImage;

    return true;
}

void ImageData::retrieveEXIFTags(FIBITMAP *img)
{
    haveEXIFMake = false;
    FITAG *make = nullptr;
    FreeImage_GetMetadata(FIMD_EXIF_MAIN, img, "Make", &make);
    if (make != nullptr)
    {
        haveEXIFMake = true;
        EXIFMake = QString((char *)FreeImage_GetTagValue(make));
        FreeImage_GetMetadata(FIMD_EXIF_MAIN, img, "Model", &make);
        if (make != nullptr)
            EXIFMake += " " + QString((char *)FreeImage_GetTagValue(make));
    }

    haveEXIFFirmware = false;
    FITAG *firmware = nullptr;
    FreeImage_GetMetadata(FIMD_EXIF_MAIN, img, "Software", &firmware);
    if (firmware != nullptr)
    {
        haveEXIFFirmware = true;
        EXIFFirmware = QString((char *)FreeImage_GetTagValue(firmware));
    }

    haveEXIFISO = false;
    FITAG *ISO = nullptr;
    FreeImage_GetMetadata(FIMD_EXIF_EXIF, img, "ISOSpeedRatings", &ISO);
    if (ISO != nullptr)
    {
        haveEXIFISO = true;
        EXIFISO = atoi((char *)FreeImage_TagToString(FIMD_EXIF_EXIF, ISO));
    }

    haveEXIFAperture = false;
    FITAG *aperture = nullptr;
    FreeImage_GetMetadata(FIMD_EXIF_EXIF, img, "FNumber", &aperture);
    if (aperture != nullptr)
    {
        haveEXIFAperture = true;
        QString temp = QString((char *)FreeImage_TagToString(FIMD_EXIF_EXIF, aperture));
        temp.remove(0, 1);
        EXIFAperture = temp.toDouble();
    }

    haveEXIFDate = false;
    FITAG *date = nullptr;
    FreeImage_GetMetadata(FIMD_EXIF_EXIF, img, "DateTimeOriginal", &date);
    if (date != nullptr)
    {
        haveEXIFDate = true;
        EXIFDate = QString((char *)FreeImage_TagToString(FIMD_EXIF_EXIF, date));
        // It apperas in YYYY:MM:DD HH:MM:SS format.
        QString subStr(":"); // String to replace.
        QString newStr("-"); // Replacement string.
        EXIFDate.replace(EXIFDate.indexOf(subStr), subStr.size(), newStr);
        EXIFDate.replace(EXIFDate.indexOf(subStr), subStr.size(), newStr);
        cout << "EXIF Date: " << EXIFDate.toStdString() << endl;
    }

    haveEXIFShutter = false;
    FITAG *shutter = nullptr;
    FreeImage_GetMetadata(FIMD_EXIF_EXIF, img, "ExposureTime", &shutter);
    if (shutter != nullptr)
    {
        haveEXIFShutter = true;
        QString temp = QString((char *)FreeImage_TagToString(FIMD_EXIF_MAIN, shutter));
        //FreeImage outputs this as a fraction. We need to calculate the number by finding the "/"
        QStringList fraction = temp.split("/");
        if (fraction.count() != 2)
            haveEXIFShutter = false;
        else
        {
            QString numer = fraction[0];
            QString denom = fraction[1];
            //it appears FreeImage 3.17 adds " sec" to the tail of the exposure.
            denom = denom.left(denom.length() - 4);
            EXIFShutter = numer.toDouble() / denom.toDouble() * 1000;
        }
    }

    haveEXIFTemperatures = false;
    FITAG *makerNote = nullptr;
    FreeImage_GetMetadata(FIMD_EXIF_EXIF, img, "MakerNote", &makerNote);
    if (makerNote != nullptr)
    {
        DWORD tagLen = FreeImage_GetTagLength(makerNote);
        if (tagLen >= 240)
        {
            BYTE *tagBytes = (BYTE *)FreeImage_GetTagValue(makerNote);
            if (
                tagBytes[220] == 24 &&
                tagBytes[224] == 'T' &&
                tagBytes[225] == 'E' &&
                tagBytes[226] == 'M' &&
                tagBytes[227] == 'P')
            {
                haveEXIFTemperatures = true;
                EXIFSensorTemp = tagBytes[228];
                EXIFDSPTemp    = tagBytes[232];
                EXIFBatTemp    = tagBytes[236];
                EXIFPMICTemp   = tagBytes[240];
            }
        }
    }
}

void ImageData::resetEXIFStatus()
{
    haveEXIFMake = false;
    EXIFMake = "";

    haveEXIFFirmware = false;
    EXIFFirmware = "";

    haveEXIFISO = false;
    EXIFISO = 0;

    haveEXIFShutter = false;
    EXIFShutter = 0;

    haveEXIFAperture = false;
    EXIFAperture = 0;

    haveEXIFDate = false;
    EXIFDate = "";

    haveEXIFTemperatures = false;
    EXIFSensorTemp = 0;
    EXIFDSPTemp = 0;
    EXIFBatTemp = 0;
    EXIFPMICTemp = 0;
}

bool ImageData::getEXIFTemperatures(double &sensor, double &dsp, double &bat, double &pmic)
{
    sensor = EXIFSensorTemp;
    dsp = EXIFDSPTemp;
    bat = EXIFBatTemp;
    pmic = EXIFPMICTemp;
    return haveEXIFTemperatures;
}

bool ImageData::getEXIFEV(double &EV)
{
    EV = 0;
    if (!(haveEXIFAperture & haveEXIFShutter & haveEXIFISO)) return false;

    double fNo;
    getEXIFAperture(fNo);
    double shutterms;
    getEXIFShutter(shutterms);
    int ISO;
    getEXIFISO(ISO);

    EV = log(fNo * fNo / (shutterms / 1000)) / log(2.0) + log((double)ISO / 100.0) / log(2.0);
    return (haveEXIFAperture & haveEXIFShutter & haveEXIFISO);
}

bool ImageData::readImage(QString filename)
{
    resetEXIFStatus();

    QStringList chunks = filename.split(".");

    if(chunks.count() < 2)
        return false; //no extension found.

    QString extension = chunks[chunks.count() - 1];
    extension = extension.toUpper();

    //How to convert QString to char*: http://stackoverflow.com/a/5505232/149506
    QByteArray ba = filename.toLatin1();
    const char *filechars = ba.data();

    FIBITMAP *img = nullptr;
    if (extension == "TIF" || extension == "TIFF")
        img = FreeImage_Load(FIF_TIFF, filechars, TIFF_DEFAULT);
    else if (extension == "JPG" || extension == "JPEG")
        img = FreeImage_Load(FIF_JPEG, filechars, JPEG_DEFAULT);
    else if (extension == "BMP")
        img = FreeImage_Load(FIF_BMP, filechars, BMP_DEFAULT);
    else if (extension == "PNG")
        img = FreeImage_Load(FIF_PNG, filechars, PNG_DEFAULT);
    else if (extension == "PSD")
        img = FreeImage_Load(FIF_PSD, filechars, PSD_DEFAULT);
    else if (extension == "RAW")
        return readRAW(filename);

    if(img == nullptr)
        return false;

    //FreeImage loads things upside down...
    FreeImage_FlipVertical(img);

    if (extension == "JPG" || extension == "JPEG")
        retrieveEXIFTags(img);

    FREE_IMAGE_TYPE imageType = FreeImage_GetImageType(img);
    if ( ! (
        imageType == FIT_BITMAP ||
        imageType == FIT_UINT16 ||
        imageType == FIT_RGB16  ||
        imageType == FIT_RGBA16
        ) )
    {
        FreeImage_Unload(img);
        return false;
    }

    int width = (int)FreeImage_GetWidth(img);
    int height = (int)FreeImage_GetHeight(img);

    int numChannels = 1;
    int BPP = 16;
    if (imageType == FIT_RGB16  || imageType == FIT_RGBA16)
    {
        numChannels = 3;
        BPP = 16;
    }
    else if (imageType == FIT_BITMAP)
    {
        unsigned FIBPP = FreeImage_GetBPP(img);
        if (FIBPP == 24 || FIBPP == 32)
        {
            BPP = 8;
            numChannels = 3;
        }
        else if (FIBPP == 1 ||  FIBPP == 4 || FIBPP == 8)
        {
            BPP = FIBPP;
            numChannels = 1;
        }
        else
        {
            FreeImage_Unload(img);
            return false;
        }
    }

    if(!reallocate(width, height, numChannels, BPP))
    {
        FreeImage_Unload(img);
        return false;
    }

    feedData(img, imageType);
    FreeImage_Unload(img);
    return true;
}

void ImageData::recalcMinMax()
{
    resetMinMax();
    for(int i = 0; i < getArea(); i++)
        checkMinMax(data[i], i);

    if (painter != nullptr) painter->signalNewData();
}

void ImageData::checkMinMax(ushort pixValue, int index)
{
    if (pixValue < minValue) {minValue = pixValue; minIndex = index;}
    if (pixValue > maxValue) {maxValue = pixValue; maxIndex = index;}
}

void ImageData::feedData(FIBITMAP *image, FREE_IMAGE_TYPE imgType)
{
    resetMinMax();

    int x, y;

    switch(imgType)
    {
    case FIT_BITMAP:
        {
            unsigned BPP = FreeImage_GetBPP(image);
            if (BPP == 24 || BPP == 32)
            {
                for (y = 0; y < getHeight(); y++)
                {
                    BYTE *pixel = (BYTE *)FreeImage_GetScanLine(image, y);
                    for (x = 0; x < getWidth(); x++)
                    {
                        int i = (y * getWidth() + x) * 3; //remember, we discard the alpha channel.
                        setPixel(i, pixel[FI_RGBA_RED]);
                        checkMinMax(data[i], i);
                        i++;
                        setPixel(i, pixel[FI_RGBA_GREEN]);
                        checkMinMax(data[i], i);
                        i++;
                        setPixel(i, pixel[FI_RGBA_BLUE]);
                        checkMinMax(data[i], i);
                        i++;
                        pixel += (BPP == 32? 4 : 3);
                    }
                }
            }
            else
            {
                //To handle lower BPPs. It appears that, otherwise, the data
                //is bit-packed.
                if (BPP != 8)
                    image = FreeImage_ConvertTo8Bits(image);

                for (y = 0; y < getHeight(); y++)
                {
                    BYTE *rowStart = (BYTE *)FreeImage_GetScanLine(image, y);
                    for (x = 0; x < getWidth(); x++)
                    {
                        int i = y * getWidth() + x;
                        setPixel(i, *(rowStart+x));
                        checkMinMax(data[i], i);
                    }
                }
            }
            break;
        }
    case FIT_UINT16:
        {
            for (y = 0; y < getHeight(); y++)
            {
                WORD *rowStart = (WORD *)FreeImage_GetScanLine(image, y);
                for (x = 0; x < getWidth(); x++)
                {
                    int i = y * getWidth() + x;
                    setPixel(i, *(rowStart+x));
                    checkMinMax(data[i], i);
                }
            }
            break;
        }
    default: break; //do nothing.
    }

    if (painter != nullptr) painter->signalNewData();
}

void ImageData::feedData(ushort *image)
{
    resetMinMax();

    for (int i = 0; i < getArea(); i++)
    {
        setPixel(i, image[i]);
        checkMinMax(data[i], i);
    }

    if (painter != nullptr) painter->signalNewData();
}

void ImageData::feedDataReverse(ushort *image)
{
    resetMinMax();

    for (int i = 0; i < getArea(); i++)
    {
        setPixel(i, image[getArea() - 1 - i]);
        checkMinMax(data[i], i);
    }

    if (painter != nullptr) painter->signalNewData();
}

bool ImageData::resetWhiteBalance()
{
    if (getNumChannels() == 1)
        return resetWhiteBalanceGrey();
    else
        return resetWhiteBalanceColor();
}

bool ImageData::doWhiteBalance(QRect &Area)
{
    cout << "White balance on ROI x: " << Area.x() << " y: " << Area.y() << " w: " << Area.width() << " h: " << Area.height() << endl;
    if (getNumChannels() == 1)
        return doWhiteBalanceGrey(Area);
    else
        return doWhiteBalanceColor(Area);
}

bool ImageData::doWhiteBalanceGrey(QRect &greyArea)
{
    // We force a 2x2 area and do white balance separately on these so that it also works for
    // raw Bayer images.
    if(greyArea.width() < 2 || greyArea.height() < 2)
        return false; //not big enough.

    double avg[2][2];
    int    qty[2][2];

    memset(avg, 0, 4 * sizeof(double));
    memset(qty, 0, 4 * sizeof(int));

    int c, r;

    for (c = 0; c < greyArea.width(); c++)
    {
        int col = greyArea.left() + c;
        for (r = 0; r < greyArea.height(); r++)
        {
            int row = greyArea.top() + r;
            avg[col % 2][row % 2] += getPixel(col, row, 0);
            qty[col % 2][row % 2]++;
        }
    }

    for (c = 0; c < 2; c++)
        for (r = 0; r < 2; r++)
            avg[c][r] /= (double)qty[c][r];

    //Now we have the average values for the pixel "quads".

    double goal = 0;
    for (c = 0; c < 2; c++)
        for (r = 0; r < 2; r++)
            goal += avg[c][r];

    goal /= 4;

    if (goal == 0)
        return false;

    double gains[2][2];

    for (c = 0; c < 2; c++)
        for (r = 0; r < 2; r++)
            gains[c][r] = goal / avg[c][r];

    //Now we have the multipliers. Let's alter the image.
    #pragma omp parallel for private(r)
    for (c = 0; c < getWidth(); c++)
        for (r = 0; r < getHeight(); r++)
        {
            setPixel(c, r, 0, getPixel(c, r, 0)*gains[c % 2][r % 2]);
        }

    //Store the combined gains so as to be able to reset this later.
    greyGains[0][0] *= gains[0][0];
    greyGains[0][1] *= gains[0][1];
    greyGains[1][0] *= gains[1][0];
    greyGains[1][1] *= gains[1][1];

    //Image altered. We need to recalculate min/max.
    //It will also trigger a re-translation/paint of the image if an
    //ImageWindow is assigned as the painter.
    recalcMinMax();

    return true;
}

bool ImageData::resetWhiteBalanceGrey()
{
    #pragma omp parallel
    for (int c = 0; c < getWidth(); c++)
        for (int r = 0; r < getHeight(); r++)
        {
            setPixel(c, r, 0, getPixel(c, r, 0)/greyGains[c % 2][r % 2]);
        }

    //Image altered. We need to recalculate min/max.
    //It will also trigger a re-translation/paint of the image if an
    //ImageWindow is assigned as the painter.
    recalcMinMax();

    // Reset the gains, or continuously "resetting" white balance will actually keep changing the image.
    greyGains[0][0] = 1;
    greyGains[1][0] = 1;
    greyGains[0][1] = 1;
    greyGains[1][1] = 1;

    return true;
}

bool ImageData::doWhiteBalanceColor(QRect &colorArea)
{
    //Did the user select enough pixels?
    if (colorArea.width() < 1 || colorArea.height() < 1)
        return false;

    double avg[3];
    int    qty = 0;
    memset(avg, 0, 3 * sizeof(double));

    int c, r;

    for (c = 0; c < colorArea.width(); c++)
    {
        int col = colorArea.left() + c;
        for (r = 0; r < colorArea.height(); r++)
        {
            int row = colorArea.top() + r;
            avg[0] += getPixel(col, row, 0);
            avg[1] += getPixel(col, row, 1);
            avg[2] += getPixel(col, row, 2);
            qty++;
        }
    }

    avg[0] /= (double)qty;
    avg[1] /= (double)qty;
    avg[2] /= (double)qty;

    double goal = (avg[0] + avg[1] + avg[2]) / 3.0;

    if (goal == 0)
        return false;

    double gains[3];

    gains[0] = goal / avg[0];
    gains[1] = goal / avg[1];
    gains[2] = goal / avg[2];

    //Now we have the multipliers. Let's alter the image.
    #pragma omp parallel for private(r)
    for (c = 0; c < getWidth(); c++)
    {
        for (r = 0; r < getHeight(); r++)
        {
            setPixel(c, r, 0, getPixel(c, r, 0)*gains[0]);
            setPixel(c, r, 1, getPixel(c, r, 1)*gains[1]);
            setPixel(c, r, 2, getPixel(c, r, 2)*gains[2]);
        }
    }

    //Store the combined gains so as to be able to reset this later.
    colorGains[0] *= gains[0];
    colorGains[1] *= gains[1];
    colorGains[2] *= gains[2];

    //Image altered. We need to recalculate min/max.
    //It will also trigger a re-translation/paint of the image if an
    //ImageWindow is assigned as the painter.
    recalcMinMax();

    return true;
}

bool ImageData::resetWhiteBalanceColor()
{
    #pragma omp parallel for
    for (int c = 0; c < getWidth(); c++)
    {
        for (int r = 0; r < getHeight(); r++)
        {
            setPixel(c, r, 0, getPixel(c, r, 0)/colorGains[0]);
            setPixel(c, r, 1, getPixel(c, r, 1)/colorGains[1]);
            setPixel(c, r, 2, getPixel(c, r, 2)/colorGains[2]);
        }
    }

    //Image altered. We need to recalculate min/max.
    //It will also trigger a re-translation/paint of the image if an
    //ImageWindow is assigned as the painter.
    recalcMinMax();

    colorGains[0] = 1;
    colorGains[1] = 1;
    colorGains[2] = 1;

    return true;
}
