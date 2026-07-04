#include "EffectStackWidget.h"
#include <QFormLayout>
#include <QHBoxLayout>
#include <QColorDialog>
#include <QDebug>

EffectStackWidget::EffectStackWidget(QWidget* parent)
    : QWidget(parent)
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(4, 4, 4, 4);
    mainLayout->setSpacing(6);

    // Dock Header Title
    m_titleLabel = new QLabel("Effect Stack", this);
    m_titleLabel->setStyleSheet(
        "QLabel {"
        "    font-weight: bold;"
        "    color: #a0a0a0;"
        "    background-color: #252525;"
        "    padding: 4px;"
        "    border-bottom: 1px solid #1a1a1a;"
        "}"
    );
    mainLayout->addWidget(m_titleLabel);

    // Dropdown to add effects (Kdenlive style)
    auto* addEffectLayout = new QHBoxLayout();
    addEffectLayout->setSpacing(4);
    
    auto* addLabel = new QLabel("Add Effect:", this);
    addLabel->setStyleSheet("color: #d2d2d2;");
    addEffectLayout->addWidget(addLabel);

    m_addEffectCombo = new QComboBox(this);
    m_addEffectCombo->addItem("-- Select Effect --");
    m_addEffectCombo->addItem("Text Overlay");
    m_addEffectCombo->addItem("Crop Video");
    m_addEffectCombo->setStyleSheet(
        "QComboBox {"
        "    background-color: #333333;"
        "    color: #d2d2d2;"
        "    border: 1px solid #555555;"
        "    padding: 3px 6px;"
        "    border-radius: 3px;"
        "}"
    );
    connect(m_addEffectCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &EffectStackWidget::onAddEffectSelected);
    addEffectLayout->addWidget(m_addEffectCombo, 1);
    
    mainLayout->addLayout(addEffectLayout);

    // Scroll Area for selected effects
    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setStyleSheet("background-color: #1e1e1e; border: 1px solid #333333;");

    m_scrollContent = new QWidget(m_scrollArea);
    m_scrollLayout = new QVBoxLayout(m_scrollContent);
    m_scrollLayout->setContentsMargins(4, 4, 4, 4);
    m_scrollLayout->setSpacing(8);
    m_scrollLayout->addStretch();
    m_scrollContent->setLayout(m_scrollLayout);
    
    m_scrollArea->setWidget(m_scrollContent);
    mainLayout->addWidget(m_scrollArea, 1);

    setSelectedClip(""); // Show default select-clip state
}

void EffectStackWidget::setProject(Project* project) {
    m_project = project;
    setSelectedClip("");
}

void EffectStackWidget::setSelectedClip(const QString& clipId) {
    m_selectedClipId = clipId;
    m_addEffectCombo->setEnabled(!clipId.isEmpty());
    refreshStack();
}

void EffectStackWidget::refreshStack() {
    // Clear old items inside scroll layout (except the stretch spacer)
    QLayoutItem* item;
    while (m_scrollLayout->count() > 1) {
        item = m_scrollLayout->takeAt(0);
        if (item->widget()) {
            delete item->widget();
        }
        delete item;
    }

    if (!m_project || m_selectedClipId.isEmpty()) {
        auto* selectLabel = new QLabel("Select a clip on the timeline\nto add/edit effects.", m_scrollContent);
        selectLabel->setAlignment(Qt::AlignCenter);
        selectLabel->setStyleSheet("color: #808080; font-style: italic; padding: 20px;");
        m_scrollLayout->insertWidget(0, selectLabel);
        return;
    }

    TimelineClip* clip = m_project->findTimelineClip(m_selectedClipId);
    if (!clip) return;

    // Display clip title
    MediaClip* mc = m_project->findMediaClip(clip->mediaClipId);
    m_titleLabel->setText(QString("Effect Stack: %1").arg(mc ? mc->fileName : "Clip"));

    for (int i = 0; i < clip->effects.size(); ++i) {
        ClipEffect& fx = clip->effects[i];
        
        auto* group = new QGroupBox(fx.type.toUpper(), m_scrollContent);
        group->setCheckable(true);
        group->setChecked(fx.enabled);
        group->setStyleSheet(
            "QGroupBox {"
            "    background-color: #2d2d2d;"
            "    border: 1px solid #444444;"
            "    border-radius: 4px;"
            "    margin-top: 20px;"
            "    padding: 8px;"
            "    font-weight: bold;"
            "    color: #e0e0e0;"
            "}"
            "QGroupBox::title {"
            "    subcontrol-origin: margin;"
            "    subcontrol-position: top left;"
            "    left: 8px;"
            "    padding: 0 3px;"
            "}"
        );

        connect(group, &QGroupBox::toggled, this, [this, type = fx.type](bool checked) {
            onEffectToggled(type, checked);
        });

        auto* groupLayout = new QVBoxLayout(group);
        groupLayout->setContentsMargins(4, 12, 4, 4);

        // Control panel depending on effect type
        if (fx.type == "text") {
            groupLayout->addWidget(createTextEffectControls(fx));
        } else if (fx.type == "crop") {
            groupLayout->addWidget(createCropEffectControls(fx));
        }

        // Delete button for this effect
        auto* deleteBtn = new QPushButton("Remove Effect", group);
        deleteBtn->setStyleSheet("background-color: #aa3333; color: white; padding: 3px; border-radius: 2px;");
        connect(deleteBtn, &QPushButton::clicked, this, [this, idx = i, clip]() {
            clip->effects.removeAt(idx);
            refreshStack();
            emit effectChanged();
        });
        groupLayout->addWidget(deleteBtn);

        m_scrollLayout->insertWidget(m_scrollLayout->count() - 1, group);
    }
}

