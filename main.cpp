//#include "ImageWindow.h"
#include "MainDialog.h"
#include <QApplication>
#include <omp.h>
#include <QCommandLineParser>
#include "definitions.h"
#include <iostream>

#include <stdio.h>

using namespace std;
void redirectOutput(char const * const filename) {
    FILE *output_file;
    freopen_s(&output_file, filename, "w", stdout);
    freopen_s(&output_file, filename, "w", stderr);
    printf("printf works\n");
    cout << "cout works\n";
}

int main(int argc, char *argv[])
{
//    redirectOutput("output.txt");

#ifdef _DEBUG
    omp_set_num_threads(1);
#else
    omp_set_num_threads(8);
#endif

    QApplication app(argc, argv);
    app.setApplicationName("pxlpeep");
    app.setOrganizationName("shaperilio");
    app.setOrganizationDomain("https://github.com/shaperilio");
    app.setApplicationVersion(VERSION_STRING);

    QCommandLineParser parser;
    parser.setApplicationDescription("pxlpeep image viewer");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("image(s)", "the image(s) file to open");
    parser.addOption(QCommandLineOption("f", "fill the screen with the images to open"));
    parser.addOption(QCommandLineOption("s", "sync open windows"));

    parser.process(app);

    for (QString const &image : parser.positionalArguments())
        cout << "Will attempt to open " << image.toStdString() << " at startup." << endl;

    MainDialog dlg;
    dlg.filesToOpenAtStartup = parser.positionalArguments();
    dlg.fillScreenAtStartup = parser.optionNames().contains("f");
    dlg.syncOpenWindowsAtStartup = parser.optionNames().contains("s");

    return dlg.exec();
}
