/*
	File                 : ConstantsWidget.cc
	Project              : LabPlot
	Description          : widget for selecting constants
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2014 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "ConstantsWidget.h"
#include "backend/gsl/ExpressionParser.h"
#include <QTimer>

/*!
	\class ConstantsWidget
	\brief Widget for selecting supported mathematical and physical constants
	that can be used in expressions in \c ExpressionTextEdit.

	\ingroup frontend
 */
ConstantsWidget::ConstantsWidget(QWidget* parent)
	: QWidget(parent) {
	ui.setupUi(this);
	ui.bInsert->setIcon(QIcon::fromTheme(QStringLiteral("edit-paste")));
	ui.bCancel->setIcon(QIcon::fromTheme(QStringLiteral("dialog-cancel")));
	m_expressionParser = ExpressionParser::getInstance();

	for (int i = 0; i < (int)Parsing::ConstantGroups::END; i++)
		ui.cbGroup->addItem(Parsing::constantGroupsToString(static_cast<Parsing::ConstantGroups>(i)), i);

	// SLOTS
	connect(ui.leFilter, &QLineEdit::textChanged, this, &ConstantsWidget::filterChanged);
	connect(ui.cbGroup, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ConstantsWidget::groupChanged);
	connect(ui.lwConstants, &QListWidget::currentTextChanged, this, &ConstantsWidget::constantChanged);
	connect(ui.bInsert, &QPushButton::clicked, this, &ConstantsWidget::insertClicked);
	connect(ui.bCancel, &QPushButton::clicked, this, &ConstantsWidget::canceled);
	connect(ui.lwConstants, &QListWidget::itemDoubleClicked, this, &ConstantsWidget::insertClicked);

	// set the focus to the search field and select the first group after the widget is shown
	QTimer::singleShot(0, this, [=]() {
		ui.leFilter->setFocus();
		this->groupChanged(0);
	});

	// set the minimum size to show the longest constant name without any horizontal scroll bar
	const QStringList& names = m_expressionParser->constantsNames();
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
 * shows all constants of the selected group and selects the first one in the QStringList
 */
void ConstantsWidget::groupChanged(int index) {
	static const QStringList& constants = m_expressionParser->constants();
	static const QStringList& names = m_expressionParser->constantsNames();
	static const QVector<Parsing::ConstantGroups>& indices = m_expressionParser->constantsGroupIndices();

	const auto group = static_cast<Parsing::ConstantGroups>(ui.cbGroup->itemData(index).toInt());

	ui.lwConstants->clear();
	for (int i = 0; i < names.size(); ++i) {
		if (indices.at(i) == group)
			ui.lwConstants->addItem(names.at(i) + QStringLiteral(" (") + constants.at(i) + QStringLiteral(")"));
	}
	ui.lwConstants->setCurrentRow(0);
}

void ConstantsWidget::filterChanged(const QString& filter) {
	if (!filter.isEmpty()) {
		ui.cbGroup->setEnabled(false);

		static const auto& names = m_expressionParser->constantsNames();
		static const auto& constants = m_expressionParser->constants();
		ui.lwConstants->clear();
		for (int i = 0; i < names.size(); ++i) {
			if (names.at(i).contains(filter, Qt::CaseInsensitive) || constants.at(i).contains(filter, Qt::CaseInsensitive))
				ui.lwConstants->addItem(names.at(i) + QStringLiteral(" (") + constants.at(i) + QStringLiteral(")"));
		}

		if (ui.lwConstants->count()) {
			ui.lwConstants->setCurrentRow(0);
			ui.bInsert->setEnabled(true);
		} else {
			ui.leValue->setText(QString());
			ui.lUnit->setText(QString());
			ui.bInsert->setEnabled(false);
		}
	} else {
		ui.cbGroup->setEnabled(true);
		groupChanged(ui.cbGroup->currentIndex());
	}
}

void ConstantsWidget::constantChanged(const QString& text) {
	static const auto& names = m_expressionParser->constantsNames();
	static const auto& values = m_expressionParser->constantsValues();
	static const auto& units = m_expressionParser->constantsUnits();

	QString name = text.left(text.indexOf(QStringLiteral(" (")));
	int index = names.indexOf(name);
	if (index != -1) {
		ui.leValue->setText(values.at(index));
		ui.leValue->setCursorPosition(0);
		ui.lUnit->setText(units.at(index));
	}
}

void ConstantsWidget::insertClicked() {
	static const auto& constants = m_expressionParser->constants();
	static const auto& names = m_expressionParser->constantsNames();

	// determine the currently selected constant
	const QString& text = ui.lwConstants->currentItem()->text();
	const QString& name = text.left(text.indexOf(QStringLiteral(" (")));
	int index = names.indexOf(name);
	const QString& constant = constants.at(index);

	Q_EMIT constantSelected(constant);
}
