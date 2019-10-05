/***************************************************************************
    File             : BaseDock.h
    Project          : LabPlot
	Description      : Base dock widget
    --------------------------------------------------------------------
	Copyright         : (C) 2019 Martin Marmsoler (martin.marmsoler@gmail.com)

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

#ifndef BASEDOCK
#define BASEDOCK

#include <QWidget>
#include <QLineEdit>

class AbstractAspect;

struct Lock {
	inline Lock(bool& variable)
		: variable(variable = true){
	}

	inline ~Lock(){
		variable = false;
	}

private:
	bool& variable;
};


class BaseDock : public QWidget {
	Q_OBJECT

public:
	explicit BaseDock(QWidget* parent);
	~BaseDock();

protected:
	bool m_initializing{false};
	QLineEdit* m_leName{nullptr};
	QLineEdit* m_leComment{nullptr};
	AbstractAspect* m_aspect{nullptr};
	QList<AbstractAspect*> m_aspects;

protected slots:
	void nameChanged();
	void commentChanged();
};

#endif
