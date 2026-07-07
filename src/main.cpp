#include "core/FFGuiApplication.h"
#include "MainWindow.h"
#include "core/ThemeManager.h"
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

    QApplication app(argc, argv);
    app.setApplicationName("FFGui");
    app.setOrganizationName("RainZhang");

    // Read the saved theme from QSettings immediately after QApplication is created
    ThemeManager::AppTheme theme = ThemeManager::loadTheme();
    // Call ThemeManager::instance().applyTheme() before MainWindow is instantiated
    ThemeManager::applyTheme(theme, &app);

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
