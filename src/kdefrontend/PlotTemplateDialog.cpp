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

	m_project = new Project;

	m_worksheet = new Worksheet(QString());
	m_worksheet->setUseViewSize(true);
	m_worksheet->setLayoutTopMargin(0.);
	m_worksheet->setLayoutBottomMargin(0.);
	m_worksheet->setLayoutLeftMargin(0.);
	m_worksheet->setLayoutRightMargin(0.);
	m_worksheetView = m_worksheet->view();
	m_worksheetView->setContextMenuPolicy(Qt::ContextMenuPolicy::NoContextMenu); // preventing that a context menu will be created to add elements to the plot
	m_project->addChild(m_worksheet);
	ui->lPreview->addWidget(m_worksheetView);
	m_worksheetView->hide();
	mTemplateListModel = new TemplateListModel(defaultTemplateInstallPath(), this);
	ui->lvInstalledTemplates->setModel(mTemplateListModel);

	ui->lInstalledTemplates->setToolTip(i18n("Path: %1", defaultTemplateInstallPath()));

	connect(ui->pbBrowse, &QPushButton::pressed, this, &PlotTemplateDialog::chooseTemplate);
	connect(ui->leTemplatePath, &QLineEdit::textChanged, this, &PlotTemplateDialog::customTemplatePathChanged);
	connect(ui->lvInstalledTemplates->selectionModel(), &QItemSelectionModel::currentRowChanged, this, &PlotTemplateDialog::listViewTemplateChanged);
	connect(ui->cbCustomTemplatePreview, &QCheckBox::clicked, this, &PlotTemplateDialog::changePreviewSource);
	updateErrorMessage("No template selected.");

	// restore saved settings if available
	KConfigGroup conf(KSharedConfig::openConfig(), "PlotTemplateDialog");
	create(); // ensure there's a window created
	if (conf.exists()) {
		KWindowConfig::restoreWindowSize(windowHandle(), conf);
		resize(windowHandle()->size()); // workaround for QTBUG-40584
	} else
		resize(QSize(0, 0).expandedTo(minimumSize()));
}

PlotTemplateDialog::~PlotTemplateDialog() {
	delete ui;
	delete m_project;

	// save current settings
	KConfigGroup conf(KSharedConfig::openConfig(), "PlotTemplateDialog");
	KWindowConfig::saveWindowSize(windowHandle(), conf);
}

void PlotTemplateDialog::customTemplatePathChanged(const QString& filename) {
	if (mLoading)
		return;

	Lock lock(mLoading);

	ui->cbCustomTemplatePreview->setChecked(true);
	mCurrentTemplateFilePath = filename;
	showPreview();
}

QString PlotTemplateDialog::defaultTemplateInstallPath() {
	// folder where config files will be stored in object specific sub-folders:
	// Linux    - ~/.local/share/labplot2/plot_templates/
	// Mac      - //TODO
	// Windows  - C:/Users/<USER>/AppData/Roaming/labplot2/plot_templates/
	return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + QLatin1String("/plot_templates/");
}

void PlotTemplateDialog::chooseTemplate() {
	Lock lock(mLoading);
	KConfigGroup conf(KSharedConfig::openConfig(), QLatin1String("PlotTemplateDialog"));
	const QString& dir = conf.readEntry(lastDirConfigEntry, QStandardPaths::writableLocation(QStandardPaths::HomeLocation));

	const QString& path =
		QFileDialog::getOpenFileName(nullptr, i18nc("@title:window", "Select Template File"), dir, i18n("Plot Templates (*%1)", format));
	ui->leTemplatePath->setText(path);

	if (!path.isEmpty()) {
		int pos = path.lastIndexOf(QLatin1String("/"));
		if (pos != -1) {
			QString newDir = path.left(pos);
			if (newDir != dir)
				conf.writeEntry(lastDirConfigEntry, newDir);
		}
	}

	ui->cbCustomTemplatePreview->setChecked(true);
}

CartesianPlot* PlotTemplateDialog::generatePlot() {
	QFile file(mCurrentTemplateFilePath);
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
	return mCurrentTemplateFilePath;
}

void PlotTemplateDialog::listViewTemplateChanged(const QModelIndex& current, const QModelIndex& previous) {
	Q_UNUSED(previous);
	if (mLoading)
		return;

	Lock lock(mLoading);
	mCurrentTemplateFilePath = mTemplateListModel->data(current, TemplateListModel::Roles::FilePathRole).toString();
	ui->cbCustomTemplatePreview->setChecked(false);
	showPreview();
}

void PlotTemplateDialog::changePreviewSource(bool custom) {
	if (mLoading)
		return;

	if (custom)
		mCurrentTemplateFilePath = ui->leTemplatePath->text();
	else {
		const QModelIndex& current = ui->lvInstalledTemplates->selectionModel()->currentIndex();
		mCurrentTemplateFilePath = mTemplateListModel->data(current, TemplateListModel::Roles::FilePathRole).toString();
	}

	showPreview();
}

//##########################################################################################################
// Listmodel
//##########################################################################################################
TemplateListModel::TemplateListModel(const QString& searchPath, QObject* parent)
	: QAbstractListModel(parent) {
	QStringList filter("*" + PlotTemplateDialog::format);
	QDirIterator it(searchPath, filter, QDir::AllEntries | QDir::NoSymLinks | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
	while (it.hasNext()) {
		QFileInfo f(it.next());
		File file{f.absoluteFilePath(), f.fileName().split(PlotTemplateDialog::format)[0]};
		mFiles << file;
	}
}

int TemplateListModel::rowCount(const QModelIndex& parent) const {
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
