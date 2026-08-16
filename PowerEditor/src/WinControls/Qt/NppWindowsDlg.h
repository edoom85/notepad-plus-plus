// WinControls/Qt/NppWindowsDlg.h — Diálogo de Ventanas/Documentos abiertos Qt6
#pragma once
#include "../Platform/PlatformTypes.h"
#include "NppDialog.h"
#ifdef NPP_PLATFORM_LINUX
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QListWidget>
#include <QPushButton>
#include <QLabel>

class NppWindowsDlg : public NppDialog {
    Q_OBJECT
public:
    explicit NppWindowsDlg(QWidget* parent = nullptr) : NppDialog(parent) {
        setWindowTitle("Ventanas (Documentos abiertos)");
        setMinimumSize(500, 350);
        auto* layout = new QVBoxLayout(this);
        layout->addWidget(new QLabel("Documentos abiertos:", this));
        _list = new QListWidget(this);
        _list->setSelectionMode(QAbstractItemView::ExtendedSelection);
        _list->setStyleSheet("QListWidget{background:#1E1E1E;color:#D4D4D4;border:none;}QListWidget::item{padding:4px;}QListWidget::item:selected{background:#094771;}");
        layout->addWidget(_list);
        auto* btnLayout = new QHBoxLayout();
        auto* btnActivate = new QPushButton("Activar", this);
        auto* btnSave = new QPushButton("Guardar", this);
        auto* btnClose = new QPushButton("Cerrar Doc(s)", this);
        auto* btnOk = new QPushButton("Aceptar", this);
        btnActivate->setStyleSheet("QPushButton{background:#0E639C;color:#FFF;border:none;padding:6px 12px;border-radius:2px;}");
        btnLayout->addWidget(btnActivate);
        btnLayout->addWidget(btnSave);
        btnLayout->addWidget(btnClose);
        btnLayout->addStretch();
        btnLayout->addWidget(btnOk);
        layout->addLayout(btnLayout);
        connect(btnActivate, &QPushButton::clicked, [this](){
            auto items = _list->selectedItems();
            if (!items.isEmpty()) emit activateDocument(items.first()->data(Qt::UserRole).toInt());
            accept();
        });
        connect(btnClose, &QPushButton::clicked, [this](){
            for (auto* item : _list->selectedItems())
                emit closeDocument(item->data(Qt::UserRole).toInt());
        });
        connect(btnOk, &QPushButton::clicked, this, &QDialog::accept);
    }
    void setDocumentList(const QStringList& names) {
        _list->clear();
        for (int i = 0; i < names.size(); ++i) {
            auto* item = new QListWidgetItem(names[i]);
            item->setData(Qt::UserRole, i);
            _list->addItem(item);
        }
    }
signals:
    void activateDocument(int index);
    void closeDocument(int index);
private:
    QListWidget* _list = nullptr;
};
#endif
