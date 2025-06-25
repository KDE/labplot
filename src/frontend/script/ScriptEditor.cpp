#include <KTextEditor/Editor>
#include <KTextEditor/Document>
#include <KTextEditor/View>

#include <QPushButton>
#include <QHBoxLayout>
#include <QMenu>
#include <QAction>
#include <QToolBar>
#include <QString>
#include <QToolButton>
#include <QVariant>
#include <QFont>
#include <QObject>
#include <QPlainTextEdit>

#include <KConfig>
#include <KConfigGroup>

#include "ScriptEditor.h"
#include "backend/script/Script.h"

ScriptEditor::ScriptEditor(Script* script, QWidget* parent)
    : QWidget(parent)
    , m_script(script) {

    ui.setupUi(this);
    m_kTextEditorView = m_script->kTextEditorDocument()->createView(this);
    m_kTextEditorView->setContextMenu(m_kTextEditorView->defaultContextMenu(nullptr));
    ui.editorParent->layout()->addWidget(m_kTextEditorView);
    
    KConfig config;
	KConfigGroup group = config.group(QStringLiteral("ScriptEditor"));
	// we dont manage default editor font or themes ourselves, so no need to check our config
    setOutputFont(group.readEntry(QStringLiteral("OutputFont"), QFont(QStringLiteral("monospace"), 10)));
    setSplitterState(group.readEntry(QStringLiteral("SplitterState"), splitterState())); // need reasonable default for splitter

    initActions();
    
    connect(m_script, &Script::requestProjectContextMenu, this, &ScriptEditor::createContextMenu);
    connect(m_script, &Script::viewPrint, [kTextEditorView = m_kTextEditorView] {
        kTextEditorView->print();
    });
    connect(m_script, &Script::viewPrintPreview, [kTextEditorView = m_kTextEditorView] {
        kTextEditorView->printPreview();
    });

    ui.output->setReadOnly(true);
}

ScriptEditor::~ScriptEditor() {    
    KConfig config;
	KConfigGroup group = config.group(QStringLiteral("ScriptEditor"));
    // we dont manage default editor font or themes ourselves, so no need to save in our config
    group.writeEntry(QStringLiteral("OutputFont"), outputFont());
    group.writeEntry(QStringLiteral("SplitterState"), splitterState());
}

bool ScriptEditor::isInitialized() const {
    if (!m_script)
        return false;
    return m_script->isInitialized();
}

void ScriptEditor::createContextMenu(QMenu* menu) {
	Q_ASSERT(menu);
    if (!m_script)
        return;

    if (!m_script->isInitialized()) {
        m_runScriptAction->setEnabled(false);
        m_clearOutputAction->setEnabled(false);
    } else {
        m_runScriptAction->setEnabled(true);
        m_clearOutputAction->setEnabled(true);
    }

    QAction* firstAction = nullptr;

    if (menu->actions().size() > 1)
        firstAction = menu->actions().at(1);

    menu->insertAction(firstAction, m_runScriptAction);

    menu->insertSeparator(firstAction);

    menu->insertAction(firstAction, m_clearOutputAction);

    if (firstAction)
        menu->insertSeparator(firstAction);
}

void ScriptEditor::initActions() {
    m_runScriptAction = new QAction(QIcon::fromTheme(QStringLiteral("quickopen")), QStringLiteral("Run"), this);
	m_runScriptAction->setWhatsThis(QStringLiteral("Run the script"));
	connect(m_runScriptAction, &QAction::triggered, this, &ScriptEditor::run);

    m_clearOutputAction = new QAction(QIcon::fromTheme(QStringLiteral("edit-clear")), QStringLiteral("Clear Output"), this);
	m_clearOutputAction->setWhatsThis(QStringLiteral("Clear the output of the script editor"));
	connect(m_clearOutputAction, &QAction::triggered, this, &ScriptEditor::clearOutput);
}

void ScriptEditor::writeOutput(bool /*isErr*/, const QString& msg) {
    if (msg.trimmed().isEmpty())
        return;

    ui.output->setPlainText(msg);
}

void ScriptEditor::setSplitterState(const QByteArray& state) {
    ui.splitter->restoreState(state);
}

QByteArray ScriptEditor::splitterState() {
    return ui.splitter->saveState();
}

QString ScriptEditor::outputText() {
    return ui.output->toPlainText();
}

void ScriptEditor::setOutputFont(const QFont& font) {
    ui.output->setFont(font);
}

QFont ScriptEditor::outputFont() {
    return ui.output->font();
}

void ScriptEditor::registerShortcuts() {

}

void ScriptEditor::unregisterShortcuts() {

}

// ##############################################################################
// ####################################  SLOTs   ################################
// ##############################################################################
void ScriptEditor::run() {
    m_script->runScript();
}

void ScriptEditor::clearOutput() {
    QFont currentOutputFont = outputFont();
    ui.output->clear();
    ui.output->setReadOnly(true);
    setOutputFont(currentOutputFont);
}
