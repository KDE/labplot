/***************************************************************************
    File                 : ImportProjectDialog.cpp
    Project              : LabPlot
    Description          : import project dialog
    --------------------------------------------------------------------
    Copyright            : (C) 2017-2019 Alexander Semke (alexander.semke@web.de)

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *  This program is free software; you can redistribute it and/or modify   *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation; either version 2 of the License, or      *
 *  (at your option) any later version.                                    *
 *                                                                         *
 *  This program is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the Free Software           *
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor,                    *
 *   Boston, MA  02110-1301  USA                                           *
 *                                                                         *
 ***************************************************************************/

#include "ImportProjectDialog.h"
#include "backend/core/AspectTreeModel.h"
#include "backend/core/Project.h"
#include "backend/datasources/projects/LabPlotProjectParser.h"
#ifdef HAVE_LIBORIGIN
#include "backend/datasources/projects/OriginProjectParser.h"
#endif
#include "kdefrontend/MainWin.h"
#include "commonfrontend/widgets/TreeViewComboBox.h"

#include <QDialogButtonBox>
#include <QDir>
#include <QFileDialog>
#include <QInputDialog>
#include <QProgressBar>
#include <QStatusBar>
#include <QWindow>

#include <KLocalizedString>
#include <KMessageBox>
#include <KSharedConfig>
#include <KWindowConfig>

/*!
    \class ImportProjectDialog
    \brief Dialog for importing project files.

	\ingroup kdefrontend
 */
