/*
	File                 : FunctionValuesDialog.cpp
	Project              : LabPlot
	Description          : Dialog for generating values from a mathematical function
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2014-2018 Alexander Semke <alexander.semke@web.de>
	SPDX-FileCopyrightText: 2020-2025 Stefan Gerlach <stefan.gerlach@uni.kn>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "FunctionValuesDialog.h"
#include "backend/core/AspectTreeModel.h"
#include "backend/core/Project.h"
#include "backend/core/Settings.h"
#include "backend/core/column/Column.h"
#include "backend/gsl/ExpressionParser.h"
#include "backend/lib/macros.h"
#include "backend/spreadsheet/Spreadsheet.h"
#include "frontend/widgets/ConstantsWidget.h"
#include "frontend/widgets/FunctionsWidget.h"
#include "frontend/widgets/TextPreview.h"
#include "frontend/widgets/TreeViewComboBox.h"

#include <QDialogButtonBox>
#include <QMenu>
#include <QPushButton>
#include <QWidgetAction>
#include <QWindow>

#include <KFileWidget>
#include <KLineEdit>
#include <KLocalizedString>
#include <KUrlComboBox>
#include <KWindowConfig>

/*!
	\class FunctionValuesDialog
	\brief Dialog for generating values from a mathematical function.

	\ingroup frontend
 */
FunctionValuesDialog::FunctionValuesDialog(Spreadsheet* s, QWidget* parent)
	: QDialog(parent)
	, m_spreadsheet(s) {
	Q_ASSERT(s);
	setWindowTitle(i18nc("@title:window", "Function Values"));

	ui.setupUi(this);
	setAttribute(Qt::WA_DeleteOnClose);
	ui.tbConstants->setIcon(QIcon::fromTheme(QStringLiteral("format-text-symbol")));
	ui.tbFunctions->setIcon(QIcon::fromTheme(QStringLiteral("preferences-desktop-font")));

	ui.teEquation->setMaximumHeight(QLineEdit().sizeHint().height() * 2);
	ui.teEquation->setFocus();

// needed for buggy compiler
#if __cplusplus < 201103L
	m_aspectTreeModel = std::auto_ptr<AspectTreeModel>(new AspectTreeModel(m_spreadsheet->project()));
#else
	m_aspectTreeModel = std::unique_ptr<AspectTreeModel>(new AspectTreeModel(m_spreadsheet->project()));
#endif
	m_aspectTreeModel->setSelectableAspects({AspectType::Column});
	m_aspectTreeModel->enableNumericColumnsOnly(true);

	ui.bAddVariable->setIcon(QIcon::fromTheme(QStringLiteral("list-add")));
	ui.bAddVariable->setToolTip(i18n("Add new variable"));

	ui.chkAutoUpdate->setToolTip(i18n("Automatically update the calculated values in the target column on changes in the variable columns"));
	ui.chkAutoResize->setToolTip(i18n("Automatically resize the target column to fit the size of the variable columns"));

	auto* btnBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
	ui.verticalLayout->addWidget(btnBox);
	m_okButton = btnBox->button(QDialogButtonBox::Ok);

	connect(btnBox, &QDialogButtonBox::accepted, this, &FunctionValuesDialog::accept);
	connect(btnBox, &QDialogButtonBox::rejected, this, &FunctionValuesDialog::reject);
	m_okButton->setText(i18n("&Generate"));

	connect(ui.bAddVariable, &QPushButton::pressed, this, &FunctionValuesDialog::addVariable);
	connect(ui.teEquation, &ExpressionTextEdit::expressionChanged, this, &FunctionValuesDialog::checkValues);
	connect(ui.tbConstants, &QToolButton::clicked, this, &FunctionValuesDialog::showConstants);
	connect(ui.tbFunctions, &QToolButton::clicked, this, &FunctionValuesDialog::showFunctions);
	connect(ui.pbLoadFunction, &QPushButton::clicked, this, &FunctionValuesDialog::loadFunction);
	connect(ui.pbSaveFunction, &QPushButton::clicked, this, &FunctionValuesDialog::saveFunction);
	connect(m_okButton, &QPushButton::clicked, this, &FunctionValuesDialog::generate);

	// restore saved settings if available
	create(); // ensure there's a window created
	KConfigGroup conf = Settings::group(QStringLiteral("FunctionValuesDialog"));
	if (conf.exists()) {
		KWindowConfig::restoreWindowSize(windowHandle(), conf);
		resize(windowHandle()->size()); // workaround for QTBUG-40584
	} else
		resize(QSize(300, 0).expandedTo(minimumSize()));
}

