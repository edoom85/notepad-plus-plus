// WinControls/Qt/NppColumnEditor.h — Editor de columna Qt6
#pragma once
#include "../Platform/PlatformTypes.h"
#include "NppDialog.h"
#ifdef NPP_PLATFORM_LINUX
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QRadioButton>
#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>
#include <QComboBox>
#include <QPushButton>

class NppColumnEditor : public NppDialog {
    Q_OBJECT
public:
    explicit NppColumnEditor(QWidget* parent = nullptr) : NppDialog(parent) {
        setWindowTitle("Editor de Columna");
        setFixedSize(380, 280);
        auto* layout = new QVBoxLayout(this);
        auto* grpText = new QGroupBox("Texto a insertar", this);
        auto* textLayout = new QVBoxLayout(grpText);
        _radioText = new QRadioButton("Texto:", grpText);
        _radioText->setChecked(true);
        _textEdit = new QLineEdit(grpText);
        textLayout->addWidget(_radioText);
        textLayout->addWidget(_textEdit);
        layout->addWidget(grpText);
        auto* grpNum = new QGroupBox("Número a insertar", this);
        auto* numLayout = new QVBoxLayout(grpNum);
        _radioNum = new QRadioButton("Número inicial:", grpNum);
        auto* numRow = new QHBoxLayout();
        _spinStart = new QSpinBox(grpNum); _spinStart->setRange(0, 999999); _spinStart->setValue(1);
        _spinInc = new QSpinBox(grpNum); _spinInc->setRange(1, 1000); _spinInc->setValue(1);
        numRow->addWidget(new QLabel("Inicio:", grpNum)); numRow->addWidget(_spinStart);
        numRow->addWidget(new QLabel("Inc:", grpNum)); numRow->addWidget(_spinInc);
        numLayout->addWidget(_radioNum);
        numLayout->addLayout(numRow);
        auto* fmtRow = new QHBoxLayout();
        fmtRow->addWidget(new QLabel("Formato:", grpNum));
        _fmtCombo = new QComboBox(grpNum);
        _fmtCombo->addItems({"Decimal", "Octal", "Hexadecimal", "Binario"});
        fmtRow->addWidget(_fmtCombo);
        numLayout->addLayout(fmtRow);
        layout->addWidget(grpNum);
        auto* btnLayout = new QHBoxLayout();
        auto* btnOk = new QPushButton("Aceptar", this);
        btnOk->setStyleSheet("QPushButton{background:#0E639C;color:#FFF;border:none;padding:6px 16px;border-radius:2px;}");
        auto* btnCancel = new QPushButton("Cancelar", this);
        btnLayout->addStretch(); btnLayout->addWidget(btnOk); btnLayout->addWidget(btnCancel);
        layout->addLayout(btnLayout);
        connect(btnOk, &QPushButton::clicked, [this](){ emit columnEditRequested(_radioText->isChecked(), _textEdit->text(), _spinStart->value(), _spinInc->value(), _fmtCombo->currentIndex()); accept(); });
        connect(btnCancel, &QPushButton::clicked, this, &QDialog::reject);
    }
signals:
    void columnEditRequested(bool isText, const QString& text, int start, int inc, int fmt);
private:
    QRadioButton* _radioText = nullptr; QRadioButton* _radioNum = nullptr;
    QLineEdit* _textEdit = nullptr;
    QSpinBox* _spinStart = nullptr; QSpinBox* _spinInc = nullptr;
    QComboBox* _fmtCombo = nullptr;
};
#endif
