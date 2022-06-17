/*
	File                 : PlotTemplateDialog.cpp
	Project              : LabPlot
	Description          : dialog to load user-defined plot definitions
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2022 Martin Marmsoler <martin.marmsoler@gmail.com>
	SPDX-FileCopyrightText: 2022 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "PlotTemplateDialog.h"
#include "ui_PlotTemplateDialog.h"

#include "kconfiggroup.h"
#include "klocalizedstring.h"
#include "ksharedconfig.h"

#include "backend/core/Project.h"
#include "backend/lib/XmlStreamReader.h"
#include "backend/worksheet/Worksheet.h"
#include "backend/worksheet/plots/cartesian/XYEquationCurve.h"

#include <QDirIterator>
#include <QFileDialog>
#include <QFileInfo>
#include <QWindow>

#include <KSharedConfig>
#include <KWindowConfig>

namespace {
const QLatin1String lastDirConfigEntry = QLatin1String("LastPlotTemplateDir");

// Copied from BaseDock
struct Lock {
	inline explicit Lock(bool& variable)
		: variable(variable = true) {
	}

	inline ~Lock() {
		variable = false;
	}

private:
	bool& variable;
};
}

const QString PlotTemplateDialog::format = QLatin1String(".labplot_template");

PlotTemplateDialog::PlotTemplateDialog(QWidget* parent)
	: QDialog(parent)
	, ui(new Ui::PlotTemplateDialog) {
	ui->setupUi(this);

	setWindowTitle(i18nc("@title:window", "Plot Templates"));
	setWindowIcon(QIcon::fromTheme("document-import-database"));

	ui->cbTemplateLocation->addItem(i18n("Default"));
	ui->cbTemplateLocation->addItem(i18n("Custom Folder"));
	ui->pbCustomFolder->setIcon(QIcon::fromTheme(QLatin1String("document-save-as-template")));

	KConfigGroup conf(KSharedConfig::openConfig(), QLatin1String("PlotTemplateDialog"));

	m_project = new Project;

	m_worksheet = new Worksheet(QString());
	m_worksheet->setInteractive(false);
	m_worksheet->setUseViewSize(true);
	m_worksheet->setLayoutTopMargin(0.);
	m_worksheet->setLayoutBottomMargin(0.);
	m_worksheet->setLayoutLeftMargin(0.);
	m_worksheet->setLayoutRightMargin(0.);
	m_worksheetView = m_worksheet->view();
	m_project->addChild(m_worksheet);
	ui->lPreview->addWidget(m_worksheetView);
	m_worksheetView->hide();
	mTemplateListModelDefault = new TemplateListModel(defaultTemplateInstallPath(), this);
	mTemplateListModelCustom = new TemplateListModel(conf.readEntry(lastDirConfigEntry, QStandardPaths::writableLocation(QStandardPaths::HomeLocation)), this);
	ui->leCustomFolder->setText(mTemplateListModelCustom->searchPath());

	connect(ui->pbCustomFolder, &QPushButton::pressed, this, &PlotTemplateDialog::chooseTemplateSearchPath);
	connect(ui->leCustomFolder, &QLineEdit::textChanged, this, &PlotTemplateDialog::customTemplatePathChanged);
	connect(ui->lvInstalledTemplates->selectionModel(), &QItemSelectionModel::currentChanged, this, &PlotTemplateDialog::listViewTemplateChanged);
	connect(ui->cbTemplateLocation, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &PlotTemplateDialog::changePreviewSource);
	updateErrorMessage("No template selected.");

	// restore saved settings if available
	create(); // ensure there's a window created
	if (conf.exists()) {
		KWindowConfig::restoreWindowSize(windowHandle(), conf);
		resize(windowHandle()->size()); // workaround for QTBUG-40584
	} else
		resize(QSize(0, 0).expandedTo(minimumSize()));


	if (ui->cbTemplateLocation->currentIndex() == 0) // otherwise no change will be triggered
		changePreviewSource(0); // use default path as initial point
	else
		ui->cbTemplateLocation->setCurrentIndex(0);
}

PlotTemplateDialog::~PlotTemplateDialog() {
	delete ui;
	delete m_project;

	// save current settings
	KConfigGroup conf(KSharedConfig::openConfig(), "PlotTemplateDialog");
	KWindowConfig::saveWindowSize(windowHandle(), conf);
}

void PlotTemplateDialog::customTemplatePathChanged(const QString& path) {
	KConfigGroup conf(KSharedConfig::openConfig(), QLatin1String("PlotTemplateDialog"));
	if (!path.isEmpty())
		conf.writeEntry(lastDirConfigEntry, path);

	mTemplateListModelCustom->setSearchPath(path);
	ui->cbTemplateLocation->setToolTip(path); // custom path index is selected
	auto index = mTemplateListModelCustom->index(0, 0);
	ui->lvInstalledTemplates->setCurrentIndex(index);

	// Because if the modelindex is invalid showPreview() will not be
	// called because currentIndex will not trigger indexChange
	if (!index.isValid())
		showPreview();
}

QString PlotTemplateDialog::defaultTemplateInstallPath() {
	// folder where config files will be stored in object specific sub-folders:
	// Linux    - ~/.local/share/labplot2/plot_templates/
	// Mac      - /Library/Application Support/labplot2
	// Windows  - C:/Users/<USER>/AppData/Roaming/labplot2/plot_templates/
	return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + QLatin1String("/plot_templates/");
}

void PlotTemplateDialog::chooseTemplateSearchPath() {
	//Lock lock(mLoading); // No lock needed
	KConfigGroup conf(KSharedConfig::openConfig(), QLatin1String("PlotTemplateDialog"));
	const QString& dir = conf.readEntry(lastDirConfigEntry, QStandardPaths::writableLocation(QStandardPaths::HomeLocation));

	const QString& path = QFileDialog::getExistingDirectory(nullptr, i18nc("@title:window", "Selec template search path"), dir);

	ui->leCustomFolder->setText(path);
}

CartesianPlot* PlotTemplateDialog::generatePlot() {
	const QString path = templatePath();
	if (path.isEmpty()) {
		updateErrorMessage(i18n("No templates found."));
		return nullptr;
	}
	QFile file(path);
	if (!file.exists()) {
		updateErrorMessage(i18n("File does not exist."));
		return nullptr;
	}

	if (!file.open(QIODevice::OpenModeFlag::ReadOnly)) {
		updateErrorMessage(i18n("Unable to open file: ") + file.errorString());
		return nullptr;
	}

	XmlStreamReader reader(&file);

	while(!(reader.isStartDocument() || reader.atEnd()))
		reader.readNext();

	if (reader.atEnd()) {
		updateErrorMessage(i18n("XML error: No start document token found"));
		return nullptr;
	}

	reader.readNext();
	if (!reader.isDTD()){
		updateErrorMessage(i18n("XML error: No DTD token found"));
		return nullptr;
	}
	reader.readNext();
	if (!reader.isStartElement() || reader.name() != "PlotTemplate"){
		updateErrorMessage(i18n("XML error: No PlotTemplate found"));
		return nullptr;
	}

	bool ok;
	int xmlVersion = reader.readAttributeInt("xmlVersion", &ok);
	if (!ok){
		updateErrorMessage(i18n("XML error: xmlVersion found"));
		return nullptr;
	}
	Project::setXmlVersion(xmlVersion);
	reader.readNext();

	while (!((reader.isStartElement() && reader.name() == "cartesianPlot") || reader.atEnd()))
		reader.readNext();

	if (reader.atEnd()) {
		updateErrorMessage(i18n("XML error: No cartesianPlot found"));
		return nullptr;
	}

	auto* plot = new CartesianPlot(i18n("xy-plot"));
	plot->setIsLoading(true);
	if (!plot->load(&reader, false)) {
		updateErrorMessage(i18n("Unable to load plot template: ") + reader.errorString());
		delete plot;
		return nullptr;
	}
	plot->setIsLoading(false);
	for (auto* child : plot->children<WorksheetElement>(AbstractAspect::ChildIndexFlag::Recursive | AbstractAspect::ChildIndexFlag::IncludeHidden))
		child->setIsLoading(false);


	for (auto* equationCurve : plot->children<XYEquationCurve>())
		static_cast<XYEquationCurve*>(equationCurve)->recalculate();
	plot->retransformAll();
	return plot;
}

void PlotTemplateDialog::showPreview() {
	for (auto* plot : m_worksheet->children<CartesianPlot>())
		m_worksheet->removeChild(plot);

	auto* plot = generatePlot();
	if (plot) {
		m_worksheet->addChild(plot);
		updateErrorMessage(""); // hide error text edit
	}
}

void PlotTemplateDialog::updateErrorMessage(const QString& message) {
	if (message.isEmpty()) {
		ui->teMessage->hide();
		m_worksheetView->show();
		auto* button = ui->buttonBox->button(QDialogButtonBox::StandardButton::Ok);
		assert(button);
		button->setEnabled(true);
		return;
	}

	m_worksheetView->hide();
	ui->teMessage->setText(message);
	ui->teMessage->show();
	auto* button = ui->buttonBox->button(QDialogButtonBox::StandardButton::Ok);
	assert(button);
	button->setEnabled(false);
}

QString PlotTemplateDialog::templatePath() const {
	return ui->lvInstalledTemplates->model()->data(ui->lvInstalledTemplates->currentIndex(), TemplateListModel::Roles::FilePathRole).toString();;
}

void PlotTemplateDialog::listViewTemplateChanged(const QModelIndex& current, const QModelIndex& previous) {
	Q_UNUSED(current);
	Q_UNUSED(previous);
	if (mLoading)
		return;

	Lock lock(mLoading);
	showPreview();
}

void PlotTemplateDialog::changePreviewSource(int row) {
	if (mLoading)
		return;

	TemplateListModel* model = mTemplateListModelDefault;
	if (row == 1)
		model = mTemplateListModelCustom;

	ui->cbTemplateLocation->setToolTip(model->searchPath());

	ui->lCustomFolder->setVisible(row == 1);
	ui->leCustomFolder->setVisible(row == 1);
	ui->pbCustomFolder->setVisible(row == 1);
	ui->lvInstalledTemplates->setModel(model);
	// must be done every time the model changes
	connect(ui->lvInstalledTemplates->selectionModel(), &QItemSelectionModel::currentChanged, this, &PlotTemplateDialog::listViewTemplateChanged);
	ui->lvInstalledTemplates->setCurrentIndex(model->index(0, 0));

	if (!model->index(0, 0).isValid())
		showPreview();
}

//##########################################################################################################
// Listmodel
//##########################################################################################################
TemplateListModel::TemplateListModel(const QString& searchPath, QObject* parent)
	: QAbstractListModel(parent) {
	setSearchPath(searchPath);
}

void TemplateListModel::setSearchPath(const QString& searchPath) {

	beginResetModel();
	mSearchPath = searchPath;
	mFiles.clear();
	QStringList filter("*" + PlotTemplateDialog::format);
	QDirIterator it(searchPath, filter, QDir::AllEntries | QDir::NoSymLinks | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
	QDir sPath(searchPath);
	while (it.hasNext()) {
		QFileInfo f(it.next());
		File file{f.absoluteFilePath(), sPath.relativeFilePath(f.absoluteFilePath()).split(PlotTemplateDialog::format)[0]};
		mFiles << file;
	}
	endResetModel();
}

int TemplateListModel::rowCount(const QModelIndex& parent) const {
	Q_UNUSED(parent);
	return mFiles.count();
}

QVariant TemplateListModel::data(const QModelIndex& index, int role) const {
	const int row = index.row();
	if (!index.isValid() || row > mFiles.count() || row < 0)
		return QVariant();

	switch (role) {
	case FilenameRole: // fall through
	case Qt::ItemDataRole::DisplayRole:
		return mFiles.at(row).filename;
	case FilePathRole:
		return mFiles.at(row).path;
	}

	return QVariant();
}
