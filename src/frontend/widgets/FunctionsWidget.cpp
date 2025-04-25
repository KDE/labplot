/*
	File                 : FunctionsWidget.cc
	Project              : LabPlot
	Description          : widget for selecting functions
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2014-2023 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "FunctionsWidget.h"
#include "backend/gsl/ExpressionParser.h"
#include <QTimer>

/*!
	\class FunctionsWidget
	\brief Widget for selecting supported mathematical functions
	that can be used in expressions in \c ExpressionTextEdit.

	\ingroup frontend
 */
FunctionsWidget::FunctionsWidget(QWidget* parent)
	: QWidget(parent) {
	using Groups = Parsing::FunctionGroups;

	ui.setupUi(this);
	ui.bInsert->setIcon(QIcon::fromTheme(QStringLiteral("edit-paste")));
	ui.bCancel->setIcon(QIcon::fromTheme(QStringLiteral("dialog-cancel")));
	m_expressionParser = ExpressionParser::getInstance();

	QVector<int> separators;
	for (int i = 0; i < (int)Groups::END; i++) {
		const auto group = static_cast<Groups>(i);
		ui.cbGroup->addItem(Parsing::FunctionGroupsToString(group), (int)i);

		// Add separator before these groups
		if (group == Groups::ColumnStatistics || group == Groups::AiryFunctionsAndDerivatives || group == Groups::RandomNumberGenerator
			|| group == Groups::GaussianDistribution)
			separators.append(i);
	}

	for (int i = 0; i < separators.count(); i++)
		ui.cbGroup->insertSeparator(separators.at(i) + i); // + i because of previous added separator

	// SLOTS
	connect(ui.leFilter, &QLineEdit::textChanged, this, &FunctionsWidget::filterChanged);
	connect(ui.cbGroup, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &FunctionsWidget::groupChanged);
	connect(ui.bInsert, &QPushButton::clicked, this, &FunctionsWidget::insertClicked);
	connect(ui.bCancel, &QPushButton::clicked, this, &FunctionsWidget::canceled);
	connect(ui.lwFunctions, &QListWidget::itemDoubleClicked, this, &FunctionsWidget::insertClicked);

	// set the focus to the search field and select the first group after the widget is shown
	QTimer::singleShot(0, this, [=]() {
		ui.leFilter->setFocus();
		this->groupChanged(0);
	});

	// set the minimum size to show the longest funciton description without any horizontal scroll bar
	const QStringList& names = m_expressionParser->functionsDescriptions();
	QString maxName;
	int maxLength = 0;
	for (const auto& name : names) {
		int length = name.length();
		if (length > maxLength) {
			maxLength = length;
			maxName = name;
		}
	}

	QFont font;
	QFontMetrics fm(font);
	const auto margins = layout()->contentsMargins();
	const int width = fm.horizontalAdvance(maxName) + margins.left() + margins.right() + qApp->style()->pixelMetric(QStyle::PM_ScrollBarExtent);
	setMinimumWidth(width);
}

/*!
 * shows all functions of the selected group and selects the first one in the QStringList
 */
void FunctionsWidget::groupChanged(int index) {
	const auto d = ui.cbGroup->itemData(index);
	if (d.isNull())
		return; // separator selected

	bool ok;
	const auto groupIndex = static_cast<Parsing::FunctionGroups>(d.toInt(&ok));
	if (!ok)
		return;

	static const QStringList& functions = m_expressionParser->functions();
	static const QStringList& names = m_expressionParser->functionsDescriptions();
	static const QVector<Parsing::FunctionGroups>& indices = m_expressionParser->functionsGroupIndices();

	QDEBUG(Q_FUNC_INFO << ", index = " << Parsing::FunctionGroupsToString(groupIndex));

	ui.lwFunctions->clear();
	for (int i = 0; i < names.size(); ++i) {
		if (indices.at(i) == groupIndex)
			ui.lwFunctions->addItem(names.at(i) + QStringLiteral(" (") + functions.at(i) + QStringLiteral(")"));
	}
	ui.lwFunctions->setCurrentRow(0);
}

void FunctionsWidget::filterChanged(const QString& filter) {
	if (!filter.isEmpty()) {
		ui.cbGroup->setEnabled(false);

		static const QStringList& names = m_expressionParser->functionsDescriptions();
		static const QStringList& functions = m_expressionParser->functions();
		ui.lwFunctions->clear();
		for (int i = 0; i < names.size(); ++i) {
			if (names.at(i).contains(filter, Qt::CaseInsensitive) || functions.at(i).contains(filter, Qt::CaseInsensitive))
				ui.lwFunctions->addItem(names.at(i) + QStringLiteral(" (") + functions.at(i) + QStringLiteral(")"));
		}

		if (ui.lwFunctions->count()) {
			ui.lwFunctions->setCurrentRow(0);
			ui.bInsert->setEnabled(true);
		} else {
			ui.bInsert->setEnabled(false);
		}
	} else {
		ui.cbGroup->setEnabled(true);
		groupChanged(ui.cbGroup->currentIndex());
	}
}

void FunctionsWidget::insertClicked() {
	static const QStringList& functions = m_expressionParser->functions();
	static const QStringList& names = m_expressionParser->functionsDescriptions();

	// determine the currently selected function from text
	const QString& text = ui.lwFunctions->currentItem()->text();
	// strip " (FUNCTIONNAME)" from end
	const QString& name = text.left(text.lastIndexOf(QStringLiteral(" (")));
	int index = names.indexOf(name);
	QDEBUG("text = " << text << ", name = " << name << ", index = " << index)

	Q_EMIT functionSelected(functions.at(index));
}
