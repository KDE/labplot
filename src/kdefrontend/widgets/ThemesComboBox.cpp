/***************************************************************************
    File                 : ThemesComboBox.cpp
    Project              : LabPlot
    Description          : Preview of all themes in a QComboBox
    --------------------------------------------------------------------
    Copyright            : (C) 2017 by Alexander Semke (alexander.semke@web.de)

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *  This program is free software; you can redistribute it and/or modify   *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation; either version 2 of the License, or      *
 *  (at your option) any later version.                                    *
 *                                                                         *
 *  This program is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the Free Software           *
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor,                    *
 *   Boston, MA  02110-1301  USA                                           *
 *                                                                         *
 ***************************************************************************/

#include "kdefrontend/widgets/ThemesComboBox.h"
#include "kdefrontend/widgets/ThemesWidget.h"

#include <QEvent>
#include <QGroupBox>
#include <QVBoxLayout>

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

	addItem("");
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
	return false;
}

void ThemesComboBox::handleThemeChanged(const QString& theme) {
	if (theme != currentText()) {
		setItemText(0, theme);
		emit currentThemeChanged(theme);
	}

	m_groupBox->hide();
}
