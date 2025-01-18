/*
	File                 : AbstractPart.cpp
	Project              : LabPlot
	Description          : Base class of Aspects with MDI windows as views.
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2008 Knut Franke <knut.franke@gmx.de>
	SPDX-FileCopyrightText: 2012-2025 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "backend/core/AbstractPart.h"
#include "backend/core/Settings.h"
#ifndef SDK
#include "frontend/core/ContentDockWidget.h"
#include <DockManager.h>
#endif
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
#ifndef SDK
	if (m_dockWidget)
		delete m_dockWidget;
#endif
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
#ifndef SDK
/**
 * \brief Wrap the view() into a PartMdiView.
 *
 * A new view is only created the first time this method is called;
 * after that, a pointer to the pre-existing view is returned.
 */
ContentDockWidget* AbstractPart::dockWidget() const {
	if (!m_dockWidget) {
		m_dockWidget = new ContentDockWidget(const_cast<AbstractPart*>(this));
		connect(m_dockWidget, &ads::CDockWidget::closed, [this] {
			const bool deleteOnClose = Settings::readDockPosBehavior() == Settings::DockPosBehavior::AboveLastActive;
			if (deleteOnClose && !m_suppressDeletion) {
				m_dockWidget->dockManager()->removeDockWidget(m_dockWidget);
				m_dockWidget = nullptr;
				deleteView();
			}
		});
	}
	return m_dockWidget;
}
#endif

#ifndef SDK
bool AbstractPart::dockWidgetExists() const {
	return m_dockWidget != nullptr;
}
#endif

void AbstractPart::suppressDeletion(bool suppress) {
	m_suppressDeletion = suppress;
}

#ifndef SDK
bool AbstractPart::hasMdiSubWindow() const {
	return m_dockWidget;
}
#endif

bool AbstractPart::viewCreated() const {
	return m_partView != nullptr;
}

/*!
 * this function is called when PartMdiView, the mdi-subwindow-wrapper of the actual view,
 * is closed (=deleted) in MainWindow. Makes sure that the view also gets deleted.
 */
void AbstractPart::deleteView() const {
	// if the parent is a Workbook or Datapicker, the actual view was already deleted when QTabWidget was deleted.
	// here just set the pointer to 0.
	auto* parent = parentAspect();
	auto type = parent->type();
	if (type == AspectType::Workbook || type == AspectType::Datapicker
		|| (parent->parentAspect() && parent->parentAspect()->type() == AspectType::Datapicker)) {
		m_partView = nullptr;
		return;
	}

	if (m_partView) {
		Q_EMIT viewAboutToBeDeleted();
		delete m_partView;
		m_partView = nullptr;
	}
}

/**
 * \brief Return AbstractAspect::createContextMenu() plus operations on the primary view.
 */
QMenu* AbstractPart::createContextMenu() {
	auto type = this->type();
	QMenu* menu;
	if (type != AspectType::StatisticsSpreadsheet) {
		menu = AbstractAspect::createContextMenu();
		menu->addSeparator();
	} else
		menu = new QMenu();

	// import actions for spreadsheet and matrix
	if ((type == AspectType::Spreadsheet || type == AspectType::Matrix) && type != AspectType::LiveDataSource && type != AspectType::MQTTTopic) {
		QMenu* subMenu = new QMenu(i18n("Import Data"), menu);
		subMenu->addAction(QIcon::fromTheme(QLatin1String("document-import")), i18n("From File..."), this, &AbstractPart::importFromFileRequested);
		subMenu->addAction(QIcon::fromTheme(QLatin1String("document-import")),
						   i18n("From SQL Database..."),
						   this,
						   &AbstractPart::importFromSQLDatabaseRequested);
		menu->addMenu(subMenu);
		menu->addSeparator();
	}

	// export/print actions
	if (type != AspectType::Notebook)
		menu->addAction(QIcon::fromTheme(QLatin1String("document-export-database")), i18n("Export"), this, &AbstractPart::exportRequested);
	menu->addAction(QIcon::fromTheme(QLatin1String("document-print")), i18n("Print"), this, &AbstractPart::printRequested);
	menu->addAction(QIcon::fromTheme(QLatin1String("document-print-preview")), i18n("Print Preview"), this, &AbstractPart::printPreviewRequested);
	menu->addSeparator();

	// window state related actions
#ifndef SDK
	if (m_dockWidget) {
		const QStyle* style = m_dockWidget->style();
		if (!m_dockWidget->isClosed()) {
			auto* action = menu->addAction(i18n("&Close"), [this]() {
				m_dockWidget->toggleView(false);
			});
			action->setIcon(style->standardIcon(QStyle::SP_TitleBarCloseButton));
		} else {
			menu->addAction(i18n("Show"), [this]() {
				m_dockWidget->toggleView(true);
			});
		}
	} else {
		// if the mdi window was closed, add the "Show" action.
		// Don't add it for:
		//* children of a workbook, cannot be hidden/minimized
		//* data spreadsheets in datapicker curves
		auto parentType = parentAspect()->type();
		bool disableShow = ((type == AspectType::Spreadsheet || type == AspectType::Matrix) && parentType == AspectType::Workbook)
			|| (type == AspectType::Spreadsheet && parentType == AspectType::DatapickerCurve);
		if (!disableShow) {
			menu->addAction(i18n("Show"), [this]() {
				m_dockWidget->toggleView(true);
			});
		}
	}
#endif

	return menu;
}

bool AbstractPart::isDraggable() const {
	// TODO: moving workbook children doesn't work at the moment, don't allow to move it for now
	if ((type() == AspectType::Spreadsheet || type() == AspectType::Matrix) && parentAspect()->type() == AspectType::Workbook)
		return false;
	else
		return true;
}

QVector<AspectType> AbstractPart::dropableOn() const {
	return QVector<AspectType>{AspectType::Folder, AspectType::Project};
}
