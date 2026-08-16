// WinControls/Qt/NppFindCharsInRange.h — Diálogo "Buscar caracteres en rango" (Qt6)
// Reemplaza FindCharsInRange.cpp/.h (findCharsInRange_rc.h) en Linux.
// Permite buscar caracteres fuera del rango ASCII (non-ASCII / UTF-8 / Caracteres extendidos).
//
// Copyright (C) Notepad++ contributors. GPL v3+

#pragma once

#include "NppDialog.h"

#ifdef NPP_PLATFORM_LINUX

#include <QRadioButton>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>

/// Diálogo "Buscar caracteres en rango" para Notepad++ Linux.
class NppFindCharsInRangeDlg : public NppDialog {
    Q_OBJECT

public:
    explicit NppFindCharsInRangeDlg(QWidget* parent = nullptr)
        : NppDialog(parent)
    {
        setWindowTitle("Buscar caracteres en rango");
        setFixedSize(380, 240);

        auto* mainLayout = new QVBoxLayout(this);

        auto* gbRange = new QGroupBox("Rango de caracteres", this);
        auto* layGroup = new QVBoxLayout(gbRange);

        _rbNonAscii = new QRadioButton("Caracteres no ASCII (128 - 255)", this);
        _rbNonAscii->setChecked(true);
        layGroup->addWidget(_rbNonAscii);

        _rbAsciiOnly = new QRadioButton("Solo caracteres ASCII (0 - 127)", this);
        layGroup->addWidget(_rbAsciiOnly);

        _rbCustom = new QRadioButton("Rango personalizado:", this);
        layGroup->addWidget(_rbCustom);

        auto* rowCustom = new QHBoxLayout();
        rowCustom->addWidget(new QLabel("Desde:", this));
        _txtStart = new QLineEdit("0", this);
        _txtStart->setFixedWidth(60);
        rowCustom->addWidget(_txtStart);

        rowCustom->addWidget(new QLabel("Hasta:", this));
        _txtEnd = new QLineEdit("255", this);
        _txtEnd->setFixedWidth(60);
        rowCustom->addWidget(_txtEnd);
        rowCustom->addStretch();
        layGroup->addLayout(rowCustom);

        mainLayout->addWidget(gbRange);

        // Botones de acción
        auto* btnLayout = new QHBoxLayout();
        btnLayout->addStretch();

        auto* btnFind = new QPushButton("Buscar", this);
        auto* btnClose = new QPushButton("Cerrar", this);
        btnFind->setDefault(true);

        btnLayout->addWidget(btnFind);
        btnLayout->addWidget(btnClose);

        mainLayout->addLayout(btnLayout);

        connect(btnFind, &QPushButton::clicked, [this]() {
            int minVal = 128, maxVal = 255;
            if (_rbAsciiOnly->isChecked()) { minVal = 0; maxVal = 127; }
            else if (_rbCustom->isChecked()) {
                minVal = _txtStart->text().toInt();
                maxVal = _txtEnd->text().toInt();
            }
            emit findInRangeRequested(minVal, maxVal);
            accept();
        });
        connect(btnClose, &QPushButton::clicked, this, &QDialog::reject);
    }

signals:
    void findInRangeRequested(int minVal, int maxVal);

private:
    QRadioButton* _rbNonAscii  = nullptr;
    QRadioButton* _rbAsciiOnly = nullptr;
    QRadioButton* _rbCustom    = nullptr;
    QLineEdit*    _txtStart    = nullptr;
    QLineEdit*    _txtEnd      = nullptr;
};


#endif // NPP_PLATFORM_LINUX
