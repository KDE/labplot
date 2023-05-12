/*
	File                 : ImportProjectDialog.cpp
	Project              : LabPlot
	Description          : import project dialog
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2017-2021 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "ImportProjectDialog.h"
#include "backend/core/AspectTreeModel.h"
#include "backend/core/Project.h"
#include "backend/datasources/projects/LabPlotProjectParser.h"
#ifdef HAVE_LIBORIGIN
#include "backend/datasources/projects/OriginProjectParser.h"
#endif
#include "commonfrontend/widgets/TreeViewComboBox.h"
#include "kdefrontend/GuiTools.h"
#include "kdefrontend/MainWin.h"

#include <KLocalizedString>
#include <KMessageBox>
#include <KSharedConfig>
#include <KUrlComboBox>
#include <KWindowConfig>
#include <kcoreaddons_version.h>

#include <QDialogButtonBox>
#include <QDir>
#include <QElapsedTimer>
#include <QFileDialog>
#include <QInputDialog>
#include <QProgressBar>
#include <QStatusBar>
#include <QWindow>

/*!
	\class ImportProjectDialog
	\brief Dialog for importing project files.

	\ingroup kdefrontend
 */
ImportProjectDialog::ImportProjectDialog(MainWin* parent, ProjectType type)
	: QDialog(parent)
	, m_mainWin(parent)
	, m_projectType(type)
	, m_aspectTreeModel(new AspectTreeModel(parent->project())) {
	auto* vLayout = new QVBoxLayout(this);

	// main widget
	auto* mainWidget = new QWidget(this);
	ui.setupUi(mainWidget);
	ui.chbUnusedObjects->hide();

	m_cbFileName = new KUrlComboBox(KUrlComboBox::Mode::Files, this);
	m_cbFileName->setMaxItems(7);
	auto* l = dynamic_cast<QHBoxLayout*>(ui.gbProject->layout());
	if (l)
		l->insertWidget(1, m_cbFileName);

	vLayout->addWidget(mainWidget);

	ui.tvPreview->setAnimated(true);
	ui.tvPreview->setAlternatingRowColors(true);
	ui.tvPreview->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.tvPreview->setSelectionMode(QAbstractItemView::ExtendedSelection);
	ui.tvPreview->setUniformRowHeights(true);

	ui.bOpen->setIcon(QIcon::fromTheme(QStringLiteral("document-open")));

	m_cbAddTo = new TreeViewComboBox(ui.gbImportTo);
	m_cbAddTo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	ui.gbImportTo->layout()->addWidget(m_cbAddTo);

	QList<AspectType> list{AspectType::Folder};
	m_cbAddTo->setTopLevelClasses(list);
	m_aspectTreeModel->setSelectableAspects(list);
	m_cbAddTo->setModel(m_aspectTreeModel);

	m_bNewFolder = new QPushButton(ui.gbImportTo);
	m_bNewFolder->setIcon(QIcon::fromTheme(QStringLiteral("list-add")));
	m_bNewFolder->setToolTip(i18n("Add new folder"));
	ui.gbImportTo->layout()->addWidget(m_bNewFolder);

	// dialog buttons
	m_buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
	vLayout->addWidget(m_buttonBox);

	// ok-button is only enabled if some project objects were selected (s.a. ImportProjectDialog::selectionChanged())
	m_buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);

	// Signals/Slots
	connect(m_cbFileName, &KUrlComboBox::urlActivated, this, [=](const QUrl& url) {
		fileNameChanged(url.path());
	});
	connect(ui.bOpen, &QPushButton::clicked, this, &ImportProjectDialog::selectFile);
	connect(m_bNewFolder, &QPushButton::clicked, this, &ImportProjectDialog::newFolder);
	connect(ui.chbUnusedObjects, &QCheckBox::toggled, this, &ImportProjectDialog::refreshPreview);
	connect(m_buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
	connect(m_buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

	QString title;
	switch (m_projectType) {
	case ProjectType::LabPlot:
		m_projectParser = new LabPlotProjectParser();
		title = i18nc("@title:window", "Import LabPlot Project");
		break;
	case ProjectType::Origin:
#ifdef HAVE_LIBORIGIN
		m_projectParser = new OriginProjectParser();
		title = i18nc("@title:window", "Import Origin Project");
#endif
		break;
	}

	// dialog title and icon
	setWindowTitle(title);
	setWindowIcon(QIcon::fromTheme(QStringLiteral("document-import")));

	//"What's this?" texts
	QString info = i18n("Specify the file where the project content has to be imported from.");
	m_cbFileName->setWhatsThis(info);

	info = i18n(
		"Select one or several objects to be imported into the current project.\n"
		"Note, all children of the selected objects as well as all the dependent objects will be automatically selected.\n"
		"To import the whole project, select the top-level project node.");
	ui.tvPreview->setWhatsThis(info);

	info = i18n("Specify the target folder in the current project where the selected objects have to be imported into.");
	m_cbAddTo->setWhatsThis(info);

	// restore saved settings if available
	create(); // ensure there's a window created
	KConfigGroup conf(KSharedConfig::openConfig(), "ImportProjectDialog");
	if (conf.exists()) {
		KWindowConfig::restoreWindowSize(windowHandle(), conf);
		resize(windowHandle()->size()); // workaround for QTBUG-40584
	} else
		resize(QSize(300, 0).expandedTo(minimumSize()));

	QString file;
	QString files;
	switch (m_projectType) {
	case ProjectType::LabPlot:
		file = QStringLiteral("LastImportedLabPlotProject");
		files = QStringLiteral("LastImportedLabPlotProjects");
		break;
	case ProjectType::Origin:
		file = QStringLiteral("LastImportedOriginProject");
		files = QStringLiteral("LastImportedOriginProjects");
		break;
	}

	QApplication::processEvents(QEventLoop::AllEvents, 100);
	m_cbFileName->setUrl(QUrl(conf.readEntry(file, "")));
	QStringList urls = m_cbFileName->urls();
	urls.append(conf.readXdgListEntry(files));
	m_cbFileName->setUrls(urls);
	fileNameChanged(m_cbFileName->currentText());
}

ImportProjectDialog::~ImportProjectDialog() {
	// save current settings
	KConfigGroup conf(KSharedConfig::openConfig(), "ImportProjectDialog");
	KWindowConfig::saveWindowSize(windowHandle(), conf);

	QString file;
	QString files;
	switch (m_projectType) {
	case ProjectType::LabPlot:
		file = QStringLiteral("LastImportedLabPlotProject");
		files = QStringLiteral("LastImportedLabPlotProjects");
		break;
	case ProjectType::Origin:
		file = QStringLiteral("LastImportedOriginProject");
		files = QStringLiteral("LastImportedOriginProjects");
		break;
	}

	conf.writeEntry(file, m_cbFileName->currentText());
	conf.writeXdgListEntry(files, m_cbFileName->urls());
}

void ImportProjectDialog::setCurrentFolder(const Folder* folder) {
	m_cbAddTo->setCurrentModelIndex(m_aspectTreeModel->modelIndexOfAspect(folder));
}

void ImportProjectDialog::importTo(QStatusBar* statusBar) const {
	DEBUG("ImportProjectDialog::importTo()");

	// determine the selected objects, convert the model indexes to string pathes
	const QModelIndexList& indexes = ui.tvPreview->selectionModel()->selectedIndexes();
	QStringList selectedPathes;
	for (int i = 0; i < indexes.size() / 4; ++i) {
		QModelIndex index = indexes.at(i * 4);
		const auto* aspect = static_cast<const AbstractAspect*>(index.internalPointer());

		// path of the current aspect and the pathes of all aspects it depends on
		selectedPathes << aspect->path();
		QDEBUG(" aspect path: " << aspect->path());
		for (const auto* depAspect : aspect->dependsOn())
			selectedPathes << depAspect->path();
	}
	selectedPathes.removeDuplicates();

	Folder* targetFolder = static_cast<Folder*>(m_cbAddTo->currentModelIndex().internalPointer());

	// check whether the selected pathes already exist in the target folder and warn the user
	const QString& targetFolderPath = targetFolder->path();
	const Project* targetProject = targetFolder->project();
	QStringList targetAllPathes;
	for (const auto* aspect : targetProject->children<AbstractAspect>(AbstractAspect::ChildIndexFlag::Recursive)) {
		if (aspect && !dynamic_cast<const Folder*>(aspect))
			targetAllPathes << aspect->path();
	}

	QStringList existingPathes;
	for (const auto& path : selectedPathes) {
		const QString& newPath = targetFolderPath + path.right(path.length() - path.indexOf(QLatin1Char('/')));
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
		msg += QLatin1Char('\n');
		for (const auto& path : existingPathes)
			msg += QLatin1Char('\n') + path.right(path.length() - path.indexOf(QLatin1Char('/')) - 1); // strip away the name of the root folder "Project"
		msg += QStringLiteral("\n\n") + i18n("Do you want to proceed?");

#if KCOREADDONS_VERSION >= QT_VERSION_CHECK(5, 100, 0)
		auto status =
			KMessageBox::warningTwoActions(nullptr, msg, i18n("Override existing objects?"), KStandardGuiItem::overwrite(), KStandardGuiItem::cancel());
#else
		auto status = KMessageBox::warningYesNo(nullptr, msg, i18n("Override existing objects?"));
#endif
		if (status == KMessageBox::No)
			return;
	}

	// show a progress bar in the status bar
	auto* progressBar = new QProgressBar();
	progressBar->setMinimum(0);
	progressBar->setMaximum(100);

	statusBar->clearMessage();
	statusBar->addWidget(progressBar, 1);

	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
	QApplication::processEvents(QEventLoop::AllEvents, 100);

	// import the selected project objects into the specified folder
	QElapsedTimer timer;
	timer.start();
	connect(m_projectParser, &ProjectParser::completed, progressBar, &QProgressBar::setValue);

#ifdef HAVE_LIBORIGIN
	if (m_projectType == ProjectType::Origin && ui.chbUnusedObjects->isVisible() && ui.chbUnusedObjects->isChecked())
		reinterpret_cast<OriginProjectParser*>(m_projectParser)->setImportUnusedObjects(true);
#endif

	m_projectParser->importTo(targetFolder, selectedPathes);
	statusBar->showMessage(i18n("Project data imported in %1 seconds.", (float)timer.elapsed() / 1000));

	QApplication::restoreOverrideCursor();
	statusBar->removeWidget(progressBar);
}

/*!
 * show the content of the project in the tree view
 */
void ImportProjectDialog::refreshPreview() {
	const QString& project = m_cbFileName->currentText();
	m_projectParser->setProjectFileName(project);

#ifdef HAVE_LIBORIGIN
	if (m_projectType == ProjectType::Origin) {
		auto* originParser = reinterpret_cast<OriginProjectParser*>(m_projectParser);
		if (originParser->hasUnusedObjects())
			ui.chbUnusedObjects->show();
		else
			ui.chbUnusedObjects->hide();

		originParser->setImportUnusedObjects(ui.chbUnusedObjects->isVisible() && ui.chbUnusedObjects->isChecked());
	}
#endif

	ui.tvPreview->setModel(m_projectParser->model());

	connect(ui.tvPreview->selectionModel(), &QItemSelectionModel::selectionChanged, this, &ImportProjectDialog::selectionChanged);

	// show top-level containers only
	if (ui.tvPreview->model()) {
		QModelIndex root = ui.tvPreview->model()->index(0, 0);
		showTopLevelOnly(root);
	}

	// select the first top-level node and
	// expand the tree to show all available top-level objects and adjust the header sizes
	ui.tvPreview->setCurrentIndex(ui.tvPreview->model()->index(0, 0));
	ui.tvPreview->expandAll();
	ui.tvPreview->header()->resizeSections(QHeaderView::ResizeToContents);
}

/*!
	Hides the non-toplevel items of the model used in the tree view.
*/
void ImportProjectDialog::showTopLevelOnly(const QModelIndex& index) {
	int rows = index.model()->rowCount(index);
	for (int i = 0; i < rows; ++i) {
		QModelIndex child = index.model()->index(i, 0, index);
		showTopLevelOnly(child);
		const auto* aspect = static_cast<const AbstractAspect*>(child.internalPointer());
		ui.tvPreview->setRowHidden(i, index, !isTopLevel(aspect));
	}
}

/*!
	checks whether \c aspect is one of the allowed top level types
*/
bool ImportProjectDialog::isTopLevel(const AbstractAspect* aspect) const {
	foreach (AspectType type, m_projectParser->topLevelClasses()) {
		if (aspect->inherits(type))
			return true;
	}
	return false;
}

// ##############################################################################
// #################################  SLOTS  ####################################
// ##############################################################################
void ImportProjectDialog::selectionChanged(const QItemSelection& selected, const QItemSelection& /*deselected*/) {
	// determine the dependent objects and select/deselect them too
	const QModelIndexList& indexes = selected.indexes();
	if (indexes.isEmpty())
		return;

	// for the just selected aspect, determine all the objects it depends on and select them, too
	// TODO: we need a better "selection", maybe with tri-state check boxes in the tree view
	const auto* aspect = static_cast<const AbstractAspect*>(indexes.at(0).internalPointer());
	const QVector<AbstractAspect*> aspects = aspect->dependsOn();

	const auto* model = reinterpret_cast<AspectTreeModel*>(ui.tvPreview->model());
	for (const auto* aspect : aspects) {
		QModelIndex index = model->modelIndexOfAspect(aspect, 0);
		ui.tvPreview->selectionModel()->select(index, QItemSelectionModel::Select | QItemSelectionModel::Rows);
	}

	// Ok-button is only enabled if some project objects were selected
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
	case ProjectType::LabPlot:
		title = i18nc("@title:window", "Open LabPlot Project");
		lastDirConfEntryName = QStringLiteral("LastImportLabPlotProjectDir");
		supportedFormats = i18n("LabPlot Projects (%1)", Project::supportedExtensions());
		break;
	case ProjectType::Origin:
#ifdef HAVE_LIBORIGIN
		title = i18nc("@title:window", "Open Origin Project");
		lastDirConfEntryName = QStringLiteral("LastImportOriginProjecttDir");
		supportedFormats = i18n("Origin Projects (%1)", OriginProjectParser::supportedExtensions());
#endif
		break;
	}

	lastDir = conf.readEntry(lastDirConfEntryName, "");
	QString path = QFileDialog::getOpenFileName(this, title, lastDir, supportedFormats);
	if (path.isEmpty())
		return; // cancel was clicked in the file-dialog

	int pos = path.lastIndexOf(QLatin1Char('/'));
	if (pos != -1) {
		QString newDir = path.left(pos);
		if (newDir != lastDir)
			conf.writeEntry(lastDirConfEntryName, newDir);
	}

	QStringList urls = m_cbFileName->urls();
	urls.insert(0, QUrl::fromLocalFile(path).url());
	m_cbFileName->setUrls(urls);
	m_cbFileName->setCurrentText(urls.first());
	fileNameChanged(path); // why do I have to call this function separately

	refreshPreview();
}