QWidget* EffectStackWidget::createTextEffectControls(ClipEffect& effect) {
    auto* widget = new QWidget(this);
    auto* form = new QFormLayout(widget);
    form->setContentsMargins(2, 2, 2, 2);
    form->setSpacing(4);

    // Text String
    auto* textEdit = new QLineEdit(effect.params["text"].toString(), widget);
    connect(textEdit, &QLineEdit::textChanged, this, [this, &effect](const QString& text) {
        effect.params["text"] = text;
        onParamChanged();
    });
    form->addRow("Text:", textEdit);

    // Font Size
    auto* sizeSpin = new QSpinBox(widget);
    sizeSpin->setRange(10, 200);
    sizeSpin->setValue(effect.params["size"].toInt());
    connect(sizeSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, [this, &effect](int value) {
        effect.params["size"] = value;
        onParamChanged();
    });
    form->addRow("Font Size (px):", sizeSpin);

    // X Coordinate
    auto* xSpin = new QSpinBox(widget);
    xSpin->setRange(0, 3840);
    xSpin->setValue(effect.params["x"].toInt());
    connect(xSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, [this, &effect](int value) {
        effect.params["x"] = value;
        onParamChanged();
    });
    form->addRow("Position X:", xSpin);

    // Y Coordinate
    auto* ySpin = new QSpinBox(widget);
    ySpin->setRange(0, 2160);
    ySpin->setValue(effect.params["y"].toInt());
    connect(ySpin, QOverload<int>::of(&QSpinBox::valueChanged), this, [this, &effect](int value) {
        effect.params["y"] = value;
        onParamChanged();
    });
    form->addRow("Position Y:", ySpin);

    // Font color
    auto* colorBtn = new QPushButton("Select Color", widget);
    colorBtn->setStyleSheet(QString("background-color: %1; color: gray;").arg(effect.params["color"].toString()));
    connect(colorBtn, &QPushButton::clicked, this, [this, &effect, colorBtn]() {
        QColor col = QColorDialog::getColor(QColor(effect.params["color"].toString()), this, "Select Font Color");
        if (col.isValid()) {
            effect.params["color"] = col.name();
            colorBtn->setStyleSheet(QString("background-color: %1; color: gray;").arg(col.name()));
            onParamChanged();
        }
    });
    form->addRow("Text Color:", colorBtn);

    return widget;
}

QWidget* EffectStackWidget::createCropEffectControls(ClipEffect& effect) {
    auto* widget = new QWidget(this);
    auto* form = new QFormLayout(widget);
    form->setContentsMargins(2, 2, 2, 2);
    form->setSpacing(4);

    // Left Crop %
    auto* leftSpin = new QSpinBox(widget);
    leftSpin->setRange(0, 100);
    leftSpin->setValue(effect.params["left"].toInt());
    connect(leftSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, [this, &effect](int value) {
        effect.params["left"] = value;
        onParamChanged();
    });
    form->addRow("Crop Left (%):", leftSpin);

    // Top Crop %
    auto* topSpin = new QSpinBox(widget);
    topSpin->setRange(0, 100);
    topSpin->setValue(effect.params["top"].toInt());
    connect(topSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, [this, &effect](int value) {
        effect.params["top"] = value;
        onParamChanged();
    });
    form->addRow("Crop Top (%):", topSpin);

    // Right Crop %
    auto* rightSpin = new QSpinBox(widget);
    rightSpin->setRange(0, 100);
    rightSpin->setValue(effect.params["right"].toInt());
    connect(rightSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, [this, &effect](int value) {
        effect.params["right"] = value;
        onParamChanged();
    });
    form->addRow("Crop Right (%):", rightSpin);

    // Bottom Crop %
    auto* bottomSpin = new QSpinBox(widget);
    bottomSpin->setRange(0, 100);
    bottomSpin->setValue(effect.params["bottom"].toInt());
    connect(bottomSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, [this, &effect](int value) {
        effect.params["bottom"] = value;
        onParamChanged();
    });
    form->addRow("Crop Bottom (%):", bottomSpin);

    return widget;
}

void EffectStackWidget::onAddEffectSelected(int index) {
    if (index <= 0 || m_selectedClipId.isEmpty() || !m_project) return;

    TimelineClip* clip = m_project->findTimelineClip(m_selectedClipId);
    if (!clip) return;

    QString effectName = m_addEffectCombo->itemText(index);
    ClipEffect newFx;

    if (effectName == "Text Overlay") {
        newFx.type = "text";
        newFx.params["text"] = "Add Custom Text Here";
        newFx.params["size"] = 32;
        newFx.params["color"] = "#ffffff";
        newFx.params["x"] = 100;
        newFx.params["y"] = 100;
    } else if (effectName == "Crop Video") {
        newFx.type = "crop";
        newFx.params["left"] = 10;
        newFx.params["top"] = 10;
        newFx.params["right"] = 10;
        newFx.params["bottom"] = 10;
    }

    clip->effects.append(newFx);
    m_addEffectCombo->setCurrentIndex(0); // Reset dropdown selection
    refreshStack();
    emit effectChanged();
}

void EffectStackWidget::onEffectToggled(const QString& type, bool checked) {
    if (m_selectedClipId.isEmpty() || !m_project) return;
    TimelineClip* clip = m_project->findTimelineClip(m_selectedClipId);
    if (!clip) return;

    for (auto& fx : clip->effects) {
        if (fx.type == type) {
            fx.enabled = checked;
            break;
        }
    }
    emit effectChanged();
}

void EffectStackWidget::onParamChanged() {
    emit effectChanged();
}
