#ifndef EFFECTSTACKWIDGET_H
#define EFFECTSTACKWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QComboBox>
#include <QVBoxLayout>
#include <QPushButton>
#include <QScrollArea>
#include <QGroupBox>
#include <QLineEdit>
#include <QSpinBox>
#include <QCheckBox>
#include "../core/ProjectModel.h"

class EffectStackWidget : public QWidget {
    Q_OBJECT

public:
    explicit EffectStackWidget(QWidget* parent = nullptr);
    ~EffectStackWidget() override = default;

    void setProject(Project* project);
    void setSelectedClip(const QString& clipId);

signals:
    void effectChanged();

private slots:
    void onAddEffectSelected(int index);
    void onEffectToggled(const QString& type, bool checked);
    void onParamChanged();

private:
    void refreshStack();
    QWidget* createTextEffectControls(ClipEffect& effect);
    QWidget* createCropEffectControls(ClipEffect& effect);

    Project* m_project = nullptr;
    QString m_selectedClipId;

    QLabel* m_titleLabel;
    QComboBox* m_addEffectCombo;
    QScrollArea* m_scrollArea;
    QWidget* m_scrollContent;
    QVBoxLayout* m_scrollLayout;
};

#endif // EFFECTSTACKWIDGET_H
