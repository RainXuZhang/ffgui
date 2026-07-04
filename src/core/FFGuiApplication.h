#ifndef FFGUIAPPLICATION_H
#define FFGUIAPPLICATION_H

#include <QApplication>
#include <QSettings>
#include <QTranslator>
#include <QLocale>

class FFGuiApplication : public QApplication
{
    Q_OBJECT

public:
    explicit FFGuiApplication(int &argc, char **argv);
    ~FFGuiApplication() override;

    void applyDarkTheme();
    void applyLightTheme();
    void loadSettings();
    void saveSettings();
    
    static FFGuiApplication* instance();
    static void setInstance(FFGuiApplication* app);

private:
    QSettings* m_settings;
    QTranslator* m_translator;
    bool m_darkTheme;
};

#endif // FFGUIAPPLICATION_H