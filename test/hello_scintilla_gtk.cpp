// test/hello_scintilla_gtk.cpp — Prueba mínima de Scintilla con GTK3
// Fase 2 del port Linux de Notepad++
//
// Compilar (desde el directorio raíz del repo):
//   g++ -std=c++17 -DGTK test/hello_scintilla_gtk.cpp \
//       -I scintilla/include -I lexilla/include \
//       $(pkg-config --cflags gtk+-3.0) \
//       -L scintilla/bin -lscintilla \
//       -L lexilla/bin -llexilla \
//       -Wl,-rpath,$(pwd)/scintilla/bin:$(pwd)/lexilla/bin \
//       -o test/hello_scintilla_gtk

#include <gtk/gtk.h>
#include <Scintilla.h>
#include <SciLexer.h>
#include <ScintillaWidget.h>
#include <ILexer.h>
#include <Lexilla.h>

static const char* SAMPLE_TEXT =
    "// Notepad++ Linux Port -- Test de Scintilla + GTK\n"
    "// Si ves colores de sintaxis, Scintilla funciona en Linux!\n"
    "\n"
    "#include <iostream>\n"
    "\n"
    "int main() {\n"
    "    // Hola desde el port Linux de Notepad++\n"
    "    std::cout << \"Hello from Notepad++ Linux Port!\" << std::endl;\n"
    "    return 0;\n"
    "}\n";

static void on_activate(GtkApplication* app, gpointer /*userdata*/) {
    GtkWidget* window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "NPP Linux -- Scintilla Test");
    gtk_window_set_default_size(GTK_WINDOW(window), 900, 600);

    GtkWidget* sci = scintilla_new();
    if (!sci) { g_critical("No se pudo crear Scintilla"); return; }

    auto SCI = [sci](unsigned int msg, uptr_t wp = 0, sptr_t lp = 0) -> sptr_t {
        return scintilla_send_message(SCINTILLA(sci), msg, wp, lp);
    };

    // Configuracion basica
    SCI(SCI_SETCODEPAGE, SC_CP_UTF8);
    SCI(SCI_SETTABWIDTH, 4);
    SCI(SCI_SETMARGINWIDTHN, 0, 44);
    SCI(SCI_SETMARGINTYPEN,  0, SC_MARGIN_NUMBER);

    // Tema oscuro VS Code-like
    SCI(SCI_STYLESETBACK, STYLE_DEFAULT, 0x1E1E1E);
    SCI(SCI_STYLESETFORE, STYLE_DEFAULT, 0xD4D4D4);
    SCI(SCI_STYLECLEARALL);
    SCI(SCI_SETCARETFORE, 0xFFFFFF);
    SCI(SCI_SETSELBACK, 1, 0x264F78);
    SCI(SCI_STYLESETBACK, STYLE_LINENUMBER, 0x252526);
    SCI(SCI_STYLESETFORE, STYLE_LINENUMBER, 0x858585);

    // Lexer C++ via Lexilla
    ILexer5* lexer = CreateLexer("cpp");
    if (lexer) {
        SCI(SCI_SETILEXER, 0, reinterpret_cast<sptr_t>(lexer));
        SCI(SCI_SETKEYWORDS, 0, reinterpret_cast<sptr_t>(
            "alignas alignof auto bool break case catch char class const constexpr "
            "continue decltype default delete do double else enum explicit extern "
            "false float for friend goto if inline int long mutable namespace new "
            "noexcept nullptr operator private protected public return short signed "
            "sizeof static struct switch template this throw true try typedef "
            "typename union unsigned using virtual void volatile while"));
        // Colores sintaxis
        SCI(SCI_STYLESETFORE, SCE_C_COMMENT,      0x57A64A);
        SCI(SCI_STYLESETFORE, SCE_C_COMMENTLINE,  0x57A64A);
        SCI(SCI_STYLESETFORE, SCE_C_STRING,        0xD69D85);
        SCI(SCI_STYLESETFORE, SCE_C_WORD,          0x569CD6);
        SCI(SCI_STYLESETFORE, SCE_C_NUMBER,        0xB5CEA8);
        SCI(SCI_STYLESETFORE, SCE_C_PREPROCESSOR,  0x9B9B9B);
        SCI(SCI_STYLESETBOLD,  SCE_C_WORD, 1);
    }

    SCI(SCI_SETTEXT, 0, reinterpret_cast<sptr_t>(SAMPLE_TEXT));
    SCI(SCI_SETSAVEPOINT);
    SCI(SCI_EMPTYUNDOBUFFER);

    gtk_container_add(GTK_CONTAINER(window), sci);
    gtk_widget_show_all(window);

    g_print("OK Scintilla GTK3 + Lexilla funcionando.\n");
    g_print("   Lexer CPP: %s\n", lexer ? "OK" : "FALLO");
}

int main(int argc, char* argv[]) {
    GtkApplication* app = gtk_application_new(
        "org.notepadpp.scintilla_test", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(on_activate), nullptr);
    int s = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    return s;
}
