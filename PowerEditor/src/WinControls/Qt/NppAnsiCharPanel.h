// WinControls/Qt/NppAnsiCharPanel.h — Panel de caracteres ANSI/Unicode Qt6
#pragma once
#include "../Platform/PlatformTypes.h"
#include "NppDockWidget.h"
#ifdef NPP_PLATFORM_LINUX
#include <QWidget>
#include <QVBoxLayout>
#include <QTableWidget>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QFont>

class NppAnsiCharPanel : public QWidget {
    Q_OBJECT
public:
    explicit NppAnsiCharPanel(QWidget* parent = nullptr) : QWidget(parent) {
        auto* layout = new QVBoxLayout(this);
        layout->setContentsMargins(0, 0, 0, 0);
        _table = new QTableWidget(16, 16, this);
        _table->setFont(QFont("Monospace", 10));
        _table->horizontalHeader()->setDefaultSectionSize(28);
        _table->verticalHeader()->setDefaultSectionSize(28);
        _table->setStyleSheet("QTableWidget{background:#1E1E1E;color:#D4D4D4;gridline-color:#333;}QTableWidget::item:selected{background:#094771;}");
        for (int r = 0; r < 16; ++r)
            for (int c = 0; c < 16; ++c) {
                int code = r * 16 + c;
                auto* item = new QTableWidgetItem(code >= 32 ? QString(QChar(code)) : "·");
                item->setToolTip(QString("Dec: %1 | Hex: 0x%2 | Oct: 0%3")
                    .arg(code).arg(code, 2, 16, QChar('0')).arg(code, 3, 8, QChar('0')));
                item->setTextAlignment(Qt::AlignCenter);
                _table->setItem(r, c, item);
            }
        layout->addWidget(_table);
        _lblInfo = new QLabel("Haz clic en un carácter para insertarlo", this);
        _lblInfo->setStyleSheet("color:#969696;padding:4px;");
        layout->addWidget(_lblInfo);
        connect(_table, &QTableWidget::itemClicked, [this](QTableWidgetItem* item) {
            _lblInfo->setText(item->toolTip());
            emit charSelected(item->text());
        });
    }
    NppDockWidget* createDock(QWidget* parent) {
        auto* dock = new NppDockWidget("Panel de Caracteres", parent, NppDockWidget::DockPosition::Right);
        dock->setContent(this);
        return dock;
    }
signals:
    void charSelected(const QString& ch);
private:
    QTableWidget* _table = nullptr;
    QLabel* _lblInfo = nullptr;
};
#endif
