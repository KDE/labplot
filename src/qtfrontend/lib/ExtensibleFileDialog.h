/***************************************************************************
    File                 : ExtensibleFileDialog.h
    Project              : SciDAVis
    --------------------------------------------------------------------
    Copyright            : (C) 2007 by Knut Franke
    Email (use @ for *)  : knut.franke*gmx.de
    Description          : QFileDialog plus generic extension support

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
#ifndef EXTENSIBLE_FILE_DIALOG_H
#define EXTENSIBLE_FILE_DIALOG_H

#include <QFileDialog>
#include <QPushButton>

//! QFileDialog plus generic extension support.
/**
 * This is a simple hack on top of QFileDialog that allows a custom extension widget to be added to
 * the bottom of the dialog. A button is provided for toggling display of this widget on/off.
 *
 * For the placement of button and extension widget, it is assumed that QFileDialog uses a
 * QGridLayout as its top-level layout. Other layouts will probably lead to a strange outlook,
 * although the functionality should stay intact.
 */
class ExtensibleFileDialog : public QFileDialog
{
	Q_OBJECT
	
	public:
		//! Constructor.
		/**
		 * \param parent parent widget (only affects placement of the dialog)
		 * \param extended flag: show/hide the advanced options on start-up
		 * \param flags window flags
		 */
		ExtensibleFileDialog(QWidget *parent=0, bool extended = true, Qt::WFlags flags=0);
		//! Set the extension widget to be displayed when the user presses the toggle button.
		void setExtensionWidget(QWidget *extension);
		//! Get the extension widget.
		QWidget * extensionWidget() const { return m_extension; }
	
		//! Tells weather the dialog has a valid extension widget
		bool isExtendable(){return m_extension != NULL;};
		bool isExtended(){return m_extension_toggle->isChecked();};
		void setExtended(bool extended){ m_extension_toggle->setChecked(extended);};
		
	protected:
		//! Button for toggling display of extension on/off.
		QPushButton *m_extension_toggle;

	private slots:
		//! Resize to make/take space for the extension widget.
		void resize(bool extension_on);

	private:
		//! The extension widget
		QWidget *m_extension;
		//! The layout row (of the assumed QGridLayout) used for extensions
		int m_extension_row;
};

#endif // ifndef EXTENSIBLE_FILE_DIALOG_H
