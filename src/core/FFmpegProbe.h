#ifndef FFMPEGPROBE_H
#define FFMPEGPROBE_H

#include "ProjectModel.h"
#include <QString>

class FFmpegProbe {
public:
    static bool probeFile(const QString& filePath, MediaClip& outClip);
    static QString generateThumbnail(const QString& videoPath, double timeSeconds);
};

#endif // FFMPEGPROBE_H
