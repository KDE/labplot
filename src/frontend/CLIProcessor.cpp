/*
	File                 : CLIProcessor.cpp
	Project              : LabPlot
	Description          : Process command-line operations
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2026 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "CLIProcessor.h"
#include "backend/core/Project.h"
#include "backend/core/column/Column.h"
#include "backend/datasources/filters/AsciiFilter.h"
#include "backend/spreadsheet/Spreadsheet.h"
#include "backend/worksheet/TextLabel.h"
#include "backend/worksheet/Worksheet.h"
#include "backend/worksheet/plots/cartesian/CartesianPlot.h"
#include "backend/worksheet/plots/cartesian/XYCurve.h"
#include "frontend/PlotTemplateDialog.h"

#include <KLocalizedString>

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>

static QTextStream out(stdout);
static QTextStream err(stderr);

namespace {
Worksheet* findWorksheet(Project* project, const QString& name) {
	const auto worksheets = project->children<Worksheet>(AbstractAspect::ChildIndexFlag::Recursive);
	for (auto* worksheet : worksheets) {
		if (worksheet->name() == name)
			return worksheet;
	}
	return nullptr;
}

QString determineFormat(const QString& outputPath, const QString& formatOption) {
	if (!formatOption.isEmpty()) {
		const QString fmt = formatOption.toLower();
		if (fmt == QLatin1String("pdf") || fmt == QLatin1String("png") || fmt == QLatin1String("svg") || fmt == QLatin1String("eps"))
			return fmt;
		return QString();
	}
	const QString suffix = QFileInfo(outputPath).suffix().toLower();
	if (suffix == QLatin1String("pdf") || suffix == QLatin1String("png") || suffix == QLatin1String("svg") || suffix == QLatin1String("eps"))
		return suffix;
	return QString();
}

int validateFileExists(const QString& filePath, const QString& description) {
	if (!QFile::exists(filePath)) {
		err << "ERROR: " << description << " not found: " << filePath << Qt::endl;
		return CLIExitCode::FileNotFound;
	}
	if (!QFileInfo(filePath).isReadable()) {
		err << "ERROR: " << description << " is not readable: " << filePath << Qt::endl;
		return CLIExitCode::FileNotFound;
	}
	return CLIExitCode::Success;
}

int validateOutputDirectory(const QString& dirPath) {
	QDir dir(dirPath);
	if (!dir.exists()) {
		if (!dir.mkpath(QStringLiteral("."))) {
			err << "ERROR: Could not create output directory: " << dirPath << Qt::endl;
			return CLIExitCode::GeneralError;
		}
		out << "Created output directory: " << dirPath << Qt::endl;
	}
	return CLIExitCode::Success;
}

int loadFile(const QString& filePath, Project** project, const QCommandLineParser& parser) {
	QFileInfo fileInfo(filePath);
	if (!fileInfo.exists()) {
		err << "ERROR: File not found: " << filePath << Qt::endl;
		return CLIExitCode::FileNotFound;
	}

	out << "Loading file: " << filePath << Qt::endl;

	*project = new Project();
	if (!*project) {
		err << "ERROR: Failed to create project." << Qt::endl;
		return CLIExitCode::GeneralError;
	}

	if (Project::isLabPlotProject(filePath)) {
		if (!(*project)->load(filePath)) {
			err << "ERROR: Failed to load project file: " << filePath << Qt::endl;
			delete *project;
			*project = nullptr;
			return CLIExitCode::ProjectLoadFailed;
		}
		out << "Project loaded successfully." << Qt::endl;
		return CLIExitCode::Success;
	}

	// Import as CSV and create a default plot (unless template will be applied)
	out << "Importing data file as CSV..." << Qt::endl;
	auto* spreadsheet = new Spreadsheet(i18n("Imported Data"));
	(*project)->addChild(spreadsheet);
	AsciiFilter filter;
	filter.readDataFromFile(filePath, spreadsheet, AbstractFileFilter::ImportMode::Replace);
	out << "Data imported successfully: " << spreadsheet->rowCount() << " rows, " << spreadsheet->columnCount() << " columns" << Qt::endl;

	// Create a default plot only if no template will be applied
	if (spreadsheet->columnCount() >= 2 && !parser.isSet(CLI_TEMPLATE)) {
		auto* worksheet = new Worksheet(i18n("Plot"));
		(*project)->addChild(worksheet);
		auto* plot = new CartesianPlot(i18n("Plot"));
		plot->setType(CartesianPlot::Type::FourAxes);
		worksheet->addChild(plot);

		// Add curves: first column as X, rest as Y
		Column* xColumn = spreadsheet->column(0);
		for (int i = 1; i < spreadsheet->columnCount(); ++i) {
			auto* curve = new XYCurve(spreadsheet->column(i)->name());
			curve->setXColumn(xColumn);
			curve->setYColumn(spreadsheet->column(i));
			plot->addChild(curve);
		}

		// Apply CLI labels if provided
		if (parser.isSet(CLI_TITLE))
			plot->title()->setText(TextLabel::TextWrapper(parser.value(CLI_TITLE)));
		if (parser.isSet(CLI_XLABEL))
			plot->horizontalAxis()->title()->setText(TextLabel::TextWrapper(parser.value(CLI_XLABEL)));
		if (parser.isSet(CLI_YLABEL))
			plot->verticalAxis()->title()->setText(TextLabel::TextWrapper(parser.value(CLI_YLABEL)));

		out << "Created default plot with " << (spreadsheet->columnCount() - 1) << " curve(s)." << Qt::endl;
	}

	return CLIExitCode::Success;
}

int applyTemplate(Project* project, const QString& templatePath) {
	int exitCode = validateFileExists(templatePath, i18n("Template file"));
	if (exitCode != CLIExitCode::Success)
		return exitCode;

	out << "Applying template: " << templatePath << Qt::endl;

	QString errorMsg;
	auto* plot = PlotTemplateDialog::loadTemplate(templatePath, &errorMsg);
	if (!plot) {
		err << "ERROR: " << errorMsg << Qt::endl;
		return CLIExitCode::TemplateFailed;
	}

	// Add plot to existing or new worksheet
	auto worksheets = project->children<Worksheet>(AbstractAspect::ChildIndexFlag::Recursive);
	Worksheet* worksheet = nullptr;
	if (worksheets.isEmpty()) {
		worksheet = new Worksheet(i18n("Worksheet"));
		project->addChild(worksheet);
	} else {
		worksheet = worksheets.first();
	}

	worksheet->addChild(plot);

	// If there's imported data (spreadsheet), add curves to the template plot
	auto spreadsheets = project->children<Spreadsheet>(AbstractAspect::ChildIndexFlag::Recursive);
	if (!spreadsheets.isEmpty()) {
		auto* spreadsheet = spreadsheets.first();
		if (spreadsheet->columnCount() >= 2) {
			Column* xColumn = spreadsheet->column(0);
			for (int i = 1; i < spreadsheet->columnCount(); ++i) {
				auto* curve = new XYCurve(spreadsheet->column(i)->name());
				curve->setXColumn(xColumn);
				curve->setYColumn(spreadsheet->column(i));
				plot->addChild(curve);
			}
			out << "Added " << (spreadsheet->columnCount() - 1) << " curve(s) to template plot." << Qt::endl;
		}
	}

	out << "Template applied successfully." << Qt::endl;
	return CLIExitCode::Success;
}

int exportWorksheet(Project* project, const QString& worksheetName, const QString& outputPath, const QString& format, int dpi, const QCommandLineParser& parser) {
	out << "Exporting worksheet '" << worksheetName << "' to " << outputPath << " (" << format.toUpper() << ", " << dpi << " DPI)..." << Qt::endl;

	Worksheet* worksheet = findWorksheet(project, worksheetName);
	if (!worksheet) {
		err << "ERROR: Worksheet not found: " << worksheetName << Qt::endl;
		return CLIExitCode::WorksheetNotFound;
	}

	// Apply CLI labels to first plot if provided
	auto plots = worksheet->children<CartesianPlot>();
	if (!plots.isEmpty() && (parser.isSet(CLI_TITLE) || parser.isSet(CLI_XLABEL) || parser.isSet(CLI_YLABEL))) {
		auto* plot = plots.first();
		if (parser.isSet(CLI_TITLE))
			plot->title()->setText(TextLabel::TextWrapper(parser.value(CLI_TITLE)));
		if (parser.isSet(CLI_XLABEL))
			plot->horizontalAxis()->title()->setText(TextLabel::TextWrapper(parser.value(CLI_XLABEL)));
		if (parser.isSet(CLI_YLABEL))
			plot->verticalAxis()->title()->setText(TextLabel::TextWrapper(parser.value(CLI_YLABEL)));
	}

	if (!worksheet->view()) {
		err << "ERROR: Failed to create worksheet view for export." << Qt::endl;
		return CLIExitCode::ExportFailed;
	}

	Worksheet::ExportFormat exportFormat;
	if (format == QLatin1String("pdf"))
		exportFormat = Worksheet::ExportFormat::PDF;
	else if (format == QLatin1String("svg"))
		exportFormat = Worksheet::ExportFormat::SVG;
	else if (format == QLatin1String("png"))
		exportFormat = Worksheet::ExportFormat::PNG;
	else if (format == QLatin1String("eps")) {
		err << "ERROR: EPS format is not directly supported. Use PDF or SVG instead." << Qt::endl;
		return CLIExitCode::InvalidArgument;
	} else {
		err << "ERROR: Unknown export format: " << format << Qt::endl;
		return CLIExitCode::InvalidArgument;
	}

	bool success = worksheet->exportToFile(outputPath, exportFormat, Worksheet::ExportArea::Worksheet, true, dpi);
	if (!success) {
		err << "ERROR: Failed to export worksheet '" << worksheetName << "' to " << outputPath << Qt::endl;
		return CLIExitCode::ExportFailed;
	}

	out << "Successfully exported worksheet '" << worksheetName << "' to " << outputPath << Qt::endl;
	return CLIExitCode::Success;
}

int exportAll(Project* project, const QString& outputDir, const QString& format, int dpi, const QCommandLineParser& parser) {
	out << "Exporting all worksheets to directory: " << outputDir << Qt::endl;

	const auto worksheets = project->children<Worksheet>(AbstractAspect::ChildIndexFlag::Recursive);
	if (worksheets.isEmpty()) {
		err << "ERROR: No worksheets found in project." << Qt::endl;
		return CLIExitCode::WorksheetNotFound;
	}

	int count = 0;
	for (const auto* worksheet : worksheets) {
		const QString outputPath = outputDir + QDir::separator() + worksheet->name() + QStringLiteral(".") + format;
		const int exitCode = exportWorksheet(project, worksheet->name(), outputPath, format, dpi, parser);
		if (exitCode != CLIExitCode::Success)
			return exitCode;
		count++;
	}

	out << "Successfully exported " << count << " worksheet(s)." << Qt::endl;
	return CLIExitCode::Success;
}
} // anonymous namespace

void configureCLI(QCommandLineParser& parser) {
	parser.addOption(QCommandLineOption(CLI_NO_SPLASH, i18n("Disable splash screen (GUI mode only).")));
	parser.addOption(QCommandLineOption(CLI_PRESENTER, i18n("Start in presenter mode (GUI mode only).")));
	parser.addOption(QCommandLineOption(CLI_INPUT, i18n("Input data file (CSV, HDF5, etc.) or project file (.lml)."), i18n("file")));
	parser.addOption(QCommandLineOption(CLI_OUTPUT, i18n("Output file for export (PDF, PNG, SVG, EPS)."), i18n("file")));
	parser.addOption(QCommandLineOption(CLI_FORMAT, i18n("Output format: pdf, png, svg, or eps (auto-detected from output extension if not specified)."), i18n("type")));
	parser.addOption(QCommandLineOption(CLI_DPI, i18n("Output resolution in DPI for raster formats (default: 300)."), i18n("value"), QStringLiteral("300")));
	parser.addOption(QCommandLineOption(CLI_NO_GUI, i18n("Run in headless mode without GUI, exit after operations complete.")));
	parser.addOption(QCommandLineOption(CLI_EXPORT_WORKSHEET, i18n("Export a specific worksheet by name."), i18n("name")));
	parser.addOption(QCommandLineOption(CLI_EXPORT_ALL, i18n("Export all worksheets and spreadsheets in the project.")));
	parser.addOption(QCommandLineOption(CLI_OUTPUT_DIR, i18n("Output directory for multiple exported files (used with --export-all)."), i18n("directory")));
	parser.addOption(QCommandLineOption(CLI_TEMPLATE, i18n("Apply a plot style template file (.labplot_template)."), i18n("file")));
	parser.addOption(QCommandLineOption(CLI_TITLE, i18n("Set the plot title."), i18n("text")));
	parser.addOption(QCommandLineOption(CLI_XLABEL, i18n("Set the X-axis label."), i18n("text")));
	parser.addOption(QCommandLineOption(CLI_YLABEL, i18n("Set the Y-axis label."), i18n("text")));
	parser.addPositionalArgument(QStringLiteral("+[file]"), i18n("Input project file or data file to open."));
}

bool isHeadlessMode(const QCommandLineParser& parser) {
	return parser.isSet(CLI_NO_GUI);
}

QString getInputFile(const QCommandLineParser& parser) {
	if (parser.isSet(CLI_INPUT))
		return parser.value(CLI_INPUT);
	const auto& args = parser.positionalArguments();
	return args.isEmpty() ? QString() : args.first();
}

int processCLI(const QCommandLineParser& parser) {
	const QString inputFile = getInputFile(parser);
	if (inputFile.isEmpty()) {
		err << "ERROR: No input file specified. Use --input <file> or provide a file as positional argument." << Qt::endl;
		return CLIExitCode::InvalidArgument;
	}

	int exitCode = validateFileExists(inputFile, i18n("Input file"));
	if (exitCode != CLIExitCode::Success)
		return exitCode;

	Project* project = nullptr;
	exitCode = loadFile(inputFile, &project, parser);
	if (exitCode != CLIExitCode::Success)
		return exitCode;

	if (!project) {
		err << "ERROR: Failed to create project instance." << Qt::endl;
		return CLIExitCode::GeneralError;
	}

	if (parser.isSet(CLI_TEMPLATE)) {
		exitCode = applyTemplate(project, parser.value(CLI_TEMPLATE));
		if (exitCode != CLIExitCode::Success) {
			delete project;
			return exitCode;
		}
	}

	bool exportPerformed = false;

	if (parser.isSet(CLI_EXPORT_ALL)) {
		QString outputDir = parser.value(CLI_OUTPUT_DIR);
		if (outputDir.isEmpty()) {
			err << "ERROR: --export-all requires --output-dir to specify where to save files." << Qt::endl;
			delete project;
			return CLIExitCode::InvalidArgument;
		}
		exitCode = validateOutputDirectory(outputDir);
		if (exitCode != CLIExitCode::Success) {
			delete project;
			return exitCode;
		}
		QString format = parser.value(CLI_FORMAT);
		if (format.isEmpty())
			format = QStringLiteral("png");
		const int dpi = parser.value(CLI_DPI).toInt();
		exitCode = exportAll(project, outputDir, format, dpi, parser);
		if (exitCode != CLIExitCode::Success) {
			delete project;
			return exitCode;
		}
		exportPerformed = true;
	} else if (parser.isSet(CLI_EXPORT_WORKSHEET)) {
		const QString worksheetName = parser.value(CLI_EXPORT_WORKSHEET);
		const QString outputPath = parser.value(CLI_OUTPUT);
		if (outputPath.isEmpty()) {
			err << "ERROR: --export-worksheet requires --output to specify the output file." << Qt::endl;
			delete project;
			return CLIExitCode::InvalidArgument;
		}
		const QString format = determineFormat(outputPath, parser.value(CLI_FORMAT));
		if (format.isEmpty()) {
			err << "ERROR: Could not determine output format. Specify explicitly with --format." << Qt::endl;
			delete project;
			return CLIExitCode::InvalidArgument;
		}
		const int dpi = parser.value(CLI_DPI).toInt();
		exitCode = exportWorksheet(project, worksheetName, outputPath, format, dpi, parser);
		if (exitCode != CLIExitCode::Success) {
			delete project;
			return exitCode;
		}
		exportPerformed = true;
	} else if (parser.isSet(CLI_OUTPUT)) {
		const QString outputPath = parser.value(CLI_OUTPUT);
		const QString format = determineFormat(outputPath, parser.value(CLI_FORMAT));
		if (format.isEmpty()) {
			err << "ERROR: Could not determine output format. Specify explicitly with --format." << Qt::endl;
			delete project;
			return CLIExitCode::InvalidArgument;
		}
		const int dpi = parser.value(CLI_DPI).toInt();
		const auto worksheets = project->children<Worksheet>(AbstractAspect::ChildIndexFlag::Recursive);
		if (worksheets.isEmpty()) {
			err << "ERROR: No worksheets found in project to export." << Qt::endl;
			delete project;
			return CLIExitCode::WorksheetNotFound;
		}
		const QString worksheetName = worksheets.first()->name();
		exitCode = exportWorksheet(project, worksheetName, outputPath, format, dpi, parser);
		if (exitCode != CLIExitCode::Success) {
			delete project;
			return exitCode;
		}
		exportPerformed = true;
	}

	if (isHeadlessMode(parser) && !exportPerformed) {
		err << "ERROR: Headless mode (--no-gui) requires at least one operation: --export-worksheet, --export-all, or --output." << Qt::endl;
		delete project;
		return CLIExitCode::InvalidArgument;
	}

	delete project;
	return CLIExitCode::Success;
}
