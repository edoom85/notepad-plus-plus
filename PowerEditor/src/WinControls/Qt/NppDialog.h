// WinControls/Qt/NppDialog.h — Diálogo base portable Qt6
// Reemplaza WinControls/StaticDialog/StaticDialog.cpp/.h en Linux
//
// En Windows: StaticDialog hereda de Window y usa DialogBoxParam, 
//             WM_INITDIALOG, EndDialog, etc.
// En Linux:   QDialog con soporte para:
//   - Creación modal y modeless
//   - DPI awareness (nativo en Qt6)
//   - Dark mode automático
//
// Copyright (C) Notepad++ contributors. GPL v3+

#pragma once

#include "../Platform/PlatformTypes.h"
#include "../Platform/PlatformString.h"
#include "../../Platform/PlatformTheme.h"


#ifdef NPP_PLATFORM_LINUX

#include <QDialog>
#include <QWidget>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QKeyEvent>
#include <QString>

/// Diálogo base portable Qt6 para Notepad++ Linux.
/// Equivalente a StaticDialog de Win32. Todos los diálogos específicos
/// (FindReplaceDlg, PreferenceDlg, AboutDlg, etc.) heredan de esta clase.
class NppDialog : public QDialog {
    Q_OBJECT

public:
    explicit NppDialog(QWidget* parent = nullptr, Qt::WindowFlags flags = Qt::WindowFlags())
        : QDialog(parent, flags)
    {
        // Estilo adaptativo (Modo Oscuro / Modo Claro)
        if (NppTheme::isDarkMode()) {
            setStyleSheet(
                "QDialog { background: #252526; color: #D4D4D4; }"
                "QLabel, QCheckBox, QRadioButton { color: #D4D4D4; }"
                "QLineEdit, QTextEdit, QPlainTextEdit, QComboBox { background: #3C3C3C; color: #D4D4D4; border: 1px solid #555555; border-radius: 2px; padding: 4px; selection-background-color: #264F78; }"
                "QLineEdit:focus, QTextEdit:focus, QComboBox:focus { border: 1px solid #007ACC; }"
                "QPushButton { background: #0E639C; color: #FFFFFF; border: none; padding: 6px 16px; border-radius: 2px; }"
                "QPushButton:hover { background: #1177BB; }"
            );
        } else {
            setStyleSheet(
                "QDialog { background: #F0F0F0; color: #000000; }"
                "QLabel, QCheckBox, QRadioButton { color: #000000; }"
                "QLineEdit, QTextEdit, QPlainTextEdit, QComboBox { background: #FFFFFF; color: #000000; border: 1px solid #CCCCCC; border-radius: 2px; padding: 4px; selection-background-color: #007ACC; }"
                "QLineEdit:focus, QTextEdit:focus, QComboBox:focus { border: 1px solid #007ACC; }"
                "QPushButton { background: #007ACC; color: #FFFFFF; border: none; padding: 6px 16px; border-radius: 2px; }"
                "QPushButton:hover { background: #0062A3; }"
            );
        }
    }

    /// Muestra el diálogo como modeless (no bloquea la ventana principal).

    /// Equivalente a CreateDialogParam + ShowWindow en Win32.
    virtual void create(QWidget* parent = nullptr) {
        if (parent) setParent(parent);
        setModal(false);
        show();
    }

    /// Muestra el diálogo como modal (bloquea hasta que el usuario cierra).
    /// Equivalente a DialogBoxParam en Win32.
    virtual int doModal(QWidget* parent = nullptr) {
        if (parent) setParent(parent);
        setModal(true);
        return exec();
    }

    /// Método virtual que las subclases sobrescriben para inicializar su contenido.
    /// Equivalente a WM_INITDIALOG.
    virtual void onInit() {}

    /// Mostrar el diálogo.
    void display(bool toShow = true) {
        if (toShow) {
            show();
            raise();
            activateWindow();
        } else {
            hide();
        }
    }

protected:
    /// Tecla Escape cierra el diálogo.
    void keyPressEvent(QKeyEvent* event) override {
        if (event->key() == Qt::Key_Escape) {
            close();
            return;
        }
        QDialog::keyPressEvent(event);
    }

    /// Llamado al mostrarse por primera vez.
    void showEvent(QShowEvent* event) override {
        static bool firstShow = true;
        if (firstShow) {
            firstShow = false;
            onInit();
        }
        QDialog::showEvent(event);
    }
};

#endif // NPP_PLATFORM_LINUX
