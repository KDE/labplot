/*
	File                 : ImportDialog.cc
	Project              : LabPlot
	Description          : import file data dialog
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2008-2025 Alexander Semke <alexander.semke@web.de>
	SPDX-FileCopyrightText: 2008-2015 Stefan Gerlach <stefan.gerlach@uni.kn>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "ImportDialog.h"
#include "backend/core/AspectFactory.h"
#include "backend/core/AspectTreeModel.h"
#include "backend/core/Settings.h"
#include "backend/lib/macros.h"
#include "frontend/MainWin.h"
#include "frontend/widgets/TreeViewComboBox.h"

#include <QDir>
#include <QGroupBox>
#include <QInputDialog>
#include <QLabel>
#include <QMenu>
#include <QProgressBar>
#include <QPushButton>
#include <QStatusBar>
#include <QToolButton>
#include <QVBoxLayout>

#include <KMessageWidget>

/*!
	\class ImportDialog
	\brief Base class for other import dialogs. Provides the "Import to" section of those dialogs.

	\ingroup frontend
 */
ImportDialog::ImportDialog(MainWin* parent)
	: QDialog(parent)
	, vLayout(new QVBoxLayout(this))
	, m_mainWin(parent)
	, m_aspectTreeModel(new AspectTreeModel(parent->project())) {
	setAttribute(Qt::WA_DeleteOnClose);
}

ImportDialog::~ImportDialog() {
	if (m_aspectTreeModel)
		delete m_aspectTreeModel;

	// save the last used import position for file imports, no need to do this for live data source (cbPosition=0)
	if (cbPosition) {
		KConfigGroup conf = Settings::group(QStringLiteral("ImportDialog"));
		conf.writeEntry("Position", cbPosition->currentIndex());
	}
}

/*!
	creates widgets for the frame "Import-To" and sets the current model in the "Add to"-combobox.
 */
void ImportDialog::setModel() {
	// Frame for the "Import To"-Stuff
	frameAddTo = new QGroupBox(this);
	frameAddTo->setTitle(i18n("Import to"));

	auto* label = new QLabel(i18n("Data container:"));
	label->setToolTip(i18n("Data container where the data has to be imported into"));

	auto* grid = new QGridLayout(frameAddTo);
	grid->addWidget(label, 0, 0);

	cbAddTo = new TreeViewComboBox(this);
	cbAddTo->setToolTip(i18n("Data container where the data has to be imported into"));
	cbAddTo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	grid->addWidget(cbAddTo, 0, 1);

	QList<AspectType> list{AspectType::Folder, AspectType::Workbook};
	if (m_importDir) {
		cbAddTo->setTopLevelClasses(list);
		m_aspectTreeModel->setSelectableAspects(list);
	} else {
		list << AspectType::Spreadsheet << AspectType::Matrix;
		cbAddTo->setTopLevelClasses(list);
		list.removeFirst(); // do not allow selection of Folders
		m_aspectTreeModel->setSelectableAspects(list);
	}

	cbAddTo->setModel(m_aspectTreeModel);

	tbNewDataContainer = new QToolButton(frameAddTo);
	tbNewDataContainer->setText(i18n("New"));
	tbNewDataContainer->setIcon(QIcon::fromTheme(QStringLiteral("list-add")));
	tbNewDataContainer->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
	tbNewDataContainer->setToolTip(i18n("Add new data container to the project"));
	grid->addWidget(tbNewDataContainer, 0, 2);

	if (m_importDir) {
		// TODO add later an option to decide if spreadsheets or matrices should be created when importing a directory
	} else {
		// widgets for the import mode/position
		lPosition = new QLabel(i18n("Position:"), frameAddTo);
		grid->addWidget(lPosition, 1, 0);

		cbPosition = new QComboBox(frameAddTo);
		cbPosition->addItem(i18n("Append"));
		cbPosition->addItem(i18n("Prepend"));
		cbPosition->addItem(i18n("Replace"));
		grid->addWidget(cbPosition, 1, 1);

		// restore the last used option
		KConfigGroup conf = Settings::group(QStringLiteral("ImportDialog"));
		cbPosition->setCurrentIndex(conf.readEntry("Position", 0));
		cbPosition->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
	}

	// add the "Import to"-frame to the layout after the first main widget
	vLayout->insertWidget(1, frameAddTo);

	connect(tbNewDataContainer, &QToolButton::clicked, this, &ImportDialog::newDataContainerMenu);
	connect(cbAddTo, &TreeViewComboBox::currentModelIndexChanged, this, &ImportDialog::currentModelIndexChanged);
}

void ImportDialog::setCurrentIndex(const QModelIndex& index) {
	QDEBUG(Q_FUNC_INFO << ", index =" << index);
	cbAddTo->setCurrentModelIndex(index);
	QDEBUG(Q_FUNC_INFO << ", cbAddTo->currentModelIndex() =" << cbAddTo->currentModelIndex());
	checkOkButton();
}

