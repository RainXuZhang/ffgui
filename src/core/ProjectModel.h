#ifndef PROJECTMODEL_H
#define PROJECTMODEL_H

#include <QString>
#include <QList>
#include <QVariantMap>
#include <QDateTime>

struct MediaClip {
    QString id;
    QString filePath;
    QString fileName;
    double duration = 0.0; // in seconds
    int width = 0;
    int height = 0;
    double fps = 25.0;
    QString thumbnailPath;
    bool hasAudio = false;
    bool hasVideo = false;
};

struct ClipEffect {
    QString type; // "crop", "text", "volume", "speed", "brightness"
    QVariantMap params;
    bool enabled = true;
};

struct TimelineClip {
    QString id;
    QString mediaClipId; // References MediaClip::id
    double timelineIn = 0.0; // In-point on timeline (seconds)
    double timelineOut = 0.0; // Out-point on timeline (seconds)
    double sourceIn = 0.0; // In-point in source file (seconds)
    double duration = 0.0; // Duration of this segment (seconds)
    QList<ClipEffect> effects;
    
    double timelineDuration() const { return duration; }
};

struct TimelineTrack {
    int id;
    QString name;
    bool isAudio = false;
    bool isLocked = false;
    bool isMutedOrHidden = false;
    QList<TimelineClip> clips;
};

class Project {
public:
    QString filePath;
    QList<MediaClip> mediaClips;
    QList<TimelineTrack> tracks;
    double duration = 300.0; // Default timeline length: 5 mins
    double currentPlayhead = 0.0; // In seconds
    
    MediaClip* findMediaClip(const QString& id) {
        for (auto& clip : mediaClips) {
            if (clip.id == id) return &clip;
        }
        return nullptr;
    }
    
    TimelineClip* findTimelineClip(const QString& id, int* trackIndexOut = nullptr) {
        for (int i = 0; i < tracks.size(); ++i) {
            for (auto& clip : tracks[i].clips) {
                if (clip.id == id) {
                    if (trackIndexOut) *trackIndexOut = i;
                    return &clip;
                }
            }
        }
        return nullptr;
    }
};

#endif // PROJECTMODEL_H
