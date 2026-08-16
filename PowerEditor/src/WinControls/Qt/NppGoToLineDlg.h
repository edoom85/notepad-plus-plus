// WinControls/Qt/NppGoToLineDlg.h — Ir a línea Qt6
#pragma once
#include "../Platform/PlatformTypes.h"
#include "NppDialog.h"
#ifdef NPP_PLATFORM_LINUX
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QRadioButton>
#include <QPushButton>
#include <QIntValidator>

class NppGoToLineDlg : public NppDialog {
    Q_OBJECT
public:
    explicit NppGoToLineDlg(QWidget* parent = nullptr) : NppDialog(parent) {
        setWindowTitle("Ir a...");
        setFixedSize(300, 160);
        auto* layout = new QVBoxLayout(this);
        auto* modeLayout = new QHBoxLayout();
        _radioLine = new QRadioButton("Línea", this);
        _radioOffset = new QRadioButton("Offset", this);
        _radioLine->setChecked(true);
        modeLayout->addWidget(_radioLine);
        modeLayout->addWidget(_radioOffset);
        layout->addLayout(modeLayout);
        auto* inputLayout = new QHBoxLayout();
        _lblPrompt = new QLabel("Línea:", this);
        _lineEdit = new QLineEdit(this);
        _lineEdit->setValidator(new QIntValidator(1, 999999999, this));
        _lineEdit->setPlaceholderText("Número de línea...");
        inputLayout->addWidget(_lblPrompt);
        inputLayout->addWidget(_lineEdit);
        layout->addLayout(inputLayout);
        _lblInfo = new QLabel("", this);
        _lblInfo->setStyleSheet("color: #969696; font-size: 9pt;");
        layout->addWidget(_lblInfo);
        auto* btnLayout = new QHBoxLayout();
        auto* btnGo = new QPushButton("Ir", this);
        btnGo->setStyleSheet("QPushButton{background:#0E639C;color:#FFF;border:none;padding:6px 16px;border-radius:2px;}QPushButton:hover{background:#1177BB;}");
        auto* btnClose = new QPushButton("Cerrar", this);
        btnLayout->addStretch();
        btnLayout->addWidget(btnGo);
        btnLayout->addWidget(btnClose);
        layout->addLayout(btnLayout);
        connect(btnGo, &QPushButton::clicked, this, &NppGoToLineDlg::onGo);
        connect(btnClose, &QPushButton::clicked, this, &QDialog::accept);
        connect(_radioLine, &QRadioButton::toggled, [this](bool c){ _lblPrompt->setText(c?"Línea:":"Offset:"); });
    }
    void setInfo(int currentLine, int totalLines) {
        _lblInfo->setText(QString("Actual: %1 / Total: %2").arg(currentLine).arg(totalLines));
    }
signals:
    void goToLine(int line);
    void goToOffset(int offset);
private slots:
    void onGo() {
        int val = _lineEdit->text().toInt();
        if (val <= 0) return;
        if (_radioLine->isChecked()) emit goToLine(val);
        else emit goToOffset(val);
    }
private:
    QRadioButton* _radioLine = nullptr;
    QRadioButton* _radioOffset = nullptr;
    QLineEdit* _lineEdit = nullptr;
    QLabel* _lblPrompt = nullptr;
    QLabel* _lblInfo = nullptr;
};
#endif
