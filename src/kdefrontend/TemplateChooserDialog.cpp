#include "TemplateChooserDialog.h"
#include "ui_TemplateChooserDialog.h"

#include "klocalizedstring.h"
#include "kconfiggroup.h"
#include "ksharedconfig.h"

#include "backend/core/Project.h"
#include "backend/worksheet/Worksheet.h"
#include "backend/lib/XmlStreamReader.h"

#include <QFileDialog>

namespace {
	const QLatin1String lastDirConfigEntry = QLatin1String("LastPlotTemplateDir");
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

	connect(ui->pbBrowse, &QPushButton::pressed, this, &TemplateChooserDialog::chooseTemplate);
	connect(ui->leTemplatePath, &QLineEdit::textChanged, this, &TemplateChooserDialog::showPreview);

	updateErrorMessage("No template selected.");
}

void TemplateChooserDialog::chooseTemplate() {
	KConfigGroup conf(KSharedConfig::openConfig(), QLatin1String("TemplateChooserDialog"));
	const QString& dir = conf.readEntry(lastDirConfigEntry, QString());

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
}

void TemplateChooserDialog::showPreview() {
	for (auto* plot: m_worksheet->children<CartesianPlot>())
		m_worksheet->removeChild(plot);

	QFile file(ui->leTemplatePath->text());
	if (!file.exists()) {
		updateErrorMessage(i18n("File does not exist."));
		return;
	}

	if (!file.open(QIODevice::OpenModeFlag::ReadOnly)) {
		updateErrorMessage(i18n("Unable to open file: ") + file.errorString());
		return;
	}

	XmlStreamReader reader(&file);
	while (!(reader.isStartDocument() || reader.atEnd()))
		reader.readNext();

	if (!(reader.atEnd())) {
		if (!reader.skipToNextTag()) {
			updateErrorMessage(i18n("XML error."));
			return;
		}
		if (!reader.isStartElement()) {
			updateErrorMessage(i18n("No XML Startelement found."));
			return;
		}

		if (reader.name() != "cartesianPlot") {
			updateErrorMessage(i18n("XML: no cartesian plot found."));
			return;
		}
	}


	auto* plot = new CartesianPlot(i18n("xy-plot"));
	if (!plot->load(&reader, false)) {
		updateErrorMessage(i18n("Unable to load plot template: ") + reader.errorString());
		delete plot;
		return;
	}
	for (auto* child : plot->children<WorksheetElement>(AbstractAspect::ChildIndexFlag::Recursive | AbstractAspect::ChildIndexFlag::IncludeHidden))
		child->setIsLoading(false);
	plot->retransformAll();
	m_worksheet->addChild(plot);

	updateErrorMessage(""); // hide error text edit
}

void TemplateChooserDialog::updateErrorMessage(const QString& message) {
	if (message.isEmpty()) {
		ui->teMessage->hide();
		m_worksheetView->show();
		return;
	}

	m_worksheetView->hide();
	ui->teMessage->setText(message);
	ui->teMessage->show();
}

TemplateChooserDialog::~TemplateChooserDialog()
{
	delete ui;
	delete m_project;
}

QString TemplateChooserDialog::templatePath() const {
	return ui->leTemplatePath->text();
}