void ImportProjectDialog::fileNameChanged(const QString& name) {
	QString fileName{name};

	// make relative path
#ifdef HAVE_WINDOWS
	if (!fileName.isEmpty() && fileName.at(1) != QLatin1Char(':'))
#else
	if (!fileName.isEmpty() && fileName.at(0) != QLatin1Char('/'))
#endif
		fileName = QDir::homePath() + QStringLiteral("/") + fileName;

	bool fileExists = QFile::exists(fileName);
	if (!fileExists) {
		// file doesn't exist -> delete the content preview that is still potentially
		// available from the previously selected file
		ui.tvPreview->setModel(nullptr);
		m_buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
		return;
	}

	refreshPreview();
}

void ImportProjectDialog::newFolder() {
	const QString& path = m_cbFileName->currentText();
	QString name = path.right(path.length() - path.lastIndexOf(QLatin1Char('/')) - 1);

	bool ok;
	auto* dlg = new QInputDialog(this);
	name = dlg->getText(this, i18n("Add new folder"), i18n("Folder name:"), QLineEdit::Normal, name, &ok);
	if (ok) {
		auto* folder = new Folder(name);
		m_mainWin->addAspectToProject(folder);
		m_cbAddTo->setCurrentModelIndex(m_mainWin->model()->modelIndexOfAspect(folder));
	}

	delete dlg;
}
