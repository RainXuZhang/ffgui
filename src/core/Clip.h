#ifndef CLIP_H
#define CLIP_H

#include <QString>

class Clip {
public:
    Clip() = default;
    ~Clip() = default;

    QString getId() const;
    void setId(const QString& id);

    double getStartTime() const;
    void setStartTime(double time);

    double getDuration() const;
    void setDuration(double duration);

    QString getMediaPath() const;
    void setMediaPath(const QString& path);

private:
    QString m_id;
    double m_startTime = 0.0;
    double m_duration = 0.0;
    QString m_mediaPath;
};

#endif // CLIP_H