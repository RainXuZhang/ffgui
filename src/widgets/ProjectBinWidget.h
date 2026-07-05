#ifndef PROJECTBINWIDGET_H
#define PROJECTBINWIDGET_H

#include <QWidget>
#include <QListWidget>
#include <QPushButton>
#include <QToolButton>
#include <QLabel>
#include <QMap>
#include "../core/ProjectModel.h"

#include <QDrag>
#include <QMimeData>
#include <QUuid>

struct MediaAsset {
    QString filePath;
    QString fileName;
    double duration;
};

class ProjectBinWidget : public QWidget {
    Q_OBJECT

public:
    explicit ProjectBinWidget(QWidget* parent = nullptr);
    ~ProjectBinWidget() override = default;

    void addClip(const MediaClip& clip);
    void clear();
    void addToSequence(const MediaClip& clip);

signals:
    void clipAdded(const MediaClip& clip);
    void clipSelected(const MediaClip& clip);
    void clipDoubleClicked(const MediaClip& clip);
    void clipRemoved(const QString& id);
    void addToTimelineRequested(const MediaClip& clip);
    void sequenceUpdated();

private slots:
    void onAddClipClicked();
    void onRemoveClipClicked();
    void onItemDoubleClicked(QListWidgetItem* item);
    void onItemSelectionChanged();
    void onItemDropped(QDropEvent* event);

private:
    QListWidget* m_listWidget;
    QPushButton* m_addButton;
    QPushButton* m_removeButton;
    QPushButton* m_addToTimelineButton;
    QMap<QString, MediaClip> m_clips; // Maps ID to MediaClip
    QMap<QString, MediaAsset> m_binAssets;
    QList<SequenceItem> m_projectSequence;

protected:
    void dropEvent(QDropEvent* event) override;
};

#endif // PROJECTBINWIDGET_H
