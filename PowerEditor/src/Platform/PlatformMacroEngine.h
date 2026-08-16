// Platform/PlatformMacroEngine.h — Motor de Grabación y Reproducción de Macros Qt6
// Reemplaza Macro/ (Macro.cpp/h, MacroPlaybackDlg.cpp/h) en Linux.
//
// Permite grabar acciones del editor Scintilla, guardarlas en memoria/XML y reproducirlas N veces.
//
// Copyright (C) Notepad++ contributors. GPL v3+

#pragma once

#include "PlatformTypes.h"

#ifdef NPP_PLATFORM_LINUX

#include <Qsci/qsciscintilla.h>
#include <QString>
#include <QVector>
#include <QMap>

/// Representa un paso grabado en una macro.
struct MacroStep {
    unsigned int msg;
    unsigned long wParam;
    long lParam;
    QString text;
};

/// Motor de grabación y reproducción de macros para Notepad++ Linux.
class NppMacroEngine {
public:
    static NppMacroEngine& instance() {
        static NppMacroEngine inst;
        return inst;
    }

    /// Inicia la grabación de una nueva macro.
    void startRecording() {
        _isRecording = true;
        _currentMacro.clear();
    }

    /// Detiene la grabación actual.
    void stopRecording() {
        _isRecording = false;
    }

    /// ¿Se está grabando una macro en este momento?
    bool isRecording() const {
        return _isRecording;
    }

    /// Registra una acción ejecutada en el editor.
    void recordStep(unsigned int msg, unsigned long wParam = 0, long lParam = 0, const QString& text = QString()) {
        if (!_isRecording) return;
        _currentMacro.append({msg, wParam, lParam, text});
    }

    /// Reproduce la macro actual N veces sobre un editor.
    void playMacro(QsciScintilla* editor, int times = 1) {
        if (!editor || _currentMacro.isEmpty()) return;

        editor->beginUndoAction();
        for (int t = 0; t < times; ++t) {
            for (const auto& step : _currentMacro) {
                if (!step.text.isEmpty()) {
                    editor->insert(step.text);
                } else if (step.msg > 0) {
                    editor->SendScintilla(step.msg, step.wParam, step.lParam);
                }
            }
        }
        editor->endUndoAction();
    }

    /// Guardar la macro actual con un nombre.
    void saveCurrentMacro(const QString& name) {
        if (!name.isEmpty() && !_currentMacro.isEmpty()) {
            _savedMacros[name] = _currentMacro;
        }
    }

    /// Reproducir una macro guardada por nombre.
    void playSavedMacro(const QString& name, QsciScintilla* editor, int times = 1) {
        if (_savedMacros.contains(name)) {
            auto original = _currentMacro;
            _currentMacro = _savedMacros[name];
            playMacro(editor, times);
            _currentMacro = original;
        }
    }

    /// Obtener lista de macros guardadas.
    QStringList savedMacroNames() const {
        return _savedMacros.keys();
    }

    /// ¿Hay alguna macro grabada lista para reproducir?
    bool hasRecordedMacro() const {
        return !_currentMacro.isEmpty();
    }

private:
    NppMacroEngine() = default;

    bool _isRecording = false;
    QVector<MacroStep> _currentMacro;
    QMap<QString, QVector<MacroStep>> _savedMacros;
};

#endif // NPP_PLATFORM_LINUX
