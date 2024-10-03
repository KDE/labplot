/*
 *    SPDX-License-Identifier: GPL-2.0-or-later
 *    SPDX-FileCopyrightText: 2009 Alexander Rieder <alexanderrieder@gmail.com>
 */

#ifndef CANTORPART_H
#define CANTORPART_H

#include <QPointer>
#include <QRegularExpression>
#include <QVector>

#include <KParts/ReadWritePart>
#include "cantor/session.h"

class QWidget;
class Worksheet;
class WorksheetView;
class SearchBar;
class ScriptEditorWidget;
class KAboutData;
class QAction;
class KToggleAction;
class KSelectAction;

namespace Cantor{
    class PanelPluginHandler;
}

/**
 * This is a "Part".  It that does all the real work in a KPart
 * application.
 *
 * @short Main Part
 * @author Alexander Rieder <alexanderrieder@gmail.com>
 */
class CantorPart : public KParts::ReadWritePart
{
    Q_OBJECT
public:
    /**
     * Default constructor
     */
    CantorPart(QObject *parent, const QVariantList &args);

    /**
     * Destructor
     */
    ~CantorPart() override;

    /**
     * This is a virtual function inherited from KParts::ReadWritePart.
     * A shell will use this to inform this Part if it should act
     * read-only
     */
    void setReadWrite(bool) override;

    /**
     * Reimplemented to disable and enable Save action
     */
    void setModified(bool) override;

    KAboutData& createAboutData();

    Worksheet* worksheet();

Q_SIGNALS:
    void setCaption(const QString& caption, const QIcon& icon);
    void showHelp(const QString&);
    void hierarchyChanged(QStringList, QStringList, QList<int>);
    void hierarhyEntryNameChange(QString name, QString searchName, int depth);
    void worksheetSave(const QUrl&);
    void setBackendName(const QString&);
    void requestScrollToHierarchyEntry(QString);
    void settingsChanges();
    void requestDocumentation(const QString& keyword);

public Q_SLOTS:
    void updateCaption();

protected:
    /**
     * This must be implemented by each part
     */
    bool openFile() override;

    /**
     * This must be implemented by each read-write part
     */
    bool saveFile() override;

    /**
     * Called when this part becomes the active one,
     * or if it looses activity
     **/
    void guiActivateEvent(KParts::GUIActivateEvent*) override;

    void loadAssistants();
    void adjustGuiToSession();

    void setReadOnly();

protected Q_SLOTS:
    void fileSaveAs();
    void fileSavePlain();
    void exportToPDF();
    void exportToLatex();
    void evaluateOrInterrupt();
    void restartBackend();
    void zoomValueEdited(const QString&);
    void updateZoomWidgetValue(double);
    void enableTypesetting(bool);
    void showBackendHelp();
    void print();
    void printPreview();

    void worksheetStatusChanged(Cantor::Session::Status);
    void showSessionError(const QString&);
    void worksheetSessionLoginStarted();
    void worksheetSessionLoginDone();
    void initialized();

    void runCommand(const QString&);

    void runAssistant();
    void publishWorksheet();

    void showScriptEditor(bool);
    void scriptEditorClosed();
    void runScript(const QString&);

    void showSearchBar();
    void showExtendedSearchBar();
    void findNext();
    void findPrev();
    void searchBarDeleted();

    /** sets the status message, or cached it, if the StatusBar is blocked.
     *  Use this method instead of "emit setStatusBarText"
     */
    void setStatusMessage(const QString&);
    /** Shows an important status message. It makes sure the message is displayed,
     *  by blocking the statusbarText for 3 seconds
     */
    void showImportantStatusMessage(const QString&);
    /** Blocks the StatusBar for new messages, so the currently shown one won't be overridden
     */
    void blockStatusBar();
    /** Removes the block from the StatusBar, and shows the last one of the StatusMessages that
     *        where set during the block
     **/
    void unblockStatusBar();

private:
    Worksheet* m_worksheet{nullptr};
    WorksheetView* m_worksheetview{nullptr};
    SearchBar* m_searchBar{nullptr};
    QPointer<ScriptEditorWidget> m_scriptEditor;

    QAction* m_evaluate;
    QAction* m_restart;
    KSelectAction* m_zoom;
    QAction* m_currectZoomAction{nullptr};
    QAction* m_save;
    QAction* m_findNext;
    QAction* m_findPrev;
    KToggleAction* m_typeset;
    KToggleAction* m_highlight;
    KToggleAction* m_completion;
    KToggleAction* m_exprNumbering;
    KToggleAction* m_animateWorksheet;
    KToggleAction* m_embeddedMath;
    QVector<QAction*> m_editActions;

    QString m_cachedStatusMessage;
    bool m_statusBarBlocked{false};
    unsigned int m_sessionStatusCounter{0};
    const QRegularExpression m_zoomRegexp{QLatin1String("(?:%?(\\d+(?:\\.\\d+)?)(?:%|\\s*))")};

private Q_SLOTS:
    void documentationRequested(const QString&);
};

#endif // CANTORPART_H