FunctionValuesDialog::~FunctionValuesDialog() {
	KConfigGroup conf = Settings::group(QStringLiteral("FunctionValuesDialog"));
	KWindowConfig::saveWindowSize(windowHandle(), conf);
}

void FunctionValuesDialog::setColumns(const QVector<Column*>& columns) {
	m_columns = columns;
	const Column* firstColumn{m_columns.first()};

	// formula expression
	ui.teEquation->setPlainText(firstColumn->formula());
	// variables
	const auto& formulaData = firstColumn->formulaData();
	if (formulaData.isEmpty()) {
		// A formula without any column variable is also possible, for example when just using the rownumber: "i / 1000"
		// This is a workaround, because right now there is no way to determine which variable is used in the formula and which not
		// it makes no sense to add variables if they are not used (preventing cyclic dependency) Gitlab #1037

		// no formula was used for this column -> add the first variable "x"
		// addVariable();
		// m_variableLineEdits[0]->setText(QStringLiteral("x"));
	} else { // formula and variables are available
		// add all available variables and select the corresponding columns
		const auto& cols = m_spreadsheet->project()->children<Column>(AbstractAspect::ChildIndexFlag::Recursive);
		for (int i = 0; i < formulaData.size(); ++i) {
			addVariable();
			m_variableLineEdits[i]->setText(formulaData.at(i).variableName());

			bool found = false;
			for (const auto* col : cols) {
				if (col != formulaData.at(i).column())
					continue;

				const auto* column = dynamic_cast<const AbstractColumn*>(col);
				if (column)
					m_variableDataColumns[i]->setCurrentModelIndex(m_aspectTreeModel->modelIndexOfAspect(column));
				else
					m_variableDataColumns[i]->setCurrentModelIndex(QModelIndex());

				m_variableDataColumns[i]->setInvalid(false);

				found = true;
				break;
			}

			// for the current variable name no column exists anymore (was deleted)
			//->highlight the combobox red
			if (!found) {
				m_variableDataColumns[i]->setCurrentModelIndex(QModelIndex());
				m_variableDataColumns[i]->setInvalid(
					true,
					i18n("The column \"%1\"\nis not available anymore. It will be automatically used once it is created again.",
						 formulaData.at(i).columnName()));
				m_variableDataColumns[i]->setText(formulaData.at(i).columnName().split(QLatin1Char('/')).last());
			}
		}
	}

	// auto update
	// Enable if linking is turned on, so the user has to explicit disable recalculation, so it cannot be forgotten
	ui.chkAutoUpdate->setChecked(firstColumn->formulaAutoUpdate() || m_spreadsheet->linking());

	// auto-resize
	if (!m_spreadsheet->linking())
		ui.chkAutoResize->setChecked(firstColumn->formulaAutoResize());
	else {
		// linking is active, deactive this option since the size of the target spreadsheet is controlled by the linked spreadsheet
		ui.chkAutoResize->setChecked(false);
		ui.chkAutoResize->setEnabled(false);
		ui.chkAutoResize->setToolTip(i18n("Spreadsheet linking is active. The size of the target spreadsheet is controlled by the linked spreadsheet."));
	}

	checkValues();
}

bool FunctionValuesDialog::validVariableName(QLineEdit* le) {
	bool isValid = false;
	if (ExpressionParser::getInstance()->constants().indexOf(le->text()) != -1) {
		SET_WARNING_STYLE(le)
		le->setToolTip(i18n("Provided variable name is already reserved for a name of a constant. Please use another name."));
	} else if (ExpressionParser::getInstance()->functions().indexOf(le->text()) != -1) {
		SET_WARNING_STYLE(le)
		le->setToolTip(i18n("Provided variable name is already reserved for a name of a function. Please use another name."));
	} else if (le->text().compare(QLatin1String("i")) == 0) {
		SET_WARNING_STYLE(le)
		le->setToolTip(i18n("The variable name 'i' is reserved for the index of the column row."));
	} else if (le->text().contains(QRegularExpression(QLatin1String("^[0-9]|[^a-zA-Z0-9_]")))) {
		SET_WARNING_STYLE(le)
		le->setToolTip(i18n("Provided variable name starts with a digit or contains special character."));
	} else {
		le->setStyleSheet(QString());
		le->setToolTip(QString());
		isValid = true;
	}
	return isValid;
}

