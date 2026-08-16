// Platform/PlatformCrashHandler.h — Crash handler portable (Linux)
// Reemplaza MISC/MiniDumper/MiniDumper.cpp y Win32Exception en Linux
//
// En Windows: MiniDumpWriteDump() + SetUnhandledExceptionFilter()
// En Linux:   sigaction(SIGSEGV) + backtrace() + backtrace_symbols()
//
// Copyright (C) Notepad++ contributors. GPL v3+

#pragma once

#include "PlatformTypes.h"

#ifdef NPP_PLATFORM_LINUX

#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <execinfo.h>  // backtrace, backtrace_symbols
#include <unistd.h>

/// Crash handler portable para Notepad++ Linux.
/// Equivalente a MiniDumper + Win32Exception de la versión Windows.
/// Captura SIGSEGV, SIGABRT, SIGFPE y genera un reporte con backtrace.
class NppCrashHandler {
public:
    /// Instala los signal handlers. Llamar una vez al inicio de la aplicación.
    static void install() {
        struct sigaction sa;
        std::memset(&sa, 0, sizeof(sa));
        sa.sa_sigaction = &signalHandler;
        sa.sa_flags = SA_SIGINFO | SA_RESETHAND; // auto-reset para evitar recursión

        sigaction(SIGSEGV, &sa, nullptr);
        sigaction(SIGABRT, &sa, nullptr);
        sigaction(SIGFPE,  &sa, nullptr);
        sigaction(SIGBUS,  &sa, nullptr);
        sigaction(SIGILL,  &sa, nullptr);
    }

private:
    static constexpr int MAX_FRAMES = 64;

    static void signalHandler(int sig, siginfo_t* info, void* /*context*/) {
        // Nombre de la señal
        const char* sigName = "UNKNOWN";
        switch (sig) {
            case SIGSEGV: sigName = "SIGSEGV (Segmentation fault)"; break;
            case SIGABRT: sigName = "SIGABRT (Abort)"; break;
            case SIGFPE:  sigName = "SIGFPE (Floating point exception)"; break;
            case SIGBUS:  sigName = "SIGBUS (Bus error)"; break;
            case SIGILL:  sigName = "SIGILL (Illegal instruction)"; break;
        }

        // Generar nombre de archivo de crash dump
        char dumpFile[256];
        time_t now = time(nullptr);
        struct tm* tm = localtime(&now);
        snprintf(dumpFile, sizeof(dumpFile),
                 "/tmp/notepadplusplus_crash_%04d%02d%02d_%02d%02d%02d.log",
                 tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
                 tm->tm_hour, tm->tm_min, tm->tm_sec);

        // Escribir crash report al archivo
        FILE* fp = fopen(dumpFile, "w");
        if (!fp) fp = stderr;

        fprintf(fp, "=== Notepad++ Linux Crash Report ===\n");
        fprintf(fp, "Signal: %s (code: %d)\n", sigName, sig);
        fprintf(fp, "Fault address: %p\n", info->si_addr);
        fprintf(fp, "PID: %d\n", getpid());
        fprintf(fp, "Time: %s\n", asctime(tm));
        fprintf(fp, "\n--- Backtrace ---\n");

        // Capturar backtrace
        void* frames[MAX_FRAMES];
        int frameCount = backtrace(frames, MAX_FRAMES);
        char** symbols = backtrace_symbols(frames, frameCount);

        if (symbols) {
            for (int i = 0; i < frameCount; ++i) {
                fprintf(fp, "  [%2d] %s\n", i, symbols[i]);
            }
            free(symbols);
        } else {
            fprintf(fp, "  (no se pudieron obtener símbolos)\n");
            // Escribir directamente a stderr como fallback
            backtrace_symbols_fd(frames, frameCount, STDERR_FILENO);
        }

        fprintf(fp, "\n--- Fin del reporte ---\n");

        if (fp != stderr) {
            fclose(fp);
            // Imprimir a stderr también
            fprintf(stderr,
                    "\n*** Notepad++ ha encontrado un error fatal: %s ***\n"
                    "Crash dump guardado en: %s\n"
                    "Por favor, reporta este error en:\n"
                    "  https://github.com/edoom85/notepad-plus-plus/issues\n\n",
                    sigName, dumpFile);
        }

        // Terminar el proceso
        _exit(128 + sig);
    }
};

#endif // NPP_PLATFORM_LINUX
