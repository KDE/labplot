/*
    File                 : ThemesComboBox.cpp
    Project              : LabPlot
    Description          : Preview of all themes in a QComboBox
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2017 Alexander Semke (alexander.semke@web.de)

*/

/***************************************************************************
 *                                                                         *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *                                                                         *
 ***************************************************************************/

#include "kdefrontend/widgets/ThemesComboBox.h"
#include "kdefrontend/widgets/ThemesWidget.h"

#include <QEvent>
#include <QGroupBox>
#include <QVBoxLayout>

#include <KLocalizedString>

/*!
    \class ThemesComboBox
    \brief Preview of all themes in a QComboBox.

    \ingroup backend/widgets
*/
ThemesComboBox::ThemesComboBox(QWidget* parent) : QComboBox(parent) {
	auto* layout = new QVBoxLayout;
	m_view = new ThemesWidget(this);
	m_groupBox = new QGroupBox;

	layout->setContentsMargins(0, 0, 0, 0);
	layout->setSpacing(0);
	layout->addWidget(m_view);

	m_groupBox->setLayout(layout);
	m_groupBox->setParent(parent, Qt::Popup);
	m_groupBox->hide();
	m_groupBox->installEventFilter(this);

	addItem(QString());
	setCurrentIndex(0);

	connect(m_view, &ThemesWidget::themeSelected, this, &ThemesComboBox::handleThemeChanged);
}

void ThemesComboBox::showPopup() {
	m_groupBox->show();
	m_groupBox->resize(this->width(), 250);
	m_groupBox->move(mapToGlobal( this->rect().topLeft() ));
}

void ThemesComboBox::hidePopup() {
	m_groupBox->hide();
}

/*!
	catches the MouseButtonPress-event and hides the tree view on mouse clicking.
*/
bool ThemesComboBox::eventFilter(QObject* object, QEvent* event) {
	if ( (object == m_groupBox) && event->type() == QEvent::MouseButtonPress ) {
		m_groupBox->hide();
		this->setFocus();
		return true;
	}

	return QComboBox::eventFilter(object, event);
}

void ThemesComboBox::handleThemeChanged(const QString& theme) {
	if (theme != currentText()) {
		if (theme.isEmpty())
			setItemText(0, i18n("Default")); //default theme
		else
			setItemText(0, theme);
		emit currentThemeChanged(theme);
	}

	m_groupBox->hide();
}