void FunctionValuesDialog::checkValues() {
	if (ui.teEquation->toPlainText().simplified().isEmpty()) {
		m_okButton->setToolTip(i18n("Empty formula expression"));
		m_okButton->setEnabled(false);
		return;
	}

	// check whether the formula syntax is correct
	if (!ui.teEquation->isValid()) {
		m_okButton->setToolTip(i18n("Incorrect formula syntax: ") + ui.teEquation->errorMessage());
		m_okButton->setEnabled(false);
		return;
	}

	// check if expression uses variables
	if (ui.teEquation->expressionUsesVariables()) {
		// check the variables
		for (int i = 0; i < m_variableDataColumns.size(); ++i) {
			const auto& varName = m_variableLineEdits.at(i)->text();

			// ignore empty
			if (varName.isEmpty())
				continue;

			// check whether a valid column was provided for the variable
			auto* cb = m_variableDataColumns.at(i);
			auto* aspect = static_cast<AbstractAspect*>(cb->currentModelIndex().internalPointer());
			if (!aspect) {
				m_okButton->setToolTip(i18n("Select a valid column"));
				m_okButton->setEnabled(false);
				return;
			}

			// check whether the variable name is correct
			if (!validVariableName(m_variableLineEdits.at(i))) {
				m_okButton->setToolTip(i18n("Variable name can contain letters, digits and '_' only and should start with a letter"));
				m_okButton->setEnabled(false);
				return;
			}
		}
	}

	m_okButton->setToolTip(i18n("Generate function values"));
	m_okButton->setEnabled(true);
}

// see also XYFitCurveDock::loadFunction()
void FunctionValuesDialog::loadFunction() {
	//easy alternative: const QString& fileName = QFileDialog::getOpenFileName(this, i18nc("@title:window", "Select file to load function definition"), dir, filter);

	QDialog dialog;
	dialog.setWindowTitle(i18n("Select file to load function definition"));
	auto* layout = new QVBoxLayout(&dialog);

	// use last open dir from MainWin (project dir)
	KConfigGroup mainGroup = Settings::group(QStringLiteral("MainWin"));
	const QString& dir = mainGroup.readEntry("LastOpenDir", "");

	//using KFileWidget to add custom widgets
	auto* fileWidget = new KFileWidget(QUrl(dir), &dialog);
	fileWidget->setOperationMode(KFileWidget::Opening);
	fileWidget->setMode(KFile::File);

	// preview
	auto* preview = new TextPreview();
	fileWidget->setPreviewWidget(preview);

	auto filterList = QList<KFileFilter>();
	filterList << KFileFilter(i18n("LabPlot Function Definition"), {QLatin1String("*.lfd"), QLatin1String("*.LFD")}, {});
	fileWidget->setFilters(filterList);

	fileWidget->okButton()->show();
	fileWidget->okButton()->setEnabled(false);
	fileWidget->cancelButton()->show();
	QObject::connect(fileWidget->okButton(), &QPushButton::clicked, &dialog, &QDialog::accept);
	QObject::connect(fileWidget, &KFileWidget::selectionChanged, &dialog, [=]() {
		QString fileName = fileWidget->locationEdit()->currentText();
		auto currentDir = fileWidget->baseUrl().toLocalFile();
		fileName.prepend(currentDir);
		if (QFile::exists(fileName))
			fileWidget->okButton()->setEnabled(true);
	});
	QObject::connect(fileWidget->cancelButton(), &QPushButton::clicked, &dialog, &QDialog::reject);
	layout->addWidget(fileWidget);

	if (dialog.exec() == QDialog::Accepted) {
		QString fileName = fileWidget->locationEdit()->currentText();
		auto currentDir = fileWidget->baseUrl().toLocalFile();
		fileName.prepend(currentDir);

		//load config from file if accepted
		QDEBUG(Q_FUNC_INFO << ", load function from file" << fileName)

		KConfig config(fileName);
		auto general = config.group(QLatin1String("General"));
		ui.teEquation->setPlainText(general.readEntry("Function", ""));

		auto description = general.readEntry("Description", "");
		auto comment = general.readEntry("Comment", "");
		QDEBUG(Q_FUNC_INFO << ", description:" << description)
		QDEBUG(Q_FUNC_INFO << ", comment:" << comment)
		if (!description.isEmpty())
			ui.teEquation->viewport()->setToolTip(description);
		if (!comment.isEmpty())
			ui.teEquation->viewport()->setWhatsThis(comment);
	}
}

