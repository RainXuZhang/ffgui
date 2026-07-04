#ifndef MONITORWIDGET_H
#define MONITORWIDGET_H

#include <QWidget>
#include <QMediaPlayer>
#include <QVideoWidget>
#include <QAudioOutput>
#include <QPushButton>
#include <QSlider>
#include <QLabel>

class MonitorWidget : public QWidget {
    Q_OBJECT

public:
    explicit MonitorWidget(const QString& title, QWidget* parent = nullptr);
    ~MonitorWidget() override;

    void setMedia(const QString& filePath);
    void play();
    void pause();
    void stop();
    void seek(double seconds);
    double duration() const;
    double currentPosition() const;
    QVideoWidget* getVideoWidget() const { return m_videoWidget; }

signals:
    void positionChanged(double seconds);
    void durationChanged(double seconds);

private slots:
    void onPlayPauseClicked();
    void onStopClicked();
    void onSliderValueChanged(int value);
    void onPlayerPositionChanged(qint64 position);
    void onPlayerDurationChanged(qint64 duration);

private:
    void updateTimeLabel(double current, double total);

    QString m_title;
    QMediaPlayer* m_player;
    QVideoWidget* m_videoWidget;
    QAudioOutput* m_audioOutput;
    
    QPushButton* m_playButton;
    QPushButton* m_stopButton;
    QPushButton* m_prevFrameButton;
    QPushButton* m_nextFrameButton;
    QSlider* m_slider;
    QLabel* m_timeLabel;
    QLabel* m_titleLabel;
    
    bool m_isSeeking = false;
};

#endif // MONITORWIDGET_H
