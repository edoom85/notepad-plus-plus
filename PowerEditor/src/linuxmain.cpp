// PowerEditor/src/linuxmain.cpp — Punto de entrada Linux para Notepad++
// Parte del plan de migración Notepad++ → Linux nativo
//
// Reemplaza winmain.cpp en Linux. Gestiona:
//   - Inicialización de la aplicación GTK4 (GtkApplication)
//   - Instancia única (evita múltiples ventanas, pasa args a la instancia existente)
//   - Parseo de argumentos de línea de comandos
//   - Arranque del Notepad_plus_Window equivalente en Linux
//
// Copyright (C) Notepad++ contributors. GPL v3+

#include "Platform/PlatformTypes.h"
#include "Platform/PlatformString.h"
#include "Platform/PlatformMsg.h"

#include <gtk/gtk.h>
#include <glib.h>
#include <gio/gio.h>

#include <string>
#include <vector>
#include <filesystem>
#include <iostream>
#include <cstring>

// ─── Forward declarations ──────────────────────────────────────────────────── 
// Declaradas aquí; la implementación estará en NotepadPlusApp_linux.cpp (Fase 3b)
void nppLinuxInit(GtkApplication* app, const std::vector<NppString>& filesToOpen);
void nppLinuxShutdown();

// ─── Application ID ───────────────────────────────────────────────────────────
static constexpr const char* NPP_APP_ID = "org.notepad-plus-plus.Notepad";

// ─── Contexto de activación ───────────────────────────────────────────────────
struct AppContext {
    GtkApplication*         app       = nullptr;
    std::vector<NppString>  openFiles;        // archivos pedidos en args
    bool                    newInstance = false; // -multiInst
    bool                    noPlugin    = false; // -noPlugin
    NppString               language;           // -l <lang>
    int                     lineNumber  = -1;   // -n <line>
    int                     colNumber   = -1;   // -c <col>
};

static AppContext g_ctx;

// ─── Parseo de argumentos ─────────────────────────────────────────────────────
// Equivalente al parseo que hace winmain.cpp con GetCommandLine()
static void parseArgs(int argc, char** argv) {
    for (int i = 1; i < argc; ++i) {
        std::string arg(argv[i]);

        if (arg == "-multiInst") {
            g_ctx.newInstance = true;
        } else if (arg == "-noPlugin") {
            g_ctx.noPlugin = true;
        } else if ((arg == "-l" || arg == "--lang") && i + 1 < argc) {
            g_ctx.language = NppString(argv[++i]);
        } else if ((arg == "-n" || arg == "--line") && i + 1 < argc) {
            g_ctx.lineNumber = std::stoi(argv[++i]);
        } else if ((arg == "-c" || arg == "--col") && i + 1 < argc) {
            g_ctx.colNumber = std::stoi(argv[++i]);
        } else if (arg[0] != '-') {
            // Es un archivo — convertir a ruta absoluta
            namespace fs = std::filesystem;
            fs::path p(arg);
            if (!p.is_absolute()) {
                p = fs::current_path() / p;
            }
            g_ctx.openFiles.push_back(p.string());
        }
        // Ignorar args desconocidos (compatibilidad futura)
    }
}

// ─── Callback: primera activación de la app ───────────────────────────────────
static void on_activate(GApplication* /*gapp*/, gpointer /*userdata*/) {
    // Inicializa la ventana principal Notepad++ en GTK
    nppLinuxInit(g_ctx.app, g_ctx.openFiles);
}

// ─── Callback: segunda instancia intenta abrir archivos ──────────────────────
// GApplication maneja instancia única; cuando se lanza otra instancia con archivos,
// llama a on_open en la instancia ya corriendo.
static void on_open(GApplication* /*gapp*/, GFile** files, gint n_files,
                    const gchar* /*hint*/, gpointer /*userdata*/) {
    std::vector<NppString> paths;
    paths.reserve(static_cast<size_t>(n_files));
    for (int i = 0; i < n_files; ++i) {
        gchar* path = g_file_get_path(files[i]);
        if (path) {
            paths.emplace_back(path);
            g_free(path);
        }
    }
    // Reenviar al bus de mensajes para que la ventana principal los abra
    // Usamos lp como puntero a vector (el receptor lo recibe en el main thread)
    NppMsgBus::instance().post(NppMsg::FILE_OPEN,
        0, reinterpret_cast<NppLparam>(&paths));
}

// ─── Callback: la app recibe argumentos de línea de comandos ─────────────────
static gint on_handle_local_options(GApplication* /*gapp*/,
                                    GVariantDict* /*options*/,
                                    gpointer /*userdata*/) {
    return -1; // -1 = continuar con el procesamiento normal de GApplication
}

// ─── main() ───────────────────────────────────────────────────────────────────
int main(int argc, char* argv[]) {
    // 1. Parsear args antes de que GTK los consuma
    parseArgs(argc, argv);

    // 2. Crear GtkApplication
    //    G_APPLICATION_HANDLES_OPEN: permite que la app reciba archivos de otras instancias
    //    Si se pide -multiInst, usar G_APPLICATION_NON_UNIQUE para no compartir instancia
    GApplicationFlags flags = g_ctx.newInstance
        ? G_APPLICATION_NON_UNIQUE
        : G_APPLICATION_HANDLES_OPEN;

    g_ctx.app = gtk_application_new(NPP_APP_ID, flags);
    if (!g_ctx.app) {
        std::cerr << "Error: No se pudo crear GtkApplication\n";
        return 1;
    }

    // 3. Conectar señales
    g_signal_connect(g_ctx.app, "activate",             G_CALLBACK(on_activate),             nullptr);
    g_signal_connect(g_ctx.app, "open",                 G_CALLBACK(on_open),                 nullptr);
    g_signal_connect(g_ctx.app, "handle-local-options", G_CALLBACK(on_handle_local_options), nullptr);

    // 4. Si hay archivos en los args, convertirlos a GFile para que GApplication
    //    los maneje correctamente (on_open se dispara en instancia existente si ya corre)
    int status;
    if (!g_ctx.openFiles.empty() && !g_ctx.newInstance) {
        // Construir argv[] solo con los archivos para g_application_run
        std::vector<char*> fakeArgv;
        fakeArgv.push_back(argv[0]);
        std::vector<std::string> paths;
        for (auto& f : g_ctx.openFiles) paths.push_back(f);
        for (auto& p : paths) fakeArgv.push_back(p.data());
        int fakeArgc = static_cast<int>(fakeArgv.size());
        status = g_application_run(G_APPLICATION(g_ctx.app), fakeArgc, fakeArgv.data());
    } else {
        status = g_application_run(G_APPLICATION(g_ctx.app), 1, argv);
    }

    // 5. Cleanup
    nppLinuxShutdown();
    g_object_unref(g_ctx.app);
    return status;
}
