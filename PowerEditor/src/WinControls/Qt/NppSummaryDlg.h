// WinControls/Qt/NppSummaryDlg.h — Resumen de documento Qt6
#pragma once
#include "../Platform/PlatformTypes.h"
#include "NppDialog.h"
#ifdef NPP_PLATFORM_LINUX
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QFileInfo>
#include <QDateTime>

class NppSummaryDlg : public NppDialog {
    Q_OBJECT
public:
    explicit NppSummaryDlg(QWidget* parent = nullptr) : NppDialog(parent) {
        setWindowTitle("Resumen del documento");
        setFixedSize(400, 280);
        _layout = new QVBoxLayout(this);
        _infoLabel = new QLabel(this);
        _infoLabel->setTextFormat(Qt::RichText);
        _infoLabel->setWordWrap(true);
        _layout->addWidget(_infoLabel);
        _layout->addStretch();
        auto* btnClose = new QPushButton("Cerrar", this);
        _layout->addWidget(btnClose);
        connect(btnClose, &QPushButton::clicked, this, &QDialog::accept);
    }
    void setSummary(const QString& filePath, const QString& content) {
        QFileInfo fi(filePath);
        int lines = content.count('\n') + 1;
        int words = content.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts).size();
        int chars = content.length();
        int bytes = content.toUtf8().size();
        QString html = QString(
            "<table style='color:#D4D4D4;font-size:10pt;'>"
            "<tr><td><b>Archivo:</b></td><td>%1</td></tr>"
            "<tr><td><b>Ruta:</b></td><td>%2</td></tr>"
            "<tr><td><b>Tamaño:</b></td><td>%3 bytes</td></tr>"
            "<tr><td><b>Líneas:</b></td><td>%4</td></tr>"
            "<tr><td><b>Palabras:</b></td><td>%5</td></tr>"
            "<tr><td><b>Caracteres:</b></td><td>%6</td></tr>"
            "<tr><td><b>Modificado:</b></td><td>%7</td></tr>"
            "</table>")
            .arg(fi.fileName()).arg(fi.absolutePath())
            .arg(bytes).arg(lines).arg(words).arg(chars)
            .arg(fi.lastModified().toString("yyyy-MM-dd hh:mm:ss"));
        _infoLabel->setText(html);
    }
private:
    QVBoxLayout* _layout = nullptr;
    QLabel* _infoLabel = nullptr;
};
#endif