ImportProjectDialog::ImportProjectDialog(MainWin* parent, ProjectType type) : QDialog(parent),
	m_mainWin(parent),
	m_projectType(type),
	m_aspectTreeModel(new AspectTreeModel(parent->project())) {

	auto* vLayout = new QVBoxLayout(this);

	//main widget
	QWidget* mainWidget = new QWidget(this);
	ui.setupUi(mainWidget);
	ui.chbUnusedObjects->hide();
	vLayout->addWidget(mainWidget);

	ui.tvPreview->setAnimated(true);
	ui.tvPreview->setAlternatingRowColors(true);
	ui.tvPreview->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.tvPreview->setSelectionMode(QAbstractItemView::ExtendedSelection);
	ui.tvPreview->setUniformRowHeights(true);

	ui.bOpen->setIcon( QIcon::fromTheme("document-open") );

	m_cbAddTo = new TreeViewComboBox(ui.gbImportTo);
	m_cbAddTo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	ui.gbImportTo->layout()->addWidget(m_cbAddTo);

	QList<const char*> list;
	list << "Folder";
	m_cbAddTo->setTopLevelClasses(list);
	m_aspectTreeModel->setSelectableAspects(list);
	m_cbAddTo->setModel(m_aspectTreeModel);

	m_bNewFolder = new QPushButton(ui.gbImportTo);
	m_bNewFolder->setIcon(QIcon::fromTheme("list-add"));
	m_bNewFolder->setToolTip(i18n("Add new folder"));
	ui.gbImportTo->layout()->addWidget(m_bNewFolder);

	//dialog buttons
	m_buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
	vLayout->addWidget(m_buttonBox);

	//ok-button is only enabled if some project objects were selected (s.a. ImportProjectDialog::selectionChanged())
	m_buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);

	//Signals/Slots
	connect(ui.leFileName, SIGNAL(textChanged(QString)), SLOT(fileNameChanged(QString)));
	connect(ui.bOpen, SIGNAL(clicked()), this, SLOT (selectFile()));
	connect(m_bNewFolder, SIGNAL(clicked()), this, SLOT(newFolder()));
	connect(ui.chbUnusedObjects, &QCheckBox::stateChanged, this, &ImportProjectDialog::refreshPreview);
	connect(m_buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(m_buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

	QString title;
	switch (m_projectType) {
	case (ProjectLabPlot):
		m_projectParser = new LabPlotProjectParser();
		title = i18nc("@title:window", "Import LabPlot Project");
		break;
	case (ProjectOrigin):
#ifdef HAVE_LIBORIGIN
		m_projectParser = new OriginProjectParser();
		title = i18nc("@title:window", "Import Origin Project");
#endif
		break;
	}

	//dialog title and icon
	setWindowTitle(title);
	setWindowIcon(QIcon::fromTheme("document-import"));

	//restore saved settings if available
	create(); // ensure there's a window created
	KConfigGroup conf(KSharedConfig::openConfig(), "ImportProjectDialog");
	if (conf.exists()) {
		KWindowConfig::restoreWindowSize(windowHandle(), conf);
		resize(windowHandle()->size()); // workaround for QTBUG-40584
	} else
		resize(QSize(300, 0).expandedTo(minimumSize()));

	QString lastImportedFile;
	switch (m_projectType) {
	case (ProjectLabPlot):
		lastImportedFile = QLatin1String("LastImportedLabPlotProject");
		break;
	case (ProjectOrigin):
		lastImportedFile = QLatin1String("LastImportedOriginProject");
		break;
	}

	QApplication::processEvents(QEventLoop::AllEvents, 100);
	ui.leFileName->setText(conf.readEntry(lastImportedFile, ""));
}

ImportProjectDialog::~ImportProjectDialog() {
	//save current settings
	KConfigGroup conf(KSharedConfig::openConfig(), "ImportProjectDialog");
	KWindowConfig::saveWindowSize(windowHandle(), conf);

	QString lastImportedFile;
	switch (m_projectType) {
	case (ProjectLabPlot):
		lastImportedFile = QLatin1String("LastImportedLabPlotProject");
		break;
	case (ProjectOrigin):
		lastImportedFile = QLatin1String("LastImportedOriginProject");
		break;
	}

	conf.writeEntry(lastImportedFile, ui.leFileName->text());
}

void ImportProjectDialog::setCurrentFolder(const Folder* folder) {
	m_cbAddTo->setCurrentModelIndex(m_aspectTreeModel->modelIndexOfAspect(folder));
}

void ImportProjectDialog::importTo(QStatusBar* statusBar) const {
	DEBUG("ImportProjectDialog::importTo()");

	//determine the selected objects, convert the model indexes to string pathes
	const QModelIndexList& indexes = ui.tvPreview->selectionModel()->selectedIndexes();
	QStringList selectedPathes;
	for (int i = 0; i < indexes.size()/4; ++i) {
		QModelIndex index = indexes.at(i*4);
		const auto* aspect = static_cast<const AbstractAspect*>(index.internalPointer());

		//path of the current aspect and the pathes of all aspects it depends on
		selectedPathes << aspect->path();
		QDEBUG(" aspect path: " << aspect->path());
		for (const auto* depAspect : aspect->dependsOn())
			selectedPathes << depAspect->path();
	}
	selectedPathes.removeDuplicates();

	Folder* targetFolder = static_cast<Folder*>(m_cbAddTo->currentModelIndex().internalPointer());

	//check whether the selected pathes already exist in the target folder and warn the user
	const QString& targetFolderPath = targetFolder->path();
	const Project* targetProject = targetFolder->project();
	QStringList targetAllPathes;
	for (const auto* aspect : targetProject->children<AbstractAspect>(AbstractAspect::Recursive)) {
		if (!dynamic_cast<const Folder*>(aspect))
			targetAllPathes << aspect->path();
	}

	QStringList existingPathes;
	for (const auto& path : selectedPathes) {
		const QString& newPath = targetFolderPath + path.right(path.length() - path.indexOf('/'));
		if (targetAllPathes.indexOf(newPath) != -1)
			existingPathes << path;
	}

	QDEBUG("project objects to be imported: " << selectedPathes);
	QDEBUG("all already available project objects: " << targetAllPathes);
	QDEBUG("already available project objects to be overwritten: " << existingPathes);

	if (!existingPathes.isEmpty()) {
		QString msg = i18np("The object listed below already exists in target folder and will be overwritten:",
							"The objects listed below already exist in target folder and will be overwritten:",
							existingPathes.size());
		msg += '\n';
		for (const auto& path : existingPathes)
			msg += '\n' + path.right(path.length() - path.indexOf('/') - 1); //strip away the name of the root folder "Project"
		msg += "\n\n" + i18n("Do you want to proceed?");

		const int rc = KMessageBox::warningYesNo(nullptr, msg, i18n("Override existing objects?"));
		if (rc == KMessageBox::No)
			return;
	}

	//show a progress bar in the status bar
	auto* progressBar = new QProgressBar();
	progressBar->setMinimum(0);
	progressBar->setMaximum(100);

	statusBar->clearMessage();
	statusBar->addWidget(progressBar, 1);

	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
	QApplication::processEvents(QEventLoop::AllEvents, 100);

	//import the selected project objects into the specified folder
	QTime timer;
	timer.start();
	connect(m_projectParser, SIGNAL(completed(int)), progressBar, SLOT(setValue(int)));

#ifdef HAVE_LIBORIGIN
	if (m_projectType == ProjectOrigin && ui.chbUnusedObjects->isVisible() && ui.chbUnusedObjects->isChecked())
		reinterpret_cast<OriginProjectParser*>(m_projectParser)->setImportUnusedObjects(true);
#endif

	m_projectParser->importTo(targetFolder, selectedPathes);
	statusBar->showMessage( i18n("Project data imported in %1 seconds.", (float)timer.elapsed()/1000) );

	QApplication::restoreOverrideCursor();
	statusBar->removeWidget(progressBar);
}

/*!
 * show the content of the project in the tree view
 */
void ImportProjectDialog::refreshPreview() {
	QString project = ui.leFileName->text();
	m_projectParser->setProjectFileName(project);

#ifdef HAVE_LIBORIGIN
	if (m_projectType == ProjectOrigin) {
		auto* originParser = reinterpret_cast<OriginProjectParser*>(m_projectParser);
		if (originParser->hasUnusedObjects())
			ui.chbUnusedObjects->show();
		else
			ui.chbUnusedObjects->hide();

		originParser->setImportUnusedObjects(ui.chbUnusedObjects->isVisible() && ui.chbUnusedObjects->isChecked());
	}
#endif

	ui.tvPreview->setModel(m_projectParser->model());

	connect(ui.tvPreview->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
			this, SLOT(selectionChanged(QItemSelection,QItemSelection)) );

	//show top-level containers only
	if (ui.tvPreview->model()) {
		QModelIndex root = ui.tvPreview->model()->index(0,0);
		showTopLevelOnly(root);
	}

	//extand the tree to show all available top-level objects and adjust the header sizes
	ui.tvPreview->expandAll();
	ui.tvPreview->header()->resizeSections(QHeaderView::ResizeToContents);
}

/*!
	Hides the non-toplevel items of the model used in the tree view.
*/
void ImportProjectDialog::showTopLevelOnly(const QModelIndex& index) {
	int rows = index.model()->rowCount(index);
	for (int i = 0; i < rows; ++i) {
		QModelIndex child = index.child(i, 0);
		showTopLevelOnly(child);
		const auto* aspect = static_cast<const AbstractAspect*>(child.internalPointer());
		ui.tvPreview->setRowHidden(i, index, !isTopLevel(aspect));
	}
}

/*!
	checks whether \c aspect is one of the allowed top level types
*/
bool ImportProjectDialog::isTopLevel(const AbstractAspect* aspect) const {
	foreach (const char* classString, m_projectParser->topLevelClasses()) {
		if (aspect->inherits(classString))
				return true;
	}
	return false;
}

//##############################################################################
//#################################  SLOTS  ####################################
//##############################################################################
void ImportProjectDialog::selectionChanged(const QItemSelection& selected, const QItemSelection& deselected) {
	Q_UNUSED(deselected);

	//determine the dependent objects and select/deselect them too
	const QModelIndexList& indexes = selected.indexes();
	if (indexes.isEmpty())
		return;

	//for the just selected aspect, determine all the objects it depends on and select them, too
	//TODO: we need a better "selection", maybe with tri-state check boxes in the tree view
	const auto* aspect = static_cast<const AbstractAspect*>(indexes.at(0).internalPointer());
	const QVector<AbstractAspect*> aspects = aspect->dependsOn();
	const auto* model = reinterpret_cast<AspectTreeModel*>(ui.tvPreview->model());
	for (const auto* aspect : aspects) {
		QModelIndex index = model->modelIndexOfAspect(aspect, 0);
		ui.tvPreview->selectionModel()->select(index, QItemSelectionModel::Select | QItemSelectionModel::Rows);
	}


	//Ok-button is only enabled if some project objects were selected
	bool enable = (selected.indexes().size() != 0);
	m_buttonBox->button(QDialogButtonBox::Ok)->setEnabled(enable);
	if (enable)
		m_buttonBox->button(QDialogButtonBox::Ok)->setToolTip(i18n("Close the dialog and import the selected objects."));
	else
		m_buttonBox->button(QDialogButtonBox::Ok)->setToolTip(i18n("Select object(s) to be imported."));
}

/*!
	opens a file dialog and lets the user select the project file.
*/
void ImportProjectDialog::selectFile() {
	KConfigGroup conf(KSharedConfig::openConfig(), "ImportProjectDialog");

	QString title;
	QString lastDir;
	QString supportedFormats;
	QString lastDirConfEntryName;
	switch (m_projectType) {
	case (ProjectLabPlot):
		title = i18nc("@title:window", "Open LabPlot Project");
		lastDirConfEntryName = QLatin1String("LastImportLabPlotProjectDir");
		supportedFormats = i18n("LabPlot Projects (%1)", Project::supportedExtensions());
		break;
	case (ProjectOrigin):
#ifdef HAVE_LIBORIGIN
		title = i18nc("@title:window", "Open Origin Project");
		lastDirConfEntryName = QLatin1String("LastImportOriginProjecttDir");
		supportedFormats = i18n("Origin Projects (%1)", OriginProjectParser::supportedExtensions());
#endif
		break;
	}

	lastDir = conf.readEntry(lastDirConfEntryName, "");
	QString path = QFileDialog::getOpenFileName(this, title, lastDir, supportedFormats);
	if (path.isEmpty())
		return; //cancel was clicked in the file-dialog

	int pos = path.lastIndexOf(QDir::separator());
	if (pos != -1) {
		QString newDir = path.left(pos);
		if (newDir != lastDir)
			conf.writeEntry(lastDirConfEntryName, newDir);
	}

	ui.leFileName->setText(path);
	refreshPreview();
}

void ImportProjectDialog::fileNameChanged(const QString& name) {
	QString fileName = name;
#ifndef HAVE_WINDOWS
	// make relative path
	if ( !fileName.isEmpty() && fileName.at(0) != QDir::separator())
		fileName = QDir::homePath() + QDir::separator() + fileName;
#endif

	bool fileExists = QFile::exists(fileName);
	if (fileExists)
		ui.leFileName->setStyleSheet(QString());
	else
		ui.leFileName->setStyleSheet("QLineEdit{background:red;}");

	if (!fileExists) {
		//file doesn't exist -> delete the content preview that is still potentially
		//available from the previously selected file
		ui.tvPreview->setModel(nullptr);
		m_buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
		return;
	}

	refreshPreview();
}

void ImportProjectDialog::newFolder() {
	QString path = ui.leFileName->text();
	QString name = path.right( path.length()-path.lastIndexOf(QDir::separator())-1 );

	bool ok;
	QInputDialog* dlg = new QInputDialog(this);
	name = dlg->getText(this, i18n("Add new folder"), i18n("Folder name:"), QLineEdit::Normal, name, &ok);
	if (ok) {
		auto* folder = new Folder(name);
		m_mainWin->addAspectToProject(folder);
		m_cbAddTo->setCurrentModelIndex(m_mainWin->model()->modelIndexOfAspect(folder));
	}

	delete dlg;
}
