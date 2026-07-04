#include "FFGuiApplication.h"
#include <QApplication>
#include <QStyle>
#include <QPalette>
#include <QColor>
#include <QFile>
#include <QDebug>

static FFGuiApplication* s_instance = nullptr;

FFGuiApplication* FFGuiApplication::instance()
{
    return s_instance;
}

void FFGuiApplication::setInstance(FFGuiApplication* app)
{
    s_instance = app;
}

FFGuiApplication::FFGuiApplication(int &argc, char **argv)
    : QApplication(argc, argv)
    , m_settings(nullptr)
    , m_translator(nullptr)
    , m_darkTheme(true)
{
    m_settings = new QSettings("FFGui", "FFGui");
    loadSettings();
}

FFGuiApplication::~FFGuiApplication()
{
    saveSettings();
    delete m_settings;
    delete m_translator;
}

void FFGuiApplication::applyDarkTheme()
{
    m_darkTheme = true;
    
    QPalette palette;
    
    // Dark theme colors (Kdenlive-like)
    const QColor windowColor(53, 53, 53);
    const QColor baseColor(35, 35, 35);
    const QColor textColor(210, 210, 210);
    const QColor highlightColor(42, 130, 218);
    const QColor disabledColor(127, 127, 127);
    
    palette.setColor(QPalette::Window, windowColor);
    palette.setColor(QPalette::WindowText, textColor);
    palette.setColor(QPalette::Base, baseColor);
    palette.setColor(QPalette::AlternateBase, windowColor);
    palette.setColor(QPalette::ToolTipBase, textColor);
    palette.setColor(QPalette::ToolTipText, textColor);
    palette.setColor(QPalette::Text, textColor);
    palette.setColor(QPalette::Button, windowColor);
    palette.setColor(QPalette::ButtonText, textColor);
    palette.setColor(QPalette::Disabled, QPalette::WindowText, disabledColor);
    palette.setColor(QPalette::Disabled, QPalette::Text, disabledColor);
    palette.setColor(QPalette::Disabled, QPalette::ButtonText, disabledColor);
    palette.setColor(QPalette::Highlight, highlightColor);
    palette.setColor(QPalette::HighlightedText, Qt::white);
    
    setPalette(palette);
    
    // Load dark stylesheet
    QFile styleFile(":/styles/dark.qss");
    if (styleFile.open(QFile::ReadOnly)) {
        setStyleSheet(styleFile.readAll());
    }
}

void FFGuiApplication::applyLightTheme()
{
    m_darkTheme = false;
    // Reset to default
    setPalette(QApplication::style()->standardPalette());
    setStyleSheet("");
}

void FFGuiApplication::loadSettings()
{
    m_darkTheme = m_settings->value("darkTheme", true).toBool();
}

void FFGuiApplication::saveSettings()
{
    m_settings->setValue("darkTheme", m_darkTheme);
}