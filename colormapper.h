#ifndef COLORMAPPER_H
#define COLORMAPPER_H

#include <QString>
#include <math.h>
enum ColormapPalette : int
{
    Grey,
    GreyInverted,
    GreySaturationWarning,
    GreyInvertedSaturationWarning,
    ColorExpansion,
    Colormap1,
    NUM_PALETTES //used as a place holder to count these.
};

//in the same order as ColormapPalette
static int ColormapPaletteSizes[ColormapPalette::NUM_PALETTES] = {256, 256, 256, 256, 256*256*256, 256*256*256};
static QString ColormapPaletteNames[ColormapPalette::NUM_PALETTES] = {
    "Grey",
    "Inv. grey",
    "Grey w/ saturation",
    "Inv. grey w/ saturation",
    "Color expansion",
    "Colormap 1"
};

class Colormapper
{
public:
    Colormapper();
    Colormapper(ColormapPalette thePalette);
    ~Colormapper();

    bool setColormap(ColormapPalette val);
    ColormapPalette getColormap() {return this->colormap;}
    QString getColormapName() {return ColormapPaletteNames[colormap];}

    bool translatePixel(uchar *destination, double source);
    bool translatePixelMultiChan(uchar *destination, double source0, double source1, double source2);
    int getMaxPaletteValue() {return ColormapPaletteSizes[colormap] - 1;}

protected:
    void constructorCore();

    uchar *currentPalette;
    uchar *palettes[ColormapPalette::NUM_PALETTES];
    ColormapPalette colormap;

    //The following are used to generate pallettes.
    void greySaturationWarning(uchar *dstPixel, double srcPixel);
    void colormap1(uchar *dstPixel, double srcPixel);
    void interpolate(double inValue, int *bottoqMindex, int *topIndex, double *bottomFrac);
    void greyscale(uchar *dstPixel, double srcPixel);
    void simpleExpansion(uchar *dstPixel, double srcPixel);
};

//If we want to define functions here, they must be inline; otherwise, we need to move them to a cpp file.

inline bool Colormapper::translatePixel(uchar *destination, double source)
{
    if (currentPalette == nullptr) return false;

    //int val = floor(source + .5);	//round the value. This is very expensive!
    //int val = (int)(source + .5); //much faster than using floor.
    int val = (int)(source);
    val = qMax(qMin(val, ColormapPaletteSizes[colormap] - 1), 0);	//clamp the value.
    memcpy(destination, &(currentPalette[val * 4]), 4);				//alpha matters in Qt.

    return true;
}

inline bool Colormapper::translatePixelMultiChan(uchar *destination, double source0, double source1, double source2)
{
    if (currentPalette == nullptr) return false;

    int val0 = (int)(source0);
    int val1 = (int)(source1);
    int val2 = (int)(source2);

    int paletteSize = ColormapPaletteSizes[colormap];
    val0 = qMax(qMin(val0, paletteSize - 1), 0);
    val1 = qMax(qMin(val1, paletteSize - 1), 0);
    val2 = qMax(qMin(val2, paletteSize - 1), 0);

    //Note the order!
    destination[2] = currentPalette[val0 * 4];
    destination[1] = currentPalette[val1 * 4];
    destination[0] = currentPalette[val2 * 4];
    destination[3] = 255; //alpha matters in Qt.
    return true;
}

inline void Colormapper::constructorCore()
{
    //Create and fill all the palettes.
    int k;

    #pragma omp parallel for //probably a waste?
    for (k = 0; k < ColormapPalette::NUM_PALETTES; k++)
    {
        int paletteSize = ColormapPaletteSizes[k];
        palettes[k] = new uchar[paletteSize * 4];
        if (palettes[k] == nullptr)
        {
//            Output::Error(L"Colormapper: palette could not be created.\n");
            continue;
        }
        ColormapPalette colormap = (ColormapPalette)k;
        int i, j;

        for (i = 0, j = 0; i < paletteSize; i++, j += 4)
        {
            uchar *dst = &(palettes[k][j]);
            switch(colormap)
            {
            case ColormapPalette::Grey:
                greyscale(dst, i); break;
            case ColormapPalette::GreyInverted:
                greyscale(dst, 255 - i); break;
            case ColormapPalette::GreySaturationWarning:
                greySaturationWarning(dst, i); break;
            case ColormapPalette::GreyInvertedSaturationWarning:
                greySaturationWarning(dst, 255 - i); break;
            case ColormapPalette::ColorExpansion:
                simpleExpansion(dst, i); break;
            case ColormapPalette::Colormap1:
                colormap1(dst, i); break;
            default:
                greyscale(dst, i); break;
            }
        }
    }
}