// see also XYFitCurveDock::saveFunction()
void FunctionValuesDialog::saveFunction() {
	QDialog dialog;
	dialog.setWindowTitle(i18n("Select file to save function definition"));
	auto* layout = new QVBoxLayout(&dialog);

	// use last open dir from MainWin (project dir)
	KConfigGroup mainGroup = Settings::group(QStringLiteral("MainWin"));
	const QString& dir = mainGroup.readEntry("LastOpenDir", "");

	//using KFileWidget to add custom widgets
	auto* fileWidget = new KFileWidget(QUrl(dir), &dialog);
	fileWidget->setOperationMode(KFileWidget::Saving);
	fileWidget->setMode(KFile::File);
	// preview
	auto* preview = new TextPreview();
	fileWidget->setPreviewWidget(preview);

	auto filterList = QList<KFileFilter>();
	filterList << KFileFilter(i18n("LabPlot Function Definition"), {QLatin1String("*.lfd"), QLatin1String("*.LFD")}, {});
	fileWidget->setFilters(filterList);

	fileWidget->okButton()->show();
	fileWidget->okButton()->setEnabled(false);
	fileWidget->cancelButton()->show();
	QObject::connect(fileWidget->okButton(), &QPushButton::clicked, &dialog, &QDialog::accept);
	QObject::connect(fileWidget->cancelButton(), &QPushButton::clicked, &dialog, &QDialog::reject);
	layout->addWidget(fileWidget);

	// custom widgets
	auto* lDescription = new QLabel(i18n("Description:"));
	auto* leDescription = new KLineEdit(ui.teEquation->viewport()->toolTip());
	auto* lComment = new QLabel(i18n("Comment:"));
	auto* leComment = new KLineEdit(ui.teEquation->viewport()->whatsThis());

	// update description and comment when selection changes
	connect(fileWidget, &KFileWidget::fileHighlighted, this, [=]() {
		QString fileName = fileWidget->locationEdit()->currentText();
		auto currentDir = fileWidget->baseUrl().toLocalFile();
		fileName.prepend(currentDir);
		QDEBUG(Q_FUNC_INFO << ", file selected:" << fileName)
		if (QFile::exists(fileName)) {
			KConfig config(fileName);
			auto group = config.group(QLatin1String("General"));
			const QString& description = group.readEntry("Description", "");
			const QString& comment = group.readEntry("Comment", "");
			if (!description.isEmpty())
				leDescription->setText(description);
			if (!comment.isEmpty())
				leComment->setText(comment);
		}
	});

	auto* grid = new QGridLayout;
	grid->addWidget(lDescription, 0, 0);
	grid->addWidget(leDescription, 0, 1);
	grid->addWidget(lComment, 1, 0);
	grid->addWidget(leComment, 1, 1);
	layout->addLayout(grid);

	dialog.adjustSize();
	if (dialog.exec() == QDialog::Accepted) {
		fileWidget->slotOk();

		QString fileName = fileWidget->selectedFile();
		if (fileName.isEmpty()) {	// if entered directly and not selected (also happens when selected!)
			// DEBUG(Q_FUNC_INFO << ", no file selected")
			fileName = fileWidget->locationEdit()->currentText();
			auto* cbExtension = fileWidget->findChild<QCheckBox*>();
			if (cbExtension) {
				bool checked = cbExtension->isChecked();
				if (checked && ! (fileName.endsWith(QLatin1String(".lfd")) || fileName.endsWith(QLatin1String(".LFD"))))
							fileName.append(QLatin1String(".lfd"));
			}
			// add current folder
			auto currentDir = fileWidget->baseUrl().toLocalFile();
			fileName.prepend(currentDir);
		}
		// save current model (with description and comment)
		// FORMAT: LFD - LabPlot Function Definition
		KConfig config(fileName);	// selected lfd file
		auto group = config.group(QLatin1String("General"));
		auto description = leDescription->text();
		auto comment = leComment->text();
		group.writeEntry("Function", ui.teEquation->toPlainText());	// model function
		group.writeEntry("Description", description);
		group.writeEntry("Comment", comment);
		config.sync();
		QDEBUG(Q_FUNC_INFO << ", saved function to" << fileName)

		// set description and comment in Dock (even when empty)
		ui.teEquation->viewport()->setToolTip(description);
		ui.teEquation->viewport()->setWhatsThis(comment);
	}
}

