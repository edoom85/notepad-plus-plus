// WinControls/Qt/NppRunMacroDlg.h — Diálogo "Ejecutar macro múltiples veces" (Qt6)
// Reemplaza RunMacroDlg.cpp/.h (RunMacroDlg_rc.h) en Linux.
// Permite ejecutar una macro grabada N veces o hasta el final del archivo.
//
// Copyright (C) Notepad++ contributors. GPL v3+

#pragma once

#include "NppDialog.h"

#ifdef NPP_PLATFORM_LINUX

#include <QRadioButton>
#include <QSpinBox>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>

/// Diálogo "Ejecutar macro múltiples veces" para Notepad++ Linux.
class NppRunMacroDlg : public NppDialog {
    Q_OBJECT

public:
    explicit NppRunMacroDlg(QWidget* parent = nullptr)
        : NppDialog(parent)
    {
        setWindowTitle("Ejecutar macro múltiples veces");
        setFixedSize(360, 220);

        auto* mainLayout = new QVBoxLayout(this);

        auto* gbOptions = new QGroupBox("Opciones de ejecución", this);
        auto* layGroup = new QVBoxLayout(gbOptions);

        _rbTimes = new QRadioButton("Ejecutar", this);
        _rbTimes->setChecked(true);
        
        auto* rowSpin = new QHBoxLayout();
        rowSpin->addWidget(_rbTimes);
        _spinTimes = new QSpinBox(this);
        _spinTimes->setRange(1, 100000);
        _spinTimes->setValue(1);
        rowSpin->addWidget(_spinTimes);
        rowSpin->addWidget(new QLabel("veces", this));
        layGroup->addLayout(rowSpin);

        _rbEof = new QRadioButton("Ejecutar hasta el final del archivo", this);
        layGroup->addWidget(_rbEof);

        mainLayout->addWidget(gbOptions);

        // Botones de acción
        auto* btnLayout = new QHBoxLayout();
        btnLayout->addStretch();

        auto* btnRun = new QPushButton("Ejecutar", this);
        auto* btnCancel = new QPushButton("Cancelar", this);
        btnRun->setDefault(true);

        btnLayout->addWidget(btnRun);
        btnLayout->addWidget(btnCancel);

        mainLayout->addLayout(btnLayout);

        connect(btnRun, &QPushButton::clicked, this, &QDialog::accept);
        connect(btnCancel, &QPushButton::clicked, this, &QDialog::reject);
    }

private:
    QRadioButton* _rbTimes = nullptr;
    QRadioButton* _rbEof   = nullptr;
    QSpinBox*     _spinTimes = nullptr;
};

#endif // NPP_PLATFORM_LINUX
