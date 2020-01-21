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

It also requires OpenGL, which Qt seems to include but somehow not link correctly so then `ld` can't find it. You can brute-force rectify this with
```
sudo apt install freeglut3-dev
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
