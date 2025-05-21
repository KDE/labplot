#ifndef SCRIPT_H
#define SCRIPT_H

#include "backend/core/AbstractPart.h"
#include "backend/core/Folder.h"

class ScriptEditor;
namespace KTextEditor {
class Document;
}
class ScriptRuntime;

class Script : public AbstractPart {
	Q_OBJECT

public:
	explicit Script(const QString&, const QString&);
	~Script();

	QWidget* view() const override;
	QIcon icon() const override;

	bool exportView() const override;
	bool printView() override;
	bool printPreview() const override;

	QMenu* createContextMenu() override;

	void save(QXmlStreamWriter*) const override;
	bool load(XmlStreamReader*, bool preview) override;

	ScriptRuntime* scriptRuntime() const;
	int scriptErrorLine() const;
	KTextEditor::Document* kTextEditorDocument() const;
	QString language() const;
	void setEditorFont(const QFont&);
	QFont editorFont() const;
	void setOutputFont(const QFont&);
	QFont outputFont() const;
	void setEditorTheme(const QString&);
	QString editorTheme() const;

	bool isInitialized();

private:
	QString m_language;
	ScriptRuntime* m_scriptRuntime{nullptr};
	mutable ScriptEditor* m_view{nullptr};
	KTextEditor::Document* m_kTextEditorDocument{nullptr};
	bool m_initialized{false};

	void prepareDocument() const;

public Q_SLOTS:
	void runScript();
	void cancelScript();

Q_SIGNALS:
	void requestProjectContextMenu(QMenu*);
    void viewPrint();
    void viewPrintPreview() const;
	void editorFontChanged(const QFont&);
	void outputFontChanged(const QFont&);
	void editorThemeChanged(const QString&);

public:
    static QStringList languages;

	static QString readRuntime(XmlStreamReader*);

private:
    static bool m_isRunning;
    static Script* m_lastRunScript;

	static ScriptRuntime* newScriptRuntime(const QString&, Script*);
    static void runScript(Script*, const QString&);
    static bool cancelScript(Script*);
};

#endif