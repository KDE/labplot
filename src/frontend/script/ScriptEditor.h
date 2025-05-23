#ifndef SCRIPTEDITOR_H
#define SCRIPTEDITOR_H

#include <QWidget>
#include "ui_scripteditorwidget.h"

class QAction;
class QMenu;
class Script;
class QToolBar;
class QToolButton;
namespace KTextEditor{
class View;
}

class ScriptEditor : public QWidget {
    Q_OBJECT

public:
    explicit ScriptEditor(Script*, QWidget* parent = nullptr);
    ~ScriptEditor();
    void fillToolBar(QToolBar*);
    void writeOutput(bool, const QString&);
    void setEditorFont(const QFont&);
    QFont editorFont();
    void setOutputFont(const QFont&);
	QFont outputFont();
    void setEditorTheme(const QString&);
	QString editorTheme();
    QString outputText();

protected:
    bool eventFilter(QObject* object, QEvent* event) override;

public Q_SLOTS:
	void createContextMenu(QMenu*);

private:
    Ui::ScriptEditorWidget ui;
    Script* m_script{nullptr};
    KTextEditor::View* m_kTextEditorView{nullptr};
    QAction* m_runScriptAction{nullptr};
    QAction* m_clearOutputAction{nullptr};
    bool m_firstOutputOfRun{false};
    QFont m_lastEditorFont;
    QString m_lastEditorTheme;

    void initActions();
    void initMenus();
    void setSplitterState(const QByteArray&);
	QByteArray splitterState();
};
#endif