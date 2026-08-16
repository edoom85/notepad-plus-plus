// WinControls/Qt/NppMD5Dlg.h — Generador de hash MD5/SHA256 Qt6
#pragma once
#include "../Platform/PlatformTypes.h"
#include "NppDialog.h"
#ifdef NPP_PLATFORM_LINUX
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QTextEdit>
#include <QPushButton>
#include <QComboBox>
#include <QCryptographicHash>
#include <QClipboard>
#include <QApplication>

class NppMD5Dlg : public NppDialog {
    Q_OBJECT
public:
    explicit NppMD5Dlg(QWidget* parent = nullptr) : NppDialog(parent) {
        setWindowTitle("Generador de Hash");
        setMinimumSize(480, 280);
        auto* layout = new QVBoxLayout(this);
        layout->addWidget(new QLabel("Texto de entrada:", this));
        _input = new QTextEdit(this);
        _input->setPlaceholderText("Pega o escribe el texto aquí...");
        _input->setMaximumHeight(100);
        layout->addWidget(_input);
        auto* algoLayout = new QHBoxLayout();
        algoLayout->addWidget(new QLabel("Algoritmo:", this));
        _algoCombo = new QComboBox(this);
        _algoCombo->addItems({"MD5", "SHA-1", "SHA-256", "SHA-512"});
        _algoCombo->setCurrentIndex(2);
        algoLayout->addWidget(_algoCombo);
        auto* btnGen = new QPushButton("Generar", this);
        btnGen->setStyleSheet("QPushButton{background:#0E639C;color:#FFF;border:none;padding:6px 16px;border-radius:2px;}");
        algoLayout->addWidget(btnGen);
        algoLayout->addStretch();
        layout->addLayout(algoLayout);
        layout->addWidget(new QLabel("Resultado:", this));
        auto* resultLayout = new QHBoxLayout();
        _result = new QLineEdit(this);
        _result->setReadOnly(true);
        _result->setFont(QFont("Monospace", 10));
        auto* btnCopy = new QPushButton("📋", this);
        btnCopy->setFixedWidth(32);
        resultLayout->addWidget(_result);
        resultLayout->addWidget(btnCopy);
        layout->addLayout(resultLayout);
        connect(btnGen, &QPushButton::clicked, this, &NppMD5Dlg::onGenerate);
        connect(btnCopy, &QPushButton::clicked, [this](){ QApplication::clipboard()->setText(_result->text()); });
    }
private slots:
    void onGenerate() {
        QByteArray data = _input->toPlainText().toUtf8();
        QCryptographicHash::Algorithm algo;
        switch (_algoCombo->currentIndex()) {
            case 0: algo = QCryptographicHash::Md5; break;
            case 1: algo = QCryptographicHash::Sha1; break;
            case 2: algo = QCryptographicHash::Sha256; break;
            case 3: algo = QCryptographicHash::Sha512; break;
            default: algo = QCryptographicHash::Sha256;
        }
        _result->setText(QCryptographicHash::hash(data, algo).toHex());
    }
private:
    QTextEdit* _input = nullptr;
    QComboBox* _algoCombo = nullptr;
    QLineEdit* _result = nullptr;
};
#endif
