/*
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation; either version 2
    of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA  02110-1301, USA.

    ---
    Copyright (C) 2009 Alexander Rieder <alexanderrieder@gmail.com>
*/

#ifndef CANTORPART_H
#define CANTORPART_H

#include <QPointer>

#include <KParts/ReadWritePart>
#include <cantor/session.h>

class QWidget;
class Worksheet;
class WorksheetView;
class SarchBar;
class SearchBar;
class ScriptEditorWidget;
class KAboutData;
class QAction;
class KToggleAction;
class KProgressDialog;

namespace Cantor {
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
    CantorPart(QWidget *parentWidget,QObject *parent, const QVariantList &args);

    /**
     * Destructor
     */
    ~CantorPart() override;

    /**
     * This is a virtual function inherited from KParts::ReadWritePart.
     * A shell will use this to inform this Part if it should act
     * read-only
     */
    void setReadWrite(bool rw) override;

    /**
     * Reimplemented to disable and enable Save action
     */
    void setModified(bool modified) override;

    KAboutData& createAboutData();

    Worksheet* worksheet();

Q_SIGNALS:
    void setCaption(const QString& caption);
    void showHelp(const QString& help);

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
    void guiActivateEvent( KParts::GUIActivateEvent * event ) override;


    void loadAssistants();
    void adjustGuiToSession();

protected Q_SLOTS:
    void fileSaveAs();
    void fileSavePlain();
    void exportToLatex();
    void evaluateOrInterrupt();
    void restartBackend();
    void enableTypesetting(bool enable);
    void showBackendHelp();
    void print();
    void printPreview();

    void worksheetStatusChanged(Cantor::Session::Status stauts);
    void showSessionError(const QString& error);
    void worksheetSessionChanged();
    void initialized();
    void updateCaption();

    void pluginsChanged();
    void runCommand(const QString& value);

    void runAssistant();
    void publishWorksheet();

    void showScriptEditor(bool show);
    void scriptEditorClosed();
    void runScript(const QString& file);

    void showSearchBar();
    void showExtendedSearchBar();
    void findNext();
    void findPrev();
    void searchBarDeleted();

    /** sets the status message, or cached it, if the StatusBar is blocked.
     *  Use this method instead of "emit setStatusBarText"
     */
    void setStatusMessage(const QString& message);
    /** Shows an important status message. It makes sure the message is displayed,
     *  by blocking the statusbarText for 3 seconds
     */
    void showImportantStatusMessage(const QString& message);
    /** Blocks the StatusBar for new messages, so the currently shown one won't be overridden
     */
    void blockStatusBar();
    /** Removes the block from the StatusBar, and shows the last one of the StatusMessages that
        where set during the block
    **/
    void unblockStatusBar();
private:
    Worksheet *m_worksheet;
    WorksheetView *m_worksheetview;
    SearchBar *m_searchBar;
    QPointer<ScriptEditorWidget> m_scriptEditor;
    Cantor::PanelPluginHandler* m_panelHandler;

    KProgressDialog* m_initProgressDlg;
    QAction * m_evaluate;
    QAction * m_save;
    QAction * m_findNext;
    QAction * m_findPrev;
    KToggleAction* m_typeset;
    KToggleAction* m_highlight;
    KToggleAction* m_completion;
    KToggleAction* m_exprNumbering;
    KToggleAction* m_animateWorksheet;
    QAction * m_showBackendHelp;

    QString m_cachedStatusMessage;
    bool m_statusBarBlocked;
};

#endif // CANTORPART_H
