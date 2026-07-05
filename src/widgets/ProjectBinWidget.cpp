#include "ProjectBinWidget.h"
#include "../core/FFmpegProbe.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QDrag>
#include <QMimeData>
#include <QIcon>
#include <QPixmap>
#include <QDropEvent>

ProjectBinWidget::ProjectBinWidget(QWidget* parent)
    : QWidget(parent)
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(4, 4, 4, 4);
    mainLayout->setSpacing(4);

    // Toolbar for adding/removing clips (looks like Kdenlive)
    auto* toolbarLayout = new QHBoxLayout();
    toolbarLayout->setSpacing(2);

    m_addButton = new QPushButton(QIcon::fromTheme("list-add"), "Add Clip", this);
    m_addButton->setStyleSheet("QPushButton { padding: 4px 8px; font-weight: bold; }");
    connect(m_addButton, &QPushButton::clicked, this, &ProjectBinWidget::onAddClipClicked);

    m_removeButton = new QPushButton(QIcon::fromTheme("list-remove"), "Delete", this);
    m_removeButton->setEnabled(false);
    m_removeButton->setStyleSheet("QPushButton { padding: 4px 8px; }");
    connect(m_removeButton, &QPushButton::clicked, this, &ProjectBinWidget::onRemoveClipClicked);

    m_addToTimelineButton = new QPushButton(QIcon::fromTheme("list-add"), "Add to Timeline", this);
    m_addToTimelineButton->setEnabled(false);
    m_addToTimelineButton->setStyleSheet("QPushButton { padding: 4px 8px; }");
    connect(m_addToTimelineButton, &QPushButton::clicked, this, [this]() {
        auto* item = m_listWidget->currentItem();
        if (item) {
            QString id = item->data(Qt::UserRole).toString();
            if (m_clips.contains(id)) {
                emit addToTimelineRequested(m_clips[id]);
            }
        }
    });

    toolbarLayout->addWidget(m_addButton);
    toolbarLayout->addWidget(m_removeButton);
    toolbarLayout->addWidget(m_addToTimelineButton);
    toolbarLayout->addStretch();
    mainLayout->addLayout(toolbarLayout);

    // List widget
    m_listWidget = new QListWidget(this);
    m_listWidget->setIconSize(QSize(120, 68));
    m_listWidget->setDragEnabled(true);
    m_listWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    m_listWidget->setAcceptDrops(true);
    m_listWidget->setStyleSheet(
        "QListWidget {"
        "    background-color: #1e1e1e;"
        "    border: 1px solid #333333;"
        "    color: #d2d2d2;"
    "}"
    "QListWidget::item {"
    "    border-bottom: 1px solid #2a2a2a;"
    "    padding: 6px;"
    "}"
    "QListWidget::item:selected {"
    "    background-color: #2a82da;"
    "    color: white;"
    "}"
    );

    connect(m_listWidget, &QListWidget::itemDoubleClicked, this, &ProjectBinWidget::onItemDoubleClicked);
    connect(m_listWidget, &QListWidget::itemSelectionChanged, this, &ProjectBinWidget::onItemSelectionChanged);

    mainLayout->addWidget(m_listWidget);
}

void ProjectBinWidget::addClip(const MediaClip& clip) {
    if (m_clips.contains(clip.id)) return;

    m_clips[clip.id] = clip;

    auto* item = new QListWidgetItem(m_listWidget);
    item->setData(Qt::UserRole, clip.id);

    // Text formatting
    QString text = QString("<b>%1</b><br/>"
                           "Duration: %2s<br/>"
                           "Res: %3x%4 | FPS: %5")
                       .arg(clip.fileName)
                       .arg(QString::number(clip.duration, 'f', 1))
                       .arg(clip.width)
                       .arg(clip.height)
                       .arg(QString::number(clip.fps, 'f', 2));

    // Set text layout
    auto* label = new QLabel(text);
    label->setStyleSheet("color: #d2d2d2;");

    // Icon thumbnail
    if (!clip.thumbnailPath.isEmpty() && QFile::exists(clip.thumbnailPath)) {
        item->setIcon(QIcon(clip.thumbnailPath));
    } else {
        // Fallback icon
        item->setIcon(QIcon::fromTheme("video-x-generic"));
    }

    item->setText(clip.fileName + QString(" [%1s]").arg(QString::number(clip.duration, 'f', 1)));
    item->setToolTip(clip.filePath);

    m_listWidget->addItem(item);

    emit clipAdded(clip);
}

void ProjectBinWidget::clear() {
    m_listWidget->clear();
    m_clips.clear();
    m_removeButton->setEnabled(false);
    m_addToTimelineButton->setEnabled(false);
}

void ProjectBinWidget::onAddClipClicked() {
    QStringList files = QFileDialog::getOpenFileNames(
        this,
        "Select Media Files",
        "",
        "Videos & Audios (*.mp4 *.mkv *.avi *.mov *.mp3 *.wav *.ogg *.m4a);;All Files (*)"
    );

    for (const QString& file : files) {
        MediaClip clip;
        if (FFmpegProbe::probeFile(file, clip)) {
            addClip(clip);
        } else {
            QMessageBox::warning(this, "Error", QString("Failed to load or parse media file:\n%1").arg(file));
        }
    }
}

void ProjectBinWidget::onRemoveClipClicked() {
    auto* item = m_listWidget->currentItem();
    if (!item) return;

    QString id = item->data(Qt::UserRole).toString();
    m_clips.remove(id);
    delete item;

    emit clipRemoved(id);
}

void ProjectBinWidget::onItemDoubleClicked(QListWidgetItem* item) {
    QString id = item->data(Qt::UserRole).toString();
    if (m_clips.contains(id)) {
        emit clipDoubleClicked(m_clips[id]);
    }
}

void ProjectBinWidget::onItemSelectionChanged() {
    auto* item = m_listWidget->currentItem();
    if (item) {
        m_removeButton->setEnabled(true);
        m_addToTimelineButton->setEnabled(true);
        QString id = item->data(Qt::UserRole).toString();
        if (m_clips.contains(id)) {
            emit clipSelected(m_clips[id]);
        }
    } else {
        m_removeButton->setEnabled(false);
        m_addToTimelineButton->setEnabled(false);
    }
}

void ProjectBinWidget::dropEvent(QDropEvent* event) {
    if (event->mimeData()->hasUrls()) {
        for (const QUrl& url : event->mimeData()->urls()) {
            QString localPath = url.toLocalFile();
            if (!localPath.isEmpty()) {
                MediaClip clip;
                if (FFmpegProbe::probeFile(localPath, clip)) {
                    addClip(clip);
                } else {
                    QMessageBox::warning(this, "Error", QString("Failed to load or parse media file:\n%1").arg(localPath));
                }
            }
        }
        event->acceptProposedAction();
    } else {
        QWidget::dropEvent(event);
    }
}

// Add dragEnterEvent right below it to fully support the drag-and-drop workflow
void ProjectBinWidget::dragEnterEvent(QDragEnterEvent* event) {
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    } else {
        QWidget::dragEnterEvent(event);
    }
}
void ProjectBinWidget::addToSequence(const MediaClip& clip) {
    SequenceItem item;
    item.assetPath = clip.filePath;
    item.inPoint = 0.0;
    item.outPoint = clip.duration;
    item.overlayText = "";
    item.sequenceOrder = m_projectSequence.size();

    m_projectSequence.append(item);
    emit sequenceUpdated();
}
