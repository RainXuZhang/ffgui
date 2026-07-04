#include "FFmpegProbe.h"
#include <QProcess>
#include <QFileInfo>
#include <QDir>
#include <QTemporaryFile>
#include <QStandardPaths>
#include <QDebug>
#include <QUuid>

bool FFmpegProbe::probeFile(const QString& filePath, MediaClip& outClip) {
    QFileInfo fileInfo(filePath);
    if (!fileInfo.exists()) return false;

    outClip.filePath = filePath;
    outClip.fileName = fileInfo.fileName();
    outClip.id = QUuid::createUuid().toString(QUuid::WithoutBraces);

    // Call ffprobe to get file details
    QProcess ffprobe;
    QStringList arguments;
    arguments << "-v" << "error"
              << "-show_entries" << "format=duration"
              << "-show_entries" << "stream=width,height,avg_frame_rate,codec_type"
              << "-of" << "csv=p=0"
              << filePath;

    ffprobe.start("ffprobe", arguments);
    if (!ffprobe.waitForFinished(3000)) {
        return false;
    }

    QString output = QString::fromUtf8(ffprobe.readAllStandardOutput()).trimmed();
    QStringList lines = output.split('\n');

    outClip.hasVideo = false;
    outClip.hasAudio = false;

    for (const QString& line : lines) {
        QStringList parts = line.split(',');
        if (parts.isEmpty()) continue;

        QString type = parts[0].trimmed();
        if (type == "video") {
            outClip.hasVideo = true;
            if (parts.size() >= 4) {
                outClip.width = parts[1].toInt();
                outClip.height = parts[2].toInt();
                
                // Parse frame rate (e.g., "30000/1001" or "25/1")
                QString fpsStr = parts[3].trimmed();
                if (fpsStr.contains('/')) {
                    QStringList fpsParts = fpsStr.split('/');
                    double num = fpsParts[0].toDouble();
                    double den = fpsParts[1].toDouble();
                    if (den != 0) {
                        outClip.fps = num / den;
                    }
                } else {
                    outClip.fps = fpsStr.toDouble();
                }
            }
        } else if (type == "audio") {
            outClip.hasAudio = true;
        } else if (parts.size() >= 1 && outClip.duration == 0.0) {
            // Might be the format duration line
            bool ok;
            double dur = parts[0].toDouble(&ok);
            if (ok) {
                outClip.duration = dur;
            }
        }
    }

    // Fallback: If duration is still 0, try checking another stream or format duration
    if (outClip.duration == 0.0) {
        QProcess durProbe;
        durProbe.start("ffprobe", QStringList() << "-v" << "error" << "-show_entries" << "format=duration" << "-of" << "default=noprint_wrappers=1:nokey=1" << filePath);
        if (durProbe.waitForFinished(1000)) {
            outClip.duration = durProbe.readAllStandardOutput().trimmed().toDouble();
        }
    }

    // Generate thumbnail
    outClip.thumbnailPath = generateThumbnail(filePath, qMin(2.0, outClip.duration / 2.0));

    return true;
}

QString FFmpegProbe::generateThumbnail(const QString& videoPath, double timeSeconds) {
    QString tempDir = QDir::tempPath();
    QString thumbPath = QString("%1/ffgui_thumb_%2.png")
                        .arg(tempDir)
                        .arg(QUuid::createUuid().toString(QUuid::WithoutBraces));

    QProcess ffmpeg;
    QStringList arguments;
    arguments << "-ss" << QString::number(timeSeconds)
              << "-i" << videoPath
              << "-vframes" << "1"
              << "-filter:v" << "scale=160:90:force_original_aspect_ratio=decrease,pad=160:90:(160-iw)/2:(90-ih)/2:black"
              << "-y"
              << thumbPath;

    ffmpeg.start("ffmpeg", arguments);
    if (ffmpeg.waitForFinished(3000)) {
        if (QFileInfo::exists(thumbPath)) {
            return thumbPath;
        }
    }
    return "";
}
