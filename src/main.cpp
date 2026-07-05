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

// Apply dark theme stylesheet
QString darkStyleSheet = R"(
    QMainWindow, QDialog {
        background-color: #1e1e2e;
    }

    QLineEdit, QComboBox, QTextEdit {
        background-color: #11111b;
        color: #cdd6f4;
        border-radius: 4px;
    }

    QPushButton {
        background-color: #313244;
        color: #cdd6f4;
        border-radius: 4px;
    }

    QPushButton:hover, QPushButton:pressed {
        background-color: #89b4fa;
    }

    QProgressBar {
        background-color: #11111b;
        border-radius: 4px;
    }

    QProgressBar::chunk {
        background-color: #89b4fa;
        border-radius: 4px;
    }
)";
mainWindow.setStyleSheet(darkStyleSheet);
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