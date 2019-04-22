//#include "ImageWindow.h"
#include "MainDialog.h"
#include <QApplication>
#include <omp.h>
#include <QCommandLineParser>
#include "definitions.h"
#include <iostream>

using namespace std;

int main(int argc, char *argv[])
{
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

    parser.process(app);

    for (QString image : parser.positionalArguments())
        cout << "Will attempt to open " << image.toStdString() << " at startup." << endl;

    MainDialog dlg;
    dlg.filesToOpenAtStartup = parser.positionalArguments();

    return dlg.exec();
}
