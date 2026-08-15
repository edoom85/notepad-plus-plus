// test/hello_scintilla_qt.cpp — Prueba mínima de QScintilla con Qt6
// Fase 2 del port Linux de Notepad++
//
// Compilar (desde el directorio raíz del repo):
//   g++ -std=c++17 test/hello_scintilla_qt.cpp \
//       $(pkg-config --cflags --libs Qt6Widgets) \
//       -lqscintilla2_qt6 -fPIC \
//       -o test/hello_scintilla_qt
//
// O con qmake:
//   QT += widgets
//   LIBS += -lqscintilla2_qt6

#include <QApplication>
#include <QMainWindow>
#include <QMenuBar>
#include <QStatusBar>
#include <QVBoxLayout>
#include <QPalette>
#include <QFont>
#include <QColor>
#include <QAction>
#include <QFileDialog>
#include <QMessageBox>
#include <QScreen>

#include <Qsci/qsciscintilla.h>
#include <Qsci/qscilexercpp.h>

static const char* SAMPLE_TEXT =
    "// Notepad++ Linux Port -- Test de QScintilla + Qt6\n"
    "// Si ves colores de sintaxis, QScintilla funciona!\n"
    "\n"
    "#include <iostream>\n"
    "#include <string>\n"
    "#include <vector>\n"
    "\n"
    "int main() {\n"
    "    std::string greeting = \"Hello from Notepad++ Linux Port!\";\n"
    "    std::vector<int> numbers = {1, 2, 3, 4, 5};\n"
    "\n"
    "    for (const auto& n : numbers) {\n"
    "        std::cout << greeting << \" #\" << n << std::endl;\n"
    "    }\n"
    "\n"
    "    return 0;\n"
    "}\n";

// ─── Aplicar tema oscuro estilo VS Code ──────────────────────────────────────
static void applyDarkTheme(QApplication& app) {
    app.setStyle("Fusion");
    QPalette dark;
    dark.setColor(QPalette::Window,          QColor(0x1E, 0x1E, 0x1E));
    dark.setColor(QPalette::WindowText,      QColor(0xD4, 0xD4, 0xD4));
    dark.setColor(QPalette::Base,            QColor(0x1E, 0x1E, 0x1E));
    dark.setColor(QPalette::AlternateBase,   QColor(0x25, 0x25, 0x26));
    dark.setColor(QPalette::ToolTipBase,     QColor(0x25, 0x25, 0x26));
    dark.setColor(QPalette::ToolTipText,     QColor(0xD4, 0xD4, 0xD4));
    dark.setColor(QPalette::Text,            QColor(0xD4, 0xD4, 0xD4));
    dark.setColor(QPalette::Button,          QColor(0x33, 0x33, 0x33));
    dark.setColor(QPalette::ButtonText,      QColor(0xD4, 0xD4, 0xD4));
    dark.setColor(QPalette::Link,            QColor(0x56, 0x9C, 0xD6));
    dark.setColor(QPalette::Highlight,       QColor(0x26, 0x4F, 0x78));
    dark.setColor(QPalette::HighlightedText, QColor(0xFF, 0xFF, 0xFF));
    app.setPalette(dark);
}