inline Colormapper::Colormapper()
{
    constructorCore();
    this->currentPalette = palettes[ColormapPalette::Grey]; //ensure color map is set.
}

inline Colormapper::Colormapper(ColormapPalette thePalette)
{
    constructorCore();
    this->currentPalette = palettes[ColormapPalette::Grey];
    this->setColormap(thePalette);
}

inline Colormapper::~Colormapper()
{
    int k;
    for (k = 0; k < ColormapPalette::NUM_PALETTES; k++)
        delete [] palettes[k];
}

inline bool Colormapper::setColormap(ColormapPalette val)
{
    if (val >= ColormapPalette::NUM_PALETTES)
        return false;
    this->colormap = val;
    this->currentPalette = palettes[colormap];
    return true;
}

inline void Colormapper::simpleExpansion(uchar *dstPixel, double srcPixel)
{
    srcPixel = qMin(qMax(0., srcPixel), 256.*256*256-1);
    uint pixel = (uint)srcPixel;
    dstPixel[0] = (uchar)((pixel & 0x000000FF) >>  0); //blue
    dstPixel[1] = (uchar)((pixel & 0x0000FF00) >>  8); //green
    dstPixel[2] = (uchar)((pixel & 0x00FF0000) >> 16); //red
    dstPixel[3] = 255;	//alpha
}

inline void Colormapper::greyscale(uchar *dstPixel, double srcPixel)
{
    srcPixel = qMin(qMax(0., srcPixel), 255.);
    uchar pixel = (uchar)srcPixel;
    dstPixel[0] = pixel; //blue
    dstPixel[1] = pixel; //green
    dstPixel[2] = pixel; //red
    dstPixel[3] = 255;	 //alpha
}

inline void Colormapper::colormap1(uchar *dstPixel, double srcPixel)
{   //		          Black|Purple|Blue|Green|Magenta|Red|Yellow|White
    const uchar R[] = {    0,   128,   0,    0,    255,255,   255,  255};
    const uchar G[] = {    0,     0,   0,  128,      0,  0,   255,  255};
    const uchar B[] = {    0,   128, 255,    0,    255,  0,     0,  255};
    const int nColors = 8;

    double index = srcPixel / (double)(ColormapPaletteSizes[ColormapPalette::Colormap1] - 1) * (nColors - 1);
    index = qMin(qMax(0., index), nColors - 1.);
    double f;
    int l, t;
    interpolate(index, &l, &t, &f);

    uchar Rpix = (uchar)(R[l]*f + R[t]*(1-f));
    uchar Gpix = (uchar)(G[l]*f + G[t]*(1-f));
    uchar Bpix = (uchar)(B[l]*f + B[t]*(1-f));
    dstPixel[0] = Bpix;
    dstPixel[1] = Gpix;
    dstPixel[2] = Rpix;
    dstPixel[3] = 255;	//alpha
}

inline void Colormapper::greySaturationWarning(uchar *dstPixel, double srcPixel)
{
    dstPixel[3] = 255;	//alpha

    if (srcPixel - 0 <= 0) //saturated on bottom end
    {
        dstPixel[0] = 255; //blue
        dstPixel[1] = 0; //green
        dstPixel[2] = 0; //red
    }
    else if (srcPixel - 0 < 0.05 * 255) //bottom end warning
    {
        dstPixel[0] = 255; //blue
        dstPixel[1] = 128; //green
        dstPixel[2] = 128; //red
    }
    else if (srcPixel - 255 >= 0) //saturated on top end
    {
        dstPixel[0] = 0; //blue
        dstPixel[1] = 0; //green
        dstPixel[2] = 255; //red
    }
    else if (255 - srcPixel < 0.05 * 255) //top end warning
    {
        dstPixel[0] = 128; //blue
        dstPixel[1] = 128; //green
        dstPixel[2] = 255; //red
    }
    else
    {
        srcPixel = qMin(qMax(0., srcPixel), 255.);
        uchar pixel = (uchar)srcPixel;
        dstPixel[0] = pixel; //blue
        dstPixel[1] = pixel; //green
        dstPixel[2] = pixel; //red
    }
}

inline void Colormapper::interpolate(double inValue, int *bottoqMindex, int *topIndex, double *bottomFrac)
{
    *bottoqMindex = (int)floor(inValue);
    if (*bottoqMindex == inValue)
    {
        *bottomFrac = 1;
        *topIndex = *bottoqMindex;
        return;
    }

    *topIndex = (int)ceil(inValue);
    if (*topIndex == inValue)
    {
        *bottomFrac = 0;
        *bottoqMindex = *topIndex;
        return;
    }

    *bottomFrac = (*topIndex - inValue) / (double)(*topIndex - *bottoqMindex);
}

#endif // COLORMAPPER_H

