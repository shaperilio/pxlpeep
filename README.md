# pxlpeep
A simple image viewer

I started this many moons ago when I needed a way to view floating point images that were the result of doing in-line digital holography. The focus is on minimum GUI with special functionality that I found lacking in other image viewers, i.e., "when you can't find the solution, just add to the problem."

Some features:
- Windows can be synchronized so that all actions taken on one (e.g. zooming, panning) will also happen on all others. I used to do this by stacking images in layers in Photoshop.
- Absolutely no interpolation during zooming. Things will look shitty zoomed in and zoomed out.
- Easy to browse through a directory of images with the arrow keys, and each window sticks to its type (e.g. JPG, PNG).
- Select which color channels to look at (only supports RGB).
- Change the color map to highlight certain aspects (e.g. saturation).
- Change the color function (e.g. logarithmic) to see more shadow detail.
- Snap windows to fractions of the screeen (since some OS's don't have this built in).

Full functionality is shown in a help menu accessible by pressing "H" (or any other key that doesn't have a function attached to it)

Builds out of the box in QtCreator on Ubuntu; presumably builds on a Mac, but I'm not going to bother trying it.

## Dependencies
Requires FreeImage, which can be installed on Ubuntu with 
```
sudo apt install libfreeimage-dev
```

On a mac:
```
brew install freeimage
```

On windows, just [download the package](https://freeimage.sourceforge.io/download.html) and extract the ZIP into a `FreeImageDLL` directory at the same level where you cloned the repo (or you'll have to modify the paths in the project file).

It also requires OpenGL, which Qt seems to include but somehow not link correctly in Ubuntu so then `ld` can't find it. You can brute-force rectify this with

```
sudo apt install freeglut3-dev
```

If on a clean OS, you may need to install `make` and `openMP` as well:

```
sudo apt install build-essential libomp-dev
```

Note that to execute pxlpeep, if you installed Qt from their installer, you need to point it to the correct libraries directory, which by default is in your home folder:

```
LD_LIBRARY_PATH=~/Qt/xxx/gcc_64/lib/ ./pxlpeep
```

where `xxx` is the Qt version; assuming you have installed Qt in its default location.

## Building MacOS Catalina
You have to install OpenMP.

```
brew install libomp
```

## Use
To be able to access pxlpeep from the "Open With" context menu in Unity, you'll need a `.desktop` file. There's one in the repo which works with my machine. You'll have to at least edit the location of the files (must be full absolute paths, e.g. `~` does not get resolved). Basically:

```
[Desktop Entry]
Name=pxlpeep
GenericName=Image Viewer
X-GNOME-FullName=pxlpeep Image Viewer
Comment=Look at it. LOOK AT IT.
Exec=<wherever you cloned the repo>/build-pxlpeep-Desktop_Qt_5_12_1_GCC_64bit-Release/pxlpeep %F
Icon=<wherever you cloned the repo>/loupe.ico
Terminal=true
Type=Application
MimeType=image/bmp;image/x-psd;image/tiff;image/jpeg;image/png;
```

Install the file with `desktop-file-install` which will also validate that it's all fine. More details [here](https://help.ubuntu.com/community/UnityLaunchersAndDesktopFiles). 

In Windows, you'll need to use Qt's deployment tool to copy all the necessary files to the build directory. You'll also have to copy `FreeImage.dll` into that directory.

While inside the directory containing `pxlpeep.exe`, do

```
C:\Qt\xxx\msvc2019_64\bin\windeployqt.exe .
```

where `xxx` is the Qt version; assuming you have installed Qt in its default location.
