/*
    File                 : AbstractPart.cpp
    Project              : LabPlot
    Description          : Base class of Aspects with MDI windows as views.
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2008 Knut Franke <knut.franke@gmx.de>
    SPDX-FileCopyrightText: 2012-2021 Alexander Semke <alexander.semke@web.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "backend/core/AbstractPart.h"
#include "commonfrontend/core/PartMdiView.h"

#include <QMenu>
#include <QStyle>

#include <KLocalizedString>

/**
 * \class AbstractPart
 * \brief Base class of Aspects with MDI windows as views (AspectParts).
 */
AbstractPart::AbstractPart(const QString& name, AspectType type)
	: AbstractAspect(name, type) {
}

AbstractPart::~AbstractPart() {
	if (m_mdiWindow)
		delete m_mdiWindow;
}

/**
 * \fn QWidget *AbstractPart::view() const
 * \brief Construct a primary view on me.
 *
 * The caller receives ownership of the view.
 *
 * This method may be called multiple times during the life time of a Part, or it might not get
 * called at all. Parts must not depend on the existence of a view for their operation.
 */

/**
 * \brief Wrap the view() into a PartMdiView.
 *
 * A new view is only created the first time this method is called;
 * after that, a pointer to the pre-existing view is returned.
 */
PartMdiView* AbstractPart::mdiSubWindow() const {
#ifndef SDK
	if (!m_mdiWindow)
		m_mdiWindow = new PartMdiView(const_cast<AbstractPart*>(this));
#endif
	return m_mdiWindow;

}

bool AbstractPart::hasMdiSubWindow() const {
	return m_mdiWindow;
}

/*!
 * this function is called when PartMdiView, the mdi-subwindow-wrapper of the actual view,
 * is closed (=deleted) in MainWindow. Makes sure that the view also gets deleted.
 */
void AbstractPart::deleteView() const {
	//if the parent is a Workbook or Datapicker, the actual view was already deleted when QTabWidget was deleted.
	//here just set the pointer to 0.
	if (parentAspect()->type() == AspectType::Workbook
		|| parentAspect()->type() == AspectType::Datapicker
		|| parentAspect()->parentAspect()->type() == AspectType::Datapicker) {
		m_partView = nullptr;
		return;
	}

	if (m_partView) {
		delete m_partView;
		m_partView = nullptr;
		m_mdiWindow = nullptr;
	}
}

/**
 * \brief Return AbstractAspect::createContextMenu() plus operations on the primary view.
 */
QMenu* AbstractPart::createContextMenu() {
	QMenu* menu = AbstractAspect::createContextMenu();
	menu->addSeparator();
	auto type = this->type();

	//import actions for spreadsheet and matrix
	if ( (type == AspectType::Spreadsheet || type == AspectType::Matrix)
		&& type != AspectType::LiveDataSource && type != AspectType::MQTTTopic ) {
		QMenu* subMenu = new QMenu(i18n("Import Data"), menu);
		subMenu->addAction(QIcon::fromTheme("document-import"), i18n("From File"), this, &AbstractPart::importFromFileRequested);
		subMenu->addAction(QIcon::fromTheme("document-import"), i18n("From SQL Database"), this, &AbstractPart::importFromSQLDatabaseRequested);
		menu->addMenu(subMenu);
		menu->addSeparator();
	}

	//export/print actions
	menu->addAction(QIcon::fromTheme("document-export-database"), i18n("Export"), this, &AbstractPart::exportRequested);
	menu->addAction(QIcon::fromTheme("document-print"), i18n("Print"), this, &AbstractPart::printRequested);
	menu->addAction(QIcon::fromTheme("document-print-preview"), i18n("Print Preview"), this, &AbstractPart::printPreviewRequested);
	menu->addSeparator();

	//window state related actions
	if (m_mdiWindow) {
		const QStyle* style = m_mdiWindow->style();
		if (m_mdiWindow->windowState() & (Qt::WindowMinimized | Qt::WindowMaximized)) {
			auto* action = menu->addAction(i18n("&Restore"), m_mdiWindow, &QMdiSubWindow::showNormal);
			action->setIcon(style->standardIcon(QStyle::SP_TitleBarNormalButton));
		}

		if (!(m_mdiWindow->windowState() & Qt::WindowMinimized)) {
			auto* action = menu->addAction(i18n("Mi&nimize"), m_mdiWindow, &QMdiSubWindow::showMinimized);
			action->setIcon(style->standardIcon(QStyle::SP_TitleBarMinButton));
		}

		if (!(m_mdiWindow->windowState() & Qt::WindowMaximized)) {
			auto* action = menu->addAction(i18n("Ma&ximize"), m_mdiWindow, &QMdiSubWindow::showMaximized);
			action->setIcon(style->standardIcon(QStyle::SP_TitleBarMaxButton));
		}
	} else {
		//if the mdi window was closed, add the "Show" action.
		//Don't add it for:
		//* children of a workbook, cannot be hidden/minimized
		//* data spreadsheets in datapicker curves
		auto parentType = parentAspect()->type();
		bool disableShow = ((type == AspectType::Spreadsheet || type == AspectType::Matrix) && parentType == AspectType::Workbook) ||
							(type == AspectType::Spreadsheet && parentType == AspectType::DatapickerCurve);
		if (!disableShow)
			menu->addAction(i18n("Show"), this, &AbstractPart::showRequested);
	}

	return menu;
}

bool AbstractPart::isDraggable() const {
	//TODO: moving workbook children doesn't work at the moment, don't allow to move it for now
	if ((type() == AspectType::Spreadsheet || type() == AspectType::Matrix)
		&& parentAspect()->type() == AspectType::Workbook)
		return false;
	else
		return true;
}

QVector<AspectType> AbstractPart::dropableOn() const {
	return QVector<AspectType>{AspectType::Folder, AspectType::Project};
}
