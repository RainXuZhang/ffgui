#ifndef THEME_MANAGER_H
#define THEME_MANAGER_H

#include <QApplication>
#include <QPalette>
#include <QSettings>

/**
 * @brief ThemeManager – a lightweight utility class for handling application
 *        themes. It provides an enum of available themes, a method to apply a
 *        theme to a QApplication (setting both a QPalette and a custom style
 *        sheet), and helpers to persist the selected theme with QSettings.
 */
class ThemeManager {
public:
    /**
     * Available themes. The integer values are stable because they are stored
     * directly in QSettings.
     */
    enum AppTheme {
        Dark = 0,
        Light = 1,
        Kdenlive = 2,
    };

    /**
     * Apply the given theme to the supplied QApplication.
     * @param theme The theme to apply.
     * @param app   Pointer to the QApplication instance.
     */
    static void applyTheme(AppTheme theme, QApplication* app);

    /**
     * Persist the selected theme using QSettings.
     * @param theme The theme to store.
     */
    static void saveTheme(AppTheme theme);

    /**
     * Load the previously persisted theme. If none is stored, defaults to
     * Dark.
     * @return The loaded theme.
     */
    static AppTheme loadTheme();
};

#endif // THEME_MANAGER_H