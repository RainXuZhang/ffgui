#ifndef PROJECTBINWIDGET_H
#define PROJECTBINWIDGET_H

#include <QWidget>
#include <QListWidget>
#include <QPushButton>
#include <QToolButton>
#include <QLabel>
#include "../core/ProjectModel.h"

class ProjectBinWidget : public QWidget {
    Q_OBJECT

public:
    explicit ProjectBinWidget(QWidget* parent = nullptr);
    ~ProjectBinWidget() override = default;

    void addClip(const MediaClip& clip);
    void clear();

signals:
    void clipAdded(const MediaClip& clip);
    void clipSelected(const MediaClip& clip);
    void clipDoubleClicked(const MediaClip& clip);
    void clipRemoved(const QString& id);

private slots:
    void onAddClipClicked();
    void onRemoveClipClicked();
    void onItemDoubleClicked(QListWidgetItem* item);
    void onItemSelectionChanged();

private:
    QListWidget* m_listWidget;
    QPushButton* m_addButton;
    QPushButton* m_removeButton;
    QMap<QString, MediaClip> m_clips; // Maps ID to MediaClip
};

#endif // PROJECTBINWIDGET_H