// ─── Configurar el editor QScintilla ─────────────────────────────────────────
static QsciScintilla* createEditor(QWidget* parent) {
    auto* editor = new QsciScintilla(parent);

    // Fuente monoespaciada
    QFont font("JetBrains Mono", 11);
    if (!font.exactMatch()) font = QFont("Monospace", 11);
    editor->setFont(font);

    // Márgenes — números de línea
    editor->setMarginType(0, QsciScintilla::NumberMargin);
    editor->setMarginWidth(0, "00000");
    editor->setMarginsForegroundColor(QColor(0x85, 0x85, 0x85));
    editor->setMarginsBackgroundColor(QColor(0x25, 0x25, 0x26));
    editor->setMarginsFont(font);

    // Colores del editor
    editor->setPaper(QColor(0x1E, 0x1E, 0x1E));      // fondo
    editor->setColor(QColor(0xD4, 0xD4, 0xD4));      // texto
    editor->setCaretForegroundColor(QColor(0xFF, 0xFF, 0xFF));
    editor->setSelectionBackgroundColor(QColor(0x26, 0x4F, 0x78));
    editor->setSelectionForegroundColor(QColor(0xFF, 0xFF, 0xFF));
    editor->setCaretLineVisible(true);
    editor->setCaretLineBackgroundColor(QColor(0x28, 0x28, 0x28));

    // Indentación
    editor->setTabWidth(4);
    editor->setIndentationsUseTabs(false);
    editor->setAutoIndent(true);
    editor->setIndentationGuides(true);
    editor->setIndentationGuidesBackgroundColor(QColor(0x40, 0x40, 0x40));
    editor->setIndentationGuidesForegroundColor(QColor(0x40, 0x40, 0x40));

    // Brackets matching
    editor->setBraceMatching(QsciScintilla::SloppyBraceMatch);
    editor->setMatchedBraceBackgroundColor(QColor(0x3A, 0x3A, 0x3A));
    editor->setMatchedBraceForegroundColor(QColor(0xFF, 0xD7, 0x00));

    // Folding (code collapse)
    editor->setFolding(QsciScintilla::BoxedTreeFoldStyle, 2);

    // Lexer C++ con colores VS Code-like
    auto* lexer = new QsciLexerCPP(editor);
    lexer->setFont(font);

    // Colores por tipo de token
    lexer->setColor(QColor(0xD4, 0xD4, 0xD4), QsciLexerCPP::Default);        // texto normal
    lexer->setColor(QColor(0x57, 0xA6, 0x4A), QsciLexerCPP::Comment);         // comentario /**/
    lexer->setColor(QColor(0x57, 0xA6, 0x4A), QsciLexerCPP::CommentLine);     // comentario //
    lexer->setColor(QColor(0x57, 0xA6, 0x4A), QsciLexerCPP::CommentDoc);      // doc comment
    lexer->setColor(QColor(0xB5, 0xCE, 0xA8), QsciLexerCPP::Number);          // números
    lexer->setColor(QColor(0xD6, 0x9D, 0x85), QsciLexerCPP::DoubleQuotedString); // strings
    lexer->setColor(QColor(0xD6, 0x9D, 0x85), QsciLexerCPP::SingleQuotedString); // chars
    lexer->setColor(QColor(0x56, 0x9C, 0xD6), QsciLexerCPP::Keyword);         // keywords
    lexer->setColor(QColor(0x9B, 0x9B, 0x9B), QsciLexerCPP::PreProcessor);    // #include etc.
    lexer->setColor(QColor(0xD4, 0xD4, 0xD4), QsciLexerCPP::Operator);        // operadores
    lexer->setColor(QColor(0x4E, 0xC9, 0xB0), QsciLexerCPP::Identifier);      // identifiers

    // Fondo oscuro para todos los estilos del lexer
    for (int i = 0; i <= QsciLexerCPP::TaskMarker; ++i)
        lexer->setPaper(QColor(0x1E, 0x1E, 0x1E), i);

    // Keywords bold
    QFont boldFont = font;
    boldFont.setBold(true);
    lexer->setFont(boldFont, QsciLexerCPP::Keyword);

    editor->setLexer(lexer);

    // Cargar texto de ejemplo
    editor->setText(SAMPLE_TEXT);

    return editor;
}

// ─── main ─────────────────────────────────────────────────────────────────────
int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("NPP Linux — QScintilla Test");

    applyDarkTheme(app);

    QMainWindow window;
    window.setWindowTitle("Notepad++ Linux Port — QScintilla Qt6 Test");

    // Centrar y dimensionar ventana
    QSize screenSize = app.primaryScreen()->size();
    window.resize(960, 640);
    window.move((screenSize.width() - 960) / 2,
                (screenSize.height() - 640) / 2);

    // Menú básico
    auto* menuBar = window.menuBar();
    auto* fileMenu = menuBar->addMenu("&Archivo");

    auto* openAction = new QAction("&Abrir...", &window);
    openAction->setShortcut(QKeySequence::Open);
    QObject::connect(openAction, &QAction::triggered, [&window]() {
        QString path = QFileDialog::getOpenFileName(&window, "Abrir archivo");
        if (!path.isEmpty()) {
            // En una app real aquí se cargaría el archivo
            QMessageBox::information(&window, "Archivo", "Abrirías: " + path);
        }
    });
    fileMenu->addAction(openAction);

    auto* quitAction = new QAction("&Salir", &window);
    quitAction->setShortcut(QKeySequence::Quit);
    QObject::connect(quitAction, &QAction::triggered, &app, &QApplication::quit);
    fileMenu->addAction(quitAction);

    // Editor central
    QsciScintilla* editor = createEditor(&window);
    window.setCentralWidget(editor);

    // Barra de estado
    window.statusBar()->showMessage("QScintilla Qt6 — Listo");

    window.show();

    qDebug("OK QScintilla Qt6 funcionando.");
    qDebug("   Lexer: QsciLexerCPP");
    qDebug("   Qt: %s", qVersion());

    return app.exec();
}
