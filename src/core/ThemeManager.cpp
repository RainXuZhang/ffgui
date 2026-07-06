#include "ThemeManager.h"

// Helper function to build a stylesheet string for the given theme.
static QString buildStyleSheet(ThemeManager::AppTheme theme) {
    switch (theme) {
    case ThemeManager::Dark:
        return QStringLiteral(
            "QWidget { background-color: #2b2b2b; color: #dcdcdc; }"
            "QPushButton { background-color: #3c3c3c; border: 1px solid #555; }"
        );
    case ThemeManager::Light:
        return QStringLiteral(
            "QWidget { background-color: #f0f0f0; color: #202020; }"
            "QPushButton { background-color: #e0e0e0; border: 1px solid #aaa; }"
        );
    case ThemeManager::Kdenlive:
        // Kdenlive style approximates the default KDE Breeze dark theme.
        return QStringLiteral(
            "QWidget { background-color: #1e1e1e; color: #eff0f1; }"
            "QPushButton { background-color: #2e2e2e; border: 1px solid #4e4e4e; }"
        );
    default:
        return QString();
    }
}

void ThemeManager::applyTheme(AppTheme theme, QApplication* app) {
    if (!app) return;

    // Create a palette appropriate for the chosen theme.
    QPalette palette;
    switch (theme) {
    case Dark:
        palette.setColor(QPalette::Window, QColor("#2b2b2b"));
        palette.setColor(QPalette::WindowText, QColor("#dcdcdc"));
        palette.setColor(QPalette::Base, QColor("#313131"));
        palette.setColor(QPalette::AlternateBase, QColor("#2b2b2b"));
        palette.setColor(QPalette::ToolTipBase, QColor("#ffffff"));
        palette.setColor(QPalette::ToolTipText, QColor("#000000"));
        palette.setColor(QPalette::Text, QColor("#dcdcdc"));
        palette.setColor(QPalette::Button, QColor("#3c3c3c"));
        palette.setColor(QPalette::ButtonText, QColor("#dcdcdc"));
        palette.setColor(QPalette::BrightText, QColor("#ff0000"));
        break;
    case Light:
        palette.setColor(QPalette::Window, QColor("#f0f0f0"));
        palette.setColor(QPalette::WindowText, QColor("#202020"));
        palette.setColor(QPalette::Base, QColor("#ffffff"));
        palette.setColor(QPalette::AlternateBase, QColor("#eaeaea"));
        palette.setColor(QPalette::ToolTipBase, QColor("#ffffff"));
        palette.setColor(QPalette::ToolTipText, QColor("#000000"));
        palette.setColor(QPalette::Text, QColor("#202020"));
        palette.setColor(QPalette::Button, QColor("#e0e0e0"));
        palette.setColor(QPalette::ButtonText, QColor("#202020"));
        palette.setColor(QPalette::BrightText, QColor("#ff0000"));
        break;
    case Kdenlive:
        palette.setColor(QPalette::Window, QColor("#1e1e1e"));
        palette.setColor(QPalette::WindowText, QColor("#eff0f1"));
        palette.setColor(QPalette::Base, QColor("#2b2b2b"));
        palette.setColor(QPalette::AlternateBase, QColor("#1e1e1e"));
        palette.setColor(QPalette::ToolTipBase, QColor("#ffffff"));
        palette.setColor(QPalette::ToolTipText, QColor("#000000"));
        palette.setColor(QPalette::Text, QColor("#eff0f1"));
        palette.setColor(QPalette::Button, QColor("#2e2e2e"));
        palette.setColor(QPalette::ButtonText, QColor("#eff0f1"));
        palette.setColor(QPalette::BrightText, QColor("#ff0000"));
        break;
    }

    app->setPalette(palette);
    app->setStyleSheet(buildStyleSheet(theme));
}

void ThemeManager::saveTheme(AppTheme theme) {
    QSettings settings;
    settings.setValue("ThemeManager/selectedTheme", static_cast<int>(theme));
}

ThemeManager::AppTheme ThemeManager::loadTheme() {
    QSettings settings;
    int stored = settings.value("ThemeManager/selectedTheme", static_cast<int>(Dark)).toInt();
    switch (stored) {
    case 0: return Dark;
    case 1: return Light;
    case 2: return Kdenlive;
    default: return Dark;
    }
}
