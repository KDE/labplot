#include "DatapickerView.h"
#include "backend/core/AbstractAspect.h"
#include "backend/core/Datapicker.h"
#include "backend/lib/macros.h"
#include "backend/spreadsheet/Spreadsheet.h"
#include "backend/worksheet/Worksheet.h"

#include <QTabWidget>
#include <QHBoxLayout>
#include <QMenu>
#include <QDebug>

#include <KAction>
#include <KLocale>

/*!
    \class DatapickerView
    \brief View class for datapicker

	\ingroup commonfrontend
 */
DatapickerView::DatapickerView(Datapicker* datapicker) : QWidget(),
	m_tabWidget(new QTabWidget(this)),
    m_datapicker(datapicker),
	lastSelectedIndex(0) {

	m_tabWidget->setTabPosition(QTabWidget::South);
	m_tabWidget->setTabShape(QTabWidget::Rounded);
	m_tabWidget->setMovable(true);
	m_tabWidget->setContextMenuPolicy(Qt::CustomContextMenu);

	QHBoxLayout* layout = new QHBoxLayout(this);
	layout->setContentsMargins(0,0,0,0);
	layout->addWidget(m_tabWidget);
	m_tabWidget->show();
	this->resize(200, 200);

    //SIGNALs/SLOTs
    connect(m_datapicker, SIGNAL(aspectAdded(const AbstractAspect*)), this, SLOT(handleAspectAdded(const AbstractAspect*)));
    connect(m_datapicker, SIGNAL(aspectAboutToBeRemoved(const AbstractAspect*)), this, SLOT(handleAspectAboutToBeRemoved(const AbstractAspect*)));
    connect(m_datapicker, SIGNAL(datapickerItemSelected(int)), m_tabWidget, SLOT(setCurrentIndex(int)) );

    connect(m_tabWidget, SIGNAL(currentChanged(int)), SLOT(tabChanged(int)));
	connect(m_tabWidget, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showTabContextMenu(QPoint)));
}

int DatapickerView::currentIndex() const {
	return m_tabWidget->currentIndex();
}

//##############################################################################
//#########################  Private slots  ####################################
//##############################################################################
void DatapickerView::tabChanged(int index) {
    m_datapicker->setChildSelectedInView(lastSelectedIndex, false);
    m_datapicker->setChildSelectedInView(index, true);
	lastSelectedIndex = index;
}


void DatapickerView::tabMoved(int from, int to) {
	//TODO:test this
    AbstractAspect* aspect = m_datapicker->child<AbstractAspect>(from);
	if (aspect)
        aspect->reparent(m_datapicker, to);
}

/*!
 * Populates the menu \c menu with the spreadsheet and spreadsheet view relevant actions.
 * The menu is used
 *   - as the context menu in DatapickerView
 *   - as the "spreadsheet menu" in the main menu-bar (called form MainWin)
 *   - as a part of the spreadsheet context menu in project explorer
 */
/*void DatapickerView::createContextMenu(QMenu* menu) const {
	Q_ASSERT(menu);

	QAction* firstAction = 0;
	// if we're populating the context menu for the project explorer, then
	//there're already actions available there. Skip the first title-action
	//and insert the action at the beginning of the menu.
	if (menu->actions().size()>1)
		firstAction = menu->actions().at(1);

	menu->insertAction(firstAction, action_add_spreadsheet);
	menu->insertAction(firstAction, action_add_matrix);
	menu->insertSeparator(firstAction);
}*/

void DatapickerView::showTabContextMenu(const QPoint& point) {
	QMenu* menu = 0;
    AbstractAspect* aspect = m_datapicker->child<AbstractAspect>(m_tabWidget->currentIndex());
	Spreadsheet* spreadsheet = dynamic_cast<Spreadsheet*>(aspect);
	if (spreadsheet) {
		menu = spreadsheet->createContextMenu();
	} else {
        Worksheet* worksheet = dynamic_cast<Worksheet*>(aspect);
        if (worksheet)
            menu = worksheet->createContextMenu();
	}

	if (menu)
		menu->exec(m_tabWidget->mapToGlobal(point));
}

void DatapickerView::addWorksheet() {
    Worksheet* worksheet = new Worksheet(0, i18n("Worksheet"));
    m_datapicker->addChild(worksheet);
}

void DatapickerView::addSpreadsheet() {
	Spreadsheet* spreadsheet = new Spreadsheet(0, i18n("Spreadsheet"));
    m_datapicker->addChild(spreadsheet);
}

void DatapickerView::handleAspectAdded(const AbstractAspect* aspect) {
	const AbstractPart* part = dynamic_cast<const AbstractPart*>(aspect);
	if (!part)
		return;

    int index = m_datapicker->indexOfChild<AbstractAspect>(aspect);
	m_tabWidget->insertTab(index, part->view(), aspect->name());
	m_tabWidget->setCurrentIndex(index);
	m_tabWidget->setTabIcon(m_tabWidget->count(), aspect->icon());
}

void DatapickerView::handleAspectAboutToBeRemoved(const AbstractAspect* aspect) {
    int index = m_datapicker->indexOfChild<AbstractAspect>(aspect);
	m_tabWidget->removeTab(index);
}
