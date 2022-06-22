#include "TemplateChooserDialog.h"
#include "ui_TemplateChooserDialog.h"

#include "klocalizedstring.h"
#include "kconfiggroup.h"
#include "ksharedconfig.h"

#include "backend/core/Project.h"
#include "backend/worksheet/Worksheet.h"
#include "backend/lib/XmlStreamReader.h"

#include <QFileDialog>
#include <QDirIterator>
#include <QFileInfo>

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

const QString TemplateChooserDialog::format = QLatin1String(".labplot_template");

TemplateChooserDialog::TemplateChooserDialog(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::TemplateChooserDialog)
{
	ui->setupUi(this);

	m_project = new Project;

	m_worksheet = new Worksheet(QString());
	m_worksheet->setUseViewSize(true);
	m_worksheet->setLayoutTopMargin(0.);
	m_worksheet->setLayoutBottomMargin(0.);
	m_worksheet->setLayoutLeftMargin(0.);
	m_worksheet->setLayoutRightMargin(0.);
	m_worksheetView = m_worksheet->view();
	m_project->addChild(m_worksheet);
	ui->lPreview->addWidget(m_worksheetView);
	m_worksheetView->hide();
	mTemplateListModel = new TemplateListModel(defaultTemplateInstallPath(), this);
	ui->lvInstalledTemplates->setModel(mTemplateListModel);

	connect(ui->pbBrowse, &QPushButton::pressed, this, &TemplateChooserDialog::chooseTemplate);
	connect(ui->leTemplatePath, &QLineEdit::textChanged, this, &TemplateChooserDialog::customTemplatePathChanged);
	connect(ui->lvInstalledTemplates->selectionModel(), &QItemSelectionModel::currentRowChanged, this, &TemplateChooserDialog::listViewTemplateChanged);
	connect(ui->cbCustomTemplatePreview, &QCheckBox::clicked, this, &TemplateChooserDialog::changePreviewSource);
	updateErrorMessage("No template selected.");
}

TemplateChooserDialog::~TemplateChooserDialog()
{
	delete ui;
	delete m_project;
}

void TemplateChooserDialog::customTemplatePathChanged(const QString& filename) {
	if (mLoading)
		return;

	Lock lock(mLoading);

	ui->cbCustomTemplatePreview->setChecked(true);
	mCurrentTemplateFilePath = filename;
	showPreview();
}

QString TemplateChooserDialog::defaultTemplateInstallPath() {
	//folder where config files will be stored in object specific sub-folders:
	//Linux    - ~/.local/share/labplot2/plot_templates/
	//Mac      - //TODO
	//Windows  - C:/Users/<USER>/AppData/Roaming/labplot2/plot_templates/
	return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + QLatin1String("/plot_templates/");
}

void TemplateChooserDialog::chooseTemplate() {
	Lock lock(mLoading);
	KConfigGroup conf(KSharedConfig::openConfig(), QLatin1String("TemplateChooserDialog"));
	const QString& dir = conf.readEntry(lastDirConfigEntry, QStandardPaths::writableLocation(QStandardPaths::HomeLocation));

	const QString& path = QFileDialog::getOpenFileName(nullptr, i18nc("@title:window", "Select Template File"), dir, i18n("Labplot Plot Templates (*%1)", format));
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

CartesianPlot* TemplateChooserDialog::generatePlot() {
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
	while (!(reader.isStartDocument() || reader.atEnd()))
		reader.readNext();

	if (!(reader.atEnd())) {
		if (!reader.skipToNextTag()) {
			updateErrorMessage(i18n("XML error."));
			return nullptr;
		}
		if (!reader.isStartElement()) {
			updateErrorMessage(i18n("No XML Startelement found."));
			return nullptr;
		}

		if (reader.name() != "cartesianPlot") {
			updateErrorMessage(i18n("XML: no cartesian plot found."));
			return nullptr;
		}
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
	plot->retransformAll();
	return plot;
}

void TemplateChooserDialog::showPreview() {
	for (auto* plot: m_worksheet->children<CartesianPlot>())
		m_worksheet->removeChild(plot);

	auto* plot = generatePlot();
	if (plot) {
		m_worksheet->addChild(plot);
		updateErrorMessage(""); // hide error text edit
	}
}

void TemplateChooserDialog::updateErrorMessage(const QString& message) {
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

QString TemplateChooserDialog::templatePath() const {
	return mCurrentTemplateFilePath;
}

void TemplateChooserDialog::listViewTemplateChanged(const QModelIndex &current, const QModelIndex &previous) {
	Q_UNUSED(previous);
	if (mLoading)
		return;

	Lock lock(mLoading);
	mCurrentTemplateFilePath = mTemplateListModel->data(current, TemplateListModel::Roles::FilePathRole).toString();
	ui->cbCustomTemplatePreview->setChecked(false);
	showPreview();
}

void TemplateChooserDialog::changePreviewSource(bool custom) {
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
TemplateListModel::TemplateListModel(const QString& searchPath, QObject* parent): QAbstractListModel(parent) {

	QStringList filter("*" + TemplateChooserDialog::format);
	QDirIterator it(searchPath, filter, QDir::AllEntries | QDir::NoSymLinks | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
	while (it.hasNext()) {
		QFileInfo f(it.next());
		File file {
			f.absoluteFilePath(),
			f.fileName().split(TemplateChooserDialog::format)[0]
		};
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
