#include "MonitorWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QStyle>
#include <QIcon>
#include <QUrl>
#include <QTime>

MonitorWidget::MonitorWidget(const QString& title, QWidget* parent)
    : QWidget(parent)
    , m_title(title)
{
    // Configure layout
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(4, 4, 4, 4);
    mainLayout->setSpacing(2);

    // Title label (looks like Kdenlive dock tab/header)
    m_titleLabel = new QLabel(title, this);
    m_titleLabel->setStyleSheet(
        "QLabel {"
        "    font-weight: bold;"
        "    color: #a0a0a0;"
        "    background-color: #252525;"
        "    padding: 3px;"
        "    border-bottom: 1px solid #1a1a1a;"
        "}"
    );
    mainLayout->addWidget(m_titleLabel);

    // Video display
    m_videoWidget = new QVideoWidget(this);
    m_videoWidget->setStyleSheet("background-color: black; border: 1px solid #151515;");
    mainLayout->addWidget(m_videoWidget, 1); // Expand to fill space

    // Seek Slider
    m_slider = new QSlider(Qt::Horizontal, this);
    m_slider->setRange(0, 1000);
    m_slider->setStyleSheet(
        "QSlider::groove:horizontal {"
        "    height: 6px;"
        "    background: #333333;"
        "    border-radius: 3px;"
        "}"
        "QSlider::sub-page:horizontal {"
        "    background: #2a82da;"
        "    border-radius: 3px;"
        "}"
        "QSlider::handle:horizontal {"
        "    background: #e0e0e0;"
        "    width: 12px;"
        "    margin-top: -3px;"
        "    margin-bottom: -3px;"
        "    border-radius: 6px;"
        "}"
    );
    connect(m_slider, &QSlider::sliderMoved, this, &MonitorWidget::onSliderValueChanged);
    connect(m_slider, &QSlider::sliderPressed, this, [this]() { m_isSeeking = true; });
    connect(m_slider, &QSlider::sliderReleased, this, [this]() { m_isSeeking = false; });
    mainLayout->addWidget(m_slider);

    // Controls layout
    auto* controlsLayout = new QHBoxLayout();
    controlsLayout->setContentsMargins(2, 2, 2, 2);
    controlsLayout->setSpacing(4);

    m_playButton = new QPushButton(this);
    m_playButton->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
    m_playButton->setFixedSize(30, 26);
    connect(m_playButton, &QPushButton::clicked, this, &MonitorWidget::onPlayPauseClicked);
    controlsLayout->addWidget(m_playButton);

    m_stopButton = new QPushButton(this);
    m_stopButton->setIcon(style()->standardIcon(QStyle::SP_MediaStop));
    m_stopButton->setFixedSize(30, 26);
    connect(m_stopButton, &QPushButton::clicked, this, &MonitorWidget::onStopClicked);
    controlsLayout->addWidget(m_stopButton);

    m_prevFrameButton = new QPushButton("<", this);
    m_prevFrameButton->setFixedSize(26, 26);
    m_prevFrameButton->setToolTip("Previous Frame");
    connect(m_prevFrameButton, &QPushButton::clicked, this, [this]() {
        seek(currentPosition() - 0.04);
    });
    controlsLayout->addWidget(m_prevFrameButton);

    m_nextFrameButton = new QPushButton(">", this);
    m_nextFrameButton->setFixedSize(26, 26);
    m_nextFrameButton->setToolTip("Next Frame");
    connect(m_nextFrameButton, &QPushButton::clicked, this, [this]() {
        seek(currentPosition() + 0.04);
    });
    controlsLayout->addWidget(m_nextFrameButton);

    // Spacer
    controlsLayout->addStretch();

    // Timecode
    m_timeLabel = new QLabel("00:00:00.00 / 00:00:00.00", this);
    m_timeLabel->setStyleSheet("font-family: monospace; font-size: 11px; color: #a0a0a0;");
    controlsLayout->addWidget(m_timeLabel);

    mainLayout->addLayout(controlsLayout);

    // Initialize media player
    m_player = new QMediaPlayer(this);
    m_audioOutput = new QAudioOutput(this);
    m_player->setAudioOutput(m_audioOutput);
    m_player->setVideoOutput(m_videoWidget);

    connect(m_player, &QMediaPlayer::positionChanged, this, &MonitorWidget::onPlayerPositionChanged);
    connect(m_player, &QMediaPlayer::durationChanged, this, &MonitorWidget::onPlayerDurationChanged);
}

MonitorWidget::~MonitorWidget() {
    m_player->stop();
}

void MonitorWidget::setMedia(const QString& filePath) {
    m_player->setSource(QUrl::fromLocalFile(filePath));
    m_timeLabel->setText("00:00:00.00 / 00:00:00.00");
}

void MonitorWidget::play() {
    m_player->play();
    m_playButton->setIcon(style()->standardIcon(QStyle::SP_MediaPause));
}

void MonitorWidget::pause() {
    m_player->pause();
    m_playButton->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
}

void MonitorWidget::stop() {
    m_player->stop();
    m_playButton->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
    seek(0);
}

void MonitorWidget::seek(double seconds) {
    m_player->setPosition(static_cast<qint64>(seconds * 1000.0));
}

double MonitorWidget::duration() const {
    return m_player->duration() / 1000.0;
}

double MonitorWidget::currentPosition() const {
    return m_player->position() / 1000.0;
}

void MonitorWidget::onPlayPauseClicked() {
    if (m_player->playbackState() == QMediaPlayer::PlayingState) {
        pause();
    } else {
        play();
    }
}

void MonitorWidget::onStopClicked() {
    stop();
}

void MonitorWidget::onSliderValueChanged(int value) {
    if (m_isSeeking) {
        double seconds = (value / 1000.0) * duration();
        seek(seconds);
    }
}

void MonitorWidget::onPlayerPositionChanged(qint64 position) {
    double posSec = position / 1000.0;
    double durSec = duration();
    
    if (!m_isSeeking && durSec > 0.0) {
        m_slider->setValue(static_cast<int>((posSec / durSec) * 1000.0));
    }
    
    updateTimeLabel(posSec, durSec);
    emit positionChanged(posSec);
}

void MonitorWidget::onPlayerDurationChanged(qint64 duration) {
    double durSec = duration / 1000.0;
    updateTimeLabel(currentPosition(), durSec);
    emit durationChanged(durSec);
}

void MonitorWidget::updateTimeLabel(double current, double total) {
    auto formatTime = [](double seconds) -> QString {
        int h = static_cast<int>(seconds) / 3600;
        int m = (static_cast<int>(seconds) % 3600) / 60;
        int s = static_cast<int>(seconds) % 60;
        int ms = static_cast<int>((seconds - static_cast<int>(seconds)) * 100.0);
        return QString("%1:%2:%3.%4")
            .arg(h, 2, 10, QChar('0'))
            .arg(m, 2, 10, QChar('0'))
            .arg(s, 2, 10, QChar('0'))
            .arg(ms, 2, 10, QChar('0'));
    };

    m_timeLabel->setText(QString("%1 / %2").arg(formatTime(current)).arg(formatTime(total)));
}
