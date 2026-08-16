// WinControls/Qt/NppRunDlg.h — Diálogo Ejecutar comando Qt6
#pragma once
#include "../Platform/PlatformTypes.h"
#include "NppDialog.h"
#ifdef NPP_PLATFORM_LINUX
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <QPushButton>
#include <QProcess>

class NppRunDlg : public NppDialog {
    Q_OBJECT
public:
    explicit NppRunDlg(QWidget* parent = nullptr) : NppDialog(parent) {
        setWindowTitle("Ejecutar...");
        setFixedSize(500, 150);
        auto* layout = new QVBoxLayout(this);
        layout->addWidget(new QLabel("Programa a ejecutar:", this));
        _cmdCombo = new QComboBox(this);
        _cmdCombo->setEditable(true);
        _cmdCombo->setInsertPolicy(QComboBox::InsertAtTop);
        _cmdCombo->addItems({"xdg-open $(FULL_CURRENT_PATH)", "xterm -e bash", "firefox $(FULL_CURRENT_PATH)"});
        layout->addWidget(_cmdCombo);
        auto* btnLayout = new QHBoxLayout();
        auto* btnRun = new QPushButton("Ejecutar", this);
        btnRun->setStyleSheet("QPushButton{background:#0E639C;color:#FFF;border:none;padding:6px 16px;border-radius:2px;}");
        auto* btnCancel = new QPushButton("Cancelar", this);
        btnLayout->addStretch();
        btnLayout->addWidget(btnRun);
        btnLayout->addWidget(btnCancel);
        layout->addLayout(btnLayout);
        connect(btnRun, &QPushButton::clicked, this, &NppRunDlg::onRun);
        connect(btnCancel, &QPushButton::clicked, this, &QDialog::reject);
    }
signals:
    void commandExecuted(const QString& cmd);
private slots:
    void onRun() {
        QString cmd = _cmdCombo->currentText().trimmed();
        if (cmd.isEmpty()) return;
        QProcess::startDetached("/bin/bash", {"-c", cmd});
        emit commandExecuted(cmd);
        accept();
    }
private:
    QComboBox* _cmdCombo = nullptr;
};
#endif