void ImportDialog::currentModelIndexChanged(const QModelIndex& index) {
	auto aspect = static_cast<AbstractAspect*>(index.internalPointer());
	Q_EMIT dataContainerChanged(aspect);
	checkOkButton();
}

void ImportDialog::newDataContainer(QAction* action) {
	const auto type = static_cast<AspectType>(action->data().toInt());
	QString addText, nameText;
	if (type == AspectType::Workbook) {
		addText = i18n("Add a new Workbook");
		nameText = i18n("Workbook name:");
	} else if (type == AspectType::Spreadsheet) {
		addText = i18n("Add a new Spreadsheet");
		nameText = i18n("Spreadsheet name:");
	} else if (type == AspectType::Matrix) {
		addText = i18n("Add a new Matrix");
		nameText = i18n("Matrix name:");
	} else if (type == AspectType::Folder) {
		addText = i18n("Add a new Folder");
		nameText = i18n("Folder name:");
	} else
		return;

	QString name = selectedObject();
	if (name.isEmpty())
		name = action->iconText();

	bool ok;
	auto* dlg = new QInputDialog(this);
	name = dlg->getText(this, addText, nameText, QLineEdit::Normal, name, &ok);
	if (ok) {
		auto* aspect = AspectFactory::createAspect(type, nullptr);
		aspect->setName(name);
		m_mainWin->addAspectToProject(aspect);
		// QDEBUG(Q_FUNC_INFO << ", cbAddTo->setCurrentModelIndex() to " << m_mainWin->model()->modelIndexOfAspect(aspect));
		cbAddTo->setCurrentModelIndex(m_mainWin->model()->modelIndexOfAspect(aspect));
		checkOkButton();

		// select "Replace" since this is the most common case when importing into a newly created container
		if (!m_importDir)
			cbPosition->setCurrentIndex(2);
	}

	delete dlg;
}

void ImportDialog::newDataContainerMenu() {
	// create the menu for new data container first if not available yet
	if (!m_newDataContainerMenu) {
		m_newDataContainerMenu = new QMenu(this);
		if (m_importDir) {
			auto* action = new QAction(QIcon::fromTheme(QStringLiteral("folder-new")), i18n("New Folder"));
			action->setData(static_cast<int>(AspectType::Folder));
			m_newDataContainerMenu->addAction(action);

			action = new QAction(QIcon::fromTheme(QStringLiteral("labplot-workbook-new")), i18n("New Workbook"));
			action->setData(static_cast<int>(AspectType::Workbook));
			m_newDataContainerMenu->addAction(action);
		} else {
			auto* action = new QAction(QIcon::fromTheme(QStringLiteral("labplot-workbook-new")), i18n("New Workbook"));
			action->setData(static_cast<int>(AspectType::Workbook));
			m_newDataContainerMenu->addAction(action);

			action = new QAction(QIcon::fromTheme(QStringLiteral("labplot-spreadsheet-new")), i18n("New Spreadsheet"));
			action->setData(static_cast<int>(AspectType::Spreadsheet));
			m_newDataContainerMenu->addAction(action);

			action = new QAction(QIcon::fromTheme(QStringLiteral("labplot-matrix-new")), i18n("New Matrix"));
			action->setData(static_cast<int>(AspectType::Matrix));
			m_newDataContainerMenu->addAction(action);
		}
		connect(m_newDataContainerMenu, &QMenu::triggered, this, &ImportDialog::newDataContainer);
	}

	m_newDataContainerMenu->exec(tbNewDataContainer->mapToGlobal(tbNewDataContainer->rect().bottomLeft()));
}

void ImportDialog::showErrorMessage(const QString& message) {
	if (message.isEmpty()) {
		if (m_messageWidget && m_messageWidget->isVisible())
			m_messageWidget->close();
	} else {
		if (!m_messageWidget) {
			m_messageWidget = new KMessageWidget(this);
			m_messageWidget->setMessageType(KMessageWidget::Error);
			vLayout->insertWidget(vLayout->count() - 1, m_messageWidget);
		}
		m_messageWidget->setText(message);
		m_messageWidget->animatedShow();
		QDEBUG(message);
	}

	if (message.isEmpty()) {
		okButton->setEnabled(true);
		okButton->setToolTip(i18n("Close the dialog and import the data."));
	} else {
		okButton->setEnabled(false);
		okButton->setToolTip(message);
	}
}

void ImportDialog::accept() {
	if (!m_liveDataSource) {
		bool rc = importTo(m_mainWin->statusBar());
		if (rc) {
			// the import was successful, set the project to Changed and close the dialog
			m_mainWin->project()->setChanged(true);
			QDialog::accept();
		}
	} else
		QDialog::accept();
}
