#include "core/FFGuiApplication.h"
#include "MainWindow.h"
#include <QApplication>
#include <QStyleFactory>
#include <QSurfaceFormat>
#include <QCommandLineParser>
#include <QDir>
#include <QStandardPaths>
#include <QDebug>

int main(int argc, char *argv[])
{
    // High DPI scaling is always enabled in Qt6, no need to set attributes
    
    // Set OpenGL format for hardware acceleration
    QSurfaceFormat format;
    format.setDepthBufferSize(24);
    format.setStencilBufferSize(8);
    format.setVersion(3, 3);
    format.setProfile(QSurfaceFormat::CoreProfile);
    format.setSamples(4); // MSAA
    QSurfaceFormat::setDefaultFormat(format);

    FFGuiApplication app(argc, argv);
    FFGuiApplication::setInstance(&app);
    app.setApplicationName("FFGui");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("FFGui Project");
    app.setOrganizationDomain("ffgui.org");

    // Parse command line arguments
    QCommandLineParser parser;
    parser.setApplicationDescription("FFGui - Professional FFmpeg GUI Frontend");
    parser.addHelpOption();
    parser.addVersionOption();
    
    QCommandLineOption projectOption(QStringList() << "p" << "project",
                                     "Open project file", "file");
    parser.addOption(projectOption);
    
    QCommandLineOption mediaOption(QStringList() << "m" << "media",
                                   "Add media file to project", "file");
    parser.addOption(mediaOption);
    
    parser.process(app);

    // Apply Kdenlive-like dark theme
    app.applyDarkTheme();

    // Create and show main window
    MainWindow mainWindow;
    
    // Handle command line arguments
    if (parser.isSet(projectOption)) {
        mainWindow.openProject(parser.value(projectOption));
    }
    
    if (parser.isSet(mediaOption)) {
        mainWindow.addMediaFiles({parser.value(mediaOption)});
    }

    mainWindow.show();
    
    return app.exec();
}