void FunctionValuesDialog::showConstants() {
	QMenu menu;
	ConstantsWidget constants(&menu);
	connect(&constants, &ConstantsWidget::constantSelected, this, &FunctionValuesDialog::insertConstant);
	connect(&constants, &ConstantsWidget::constantSelected, &menu, &QMenu::close);
	connect(&constants, &ConstantsWidget::canceled, &menu, &QMenu::close);

	auto* widgetAction{new QWidgetAction(this)};
	widgetAction->setDefaultWidget(&constants);
	menu.addAction(widgetAction);

	QPoint pos(-menu.sizeHint().width() + ui.tbConstants->width(), -menu.sizeHint().height());
	menu.exec(ui.tbConstants->mapToGlobal(pos));
}

void FunctionValuesDialog::showFunctions() {
	QMenu menu;
	FunctionsWidget functions(&menu);
	connect(&functions, &FunctionsWidget::functionSelected, this, &FunctionValuesDialog::insertFunction);
	connect(&functions, &FunctionsWidget::functionSelected, &menu, &QMenu::close);
	connect(&functions, &FunctionsWidget::canceled, &menu, &QMenu::close);

	auto* widgetAction{new QWidgetAction(this)};
	widgetAction->setDefaultWidget(&functions);
	menu.addAction(widgetAction);

	QPoint pos(-menu.sizeHint().width() + ui.tbFunctions->width(), -menu.sizeHint().height());
	menu.exec(ui.tbFunctions->mapToGlobal(pos));
}

void FunctionValuesDialog::insertFunction(const QString& functionName) const {
	ui.teEquation->insertPlainText(functionName + ExpressionParser::functionArgumentString(functionName, XYEquationCurve::EquationType::Cartesian));
}

void FunctionValuesDialog::insertConstant(const QString& constantsName) const {
	ui.teEquation->insertPlainText(constantsName);
}

void FunctionValuesDialog::addVariable() {
	auto* layout{ui.gridLayoutVariables};
	auto row{m_variableLineEdits.size()};
	// text field for the variable name
	auto* le{new QLineEdit};
	le->setToolTip(i18n("Variable name can contain letters, digits and '_' only and should start with a letter"));
	auto* validator = new QRegularExpressionValidator(QRegularExpression(QLatin1String("[a-zA-Z][a-zA-Z0-9_]*")), le);
	le->setValidator(validator);
	le->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::MinimumExpanding));
	connect(le, &QLineEdit::textChanged, this, &FunctionValuesDialog::variableNameChanged);
	layout->addWidget(le, row, 0, 1, 1);
	m_variableLineEdits << le;
	auto* l{new QLabel(QStringLiteral("="))};
	layout->addWidget(l, row, 1, 1, 1);
	m_variableLabels << l;

	// combo box for the data column
	auto* cb{new TreeViewComboBox()};
	cb->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred));
	connect(cb, &TreeViewComboBox::currentModelIndexChanged, this, &FunctionValuesDialog::variableColumnChanged);
	layout->addWidget(cb, row, 2, 1, 1);
	m_variableDataColumns << cb;

	cb->setTopLevelClasses(TreeViewComboBox::plotColumnTopLevelClasses());
	cb->setModel(m_aspectTreeModel.get());

	// don't allow to select columns to be calculated as variable columns (avoid circular dependencies)
	QList<const AbstractAspect*> aspects;
	for (auto* col : m_columns)
		aspects << col;
	cb->setHiddenAspects(aspects);

	// for the variable column select the first non-selected column in the spreadsheet
	for (auto* col : m_spreadsheet->children<Column>()) {
		if (m_columns.indexOf(col) == -1) {
			cb->setCurrentModelIndex(m_aspectTreeModel->modelIndexOfAspect(col));
			break;
		}
	}

	// move the add-button to the next row
	layout->removeWidget(ui.bAddVariable);
	layout->addWidget(ui.bAddVariable, row + 1, 3, 1, 1);

	// add delete-button for the just added variable
	if (row != 0) {
		auto* b{new QToolButton()};
		b->setIcon(QIcon::fromTheme(QStringLiteral("list-remove")));
		b->setToolTip(i18n("Delete variable"));
		layout->addWidget(b, row, 3, 1, 1);
		m_variableDeleteButtons << b;
		connect(b, &QToolButton::pressed, this, &FunctionValuesDialog::deleteVariable);
	}

	ui.lVariable->setText(i18n("Variables:"));

	// TODO: adjust the tab-ordering after new widgets were added
}

void FunctionValuesDialog::deleteVariable() {
	QObject* ob{QObject::sender()};
	const auto index{m_variableDeleteButtons.indexOf(qobject_cast<QToolButton*>(ob))};

	delete m_variableLineEdits.takeAt(index + 1);
	delete m_variableLabels.takeAt(index + 1);
	delete m_variableDataColumns.takeAt(index + 1);
	delete m_variableDeleteButtons.takeAt(index);

	variableNameChanged();
	checkValues();

	// adjust the layout
	resize(QSize(width(), 0).expandedTo(minimumSize()));

	m_variableLineEdits.size() > 1 ? ui.lVariable->setText(i18n("Variables:")) : ui.lVariable->setText(i18n("Variable:"));

	// TODO: adjust the tab-ordering after some widgets were deleted
}

void FunctionValuesDialog::variableNameChanged() {
	QStringList vars;
	QString argText;
	for (auto* varName : m_variableLineEdits) {
		QString name = varName->text().simplified();
		if (!name.isEmpty()) {
			vars << name;

			if (argText.isEmpty())
				argText += name;
			else
				argText += QStringLiteral(", ") + name;
		}
	}

	QString funText = QStringLiteral("f = ");
	if (!argText.isEmpty())
		funText = QStringLiteral("f(") + argText + QStringLiteral(") = ");

	ui.lFunction->setText(funText);
	ui.teEquation->setVariables(vars);
	checkValues();
}

void FunctionValuesDialog::variableColumnChanged(const QModelIndex& index) {
	// combobox was potentially red-highlighted because of a missing column
	// remove the highlighting when we have a valid selection now
	auto* aspect{static_cast<AbstractAspect*>(index.internalPointer())};
	if (aspect) {
		auto* cb{dynamic_cast<TreeViewComboBox*>(QObject::sender())};
		if (cb)
			cb->setStyleSheet(QString());
	}

	checkValues();
}

void FunctionValuesDialog::generate() {
	Q_ASSERT(m_spreadsheet);

	WAIT_CURSOR;
	m_spreadsheet->beginMacro(i18np("%1: fill column with function values", "%1: fill columns with function values", m_spreadsheet->name(), m_columns.size()));

	// determine variable names and data vectors of the specified columns
	QStringList variableNames;
	QVector<Column*> variableColumns;
	for (int i = 0; i < m_variableLineEdits.size(); ++i) {
		variableNames << m_variableLineEdits.at(i)->text().simplified();

		auto* aspect{static_cast<AbstractAspect*>(m_variableDataColumns.at(i)->currentModelIndex().internalPointer())};
		if (aspect) {
			auto* column{dynamic_cast<Column*>(aspect)};
			if (column)
				variableColumns << column;
		}
	}

	// set the new values and store the expression, variable names and used data columns
	const QString& expression{ui.teEquation->toPlainText()};
	bool autoUpdate{(ui.chkAutoUpdate->checkState() == Qt::Checked)};
	bool autoResize{(ui.chkAutoResize->checkState() == Qt::Checked)};
	for (auto* col : m_columns) {
		col->setColumnMode(AbstractColumn::ColumnMode::Double);
		col->setFormula(expression, variableNames, variableColumns, autoUpdate, autoResize);
		col->updateFormula();
	}
	m_spreadsheet->endMacro();
	RESET_CURSOR;
}
