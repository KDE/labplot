/***************************************************************************
    File                 : TextDialog.cpp
    Project              : SciDAVis
    --------------------------------------------------------------------
    Copyright            : (C) 2006 by Ion Vasilief, Tilman Benkert
    Email (use @ for *)  : ion_vasilief*yahoo.fr, thzs*gmx.net
    Description          : Text label/axis label options dialog
                           
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

#include "TextDialog.h"
#include "ColorButton.h"
#include "TextFormatButtons.h"

#include <QFontDialog>
#include <QColorDialog>
#include <QFont>
#include <QGroupBox>
#include <QTextEdit>
#include <QTextCursor>
#include <QComboBox>
#include <QLabel>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QSpinBox>

TextDialog::TextDialog(TextType type, QWidget* parent, Qt::WFlags fl )
	: QDialog( parent, fl )
{
	setWindowTitle( tr( "Text options" ) );
	setSizeGripEnabled( true );

	textType = type;

	// top groupbox
	groupBox1 = new QGroupBox(QString());

	// grid layout for top groupbox
	QGridLayout * topLayout = new QGridLayout(groupBox1);
	// add text color label
	topLayout->addWidget(new QLabel(tr("Text Color")), 0, 0);

	colorBtn = new ColorButton();
	// add color button
	topLayout->addWidget(colorBtn, 0, 1);

	buttonOk = new QPushButton(tr("&OK"));
	buttonOk->setAutoDefault( true );
	buttonOk->setDefault( true );

	// add ok button
	topLayout->addWidget(buttonOk, 0, 3);

	// add font label
	topLayout->addWidget(new QLabel(tr("Font")), 1, 0);

	buttonFont = new QPushButton(tr( "&Font" ));

	// add font button
	topLayout->addWidget(buttonFont, 1, 1);

	buttonApply = new QPushButton(tr( "&Apply" ));
	buttonApply->setDefault( true );
	
	// add apply button
	topLayout->addWidget( buttonApply, 1, 3 );

	if (textType == TextDialog::AxisTitle)
	{
		// add label "alignment"
		topLayout->addWidget(new QLabel(tr("Alignment")), 2, 0);
		alignmentBox = new QComboBox();
		alignmentBox->addItem( tr( "Center" ) );
		alignmentBox->addItem( tr( "Left" ) );
		alignmentBox->addItem( tr( "Right" ) );
		// add alignment combo box
		topLayout->addWidget(alignmentBox, 2, 1);
	}
	else
	{
		// add label "frame"
		topLayout->addWidget(new QLabel(tr("Frame")), 2, 0);
		backgroundBox = new QComboBox();
		backgroundBox->addItem( tr( "None" ) );
		backgroundBox->addItem( tr( "Rectangle" ) );
		backgroundBox->addItem( tr( "Shadow" ) );
		topLayout->addWidget(backgroundBox, 2, 1);
	}

	buttonCancel = new QPushButton( tr( "&Cancel" ) );
	// add cancel button
	topLayout->addWidget( buttonCancel, 2, 3 );

	if (textType == TextDialog::TextMarker)
	{ //TODO: Sometime background features for axes lables should be implemented
		topLayout->addWidget(new QLabel(tr("Opacity")), 3, 0);
		boxBackgroundTransparency = new QSpinBox();
		boxBackgroundTransparency->setRange(0, 255);
     	boxBackgroundTransparency->setSingleStep(5);
		boxBackgroundTransparency->setWrapping(true);
     	boxBackgroundTransparency->setSpecialValueText(tr("Transparent"));
		// add background button
		topLayout->addWidget( boxBackgroundTransparency, 3, 1 );
		
		// add label "background color"	
		topLayout->addWidget(new QLabel(tr("Background color")), 4, 0);
		backgroundBtn = new ColorButton(groupBox1);
		backgroundBtn->setEnabled(false);
		// add background button
		topLayout->addWidget( backgroundBtn, 4, 1 );	

		connect(backgroundBtn, SIGNAL(clicked()), this, SLOT(pickBackgroundColor()));
		connect(boxBackgroundTransparency, SIGNAL(valueChanged(int)), 
				this, SLOT(updateTransparency(int)));

		buttonDefault = new QPushButton( tr( "Set As &Default" ) );
		topLayout->addWidget( buttonDefault, 3, 3 );
		connect( buttonDefault, SIGNAL(clicked()), this, SLOT(setDefaultValues()));
	}

	// align the OK, Apply, and Cancel buttons to the right
	topLayout->setColumnStretch(2, 1);

	/* TODO: Angle feature not implemented, yet
	 * caution: This code is still the old Qt3 code
	   QLabel* rotate=new QLabel(tr( "Rotate (deg.)" ),GroupBox1, "TextLabel1_2",0);
	   rotate->hide();

	   rotateBox = new QComboBox( false, GroupBox1, "rotateBox" );
	   rotateBox->insertItem( tr( "0" ) );
	   rotateBox->insertItem( tr( "45" ) );
	   rotateBox->insertItem( tr( "90" ) );
	   rotateBox->insertItem( tr( "135" ) );
	   rotateBox->insertItem( tr( "180" ) );
	   rotateBox->insertItem( tr( "225" ) );
	   rotateBox->insertItem( tr( "270" ) );
	   rotateBox->insertItem( tr( "315" ) );
	   rotateBox->setEditable (true);
	   rotateBox->setCurrentItem(0);
	   rotateBox->hide();
	   */

	textEditBox = new QTextEdit();
	textEditBox->setAcceptRichText(false);
	textEditBox->setFont(QFont());

	formatButtons =  new TextFormatButtons(textEditBox);
	formatButtons->toggleCurveButton(textType == TextDialog::TextMarker);

	setFocusPolicy(Qt::StrongFocus);
	setFocusProxy(textEditBox);

	// put everything together
	QVBoxLayout* mainLayout = new QVBoxLayout();
	mainLayout->addWidget(groupBox1);
	mainLayout->addWidget(formatButtons);
	mainLayout->addWidget(textEditBox);
	setLayout( mainLayout );

	// signals and slots connections
	connect( colorBtn, SIGNAL( clicked() ), this, SLOT( pickTextColor() ) );
	connect( buttonOk, SIGNAL( clicked() ), this, SLOT( accept() ) );
	connect( buttonApply, SIGNAL( clicked() ), this, SLOT( apply() ) );
	connect( buttonCancel, SIGNAL( clicked() ), this, SLOT( reject() ) );
	connect( buttonFont, SIGNAL( clicked() ), this, SLOT(customFont() ) );
}

void TextDialog::apply()
{
	if (textType == TextDialog::AxisTitle)
	{
		emit changeAlignment(alignment());
		emit changeText(textEditBox->toPlainText());
		emit changeColor(colorBtn->color());
	}
	else
	{
		QColor c = backgroundBtn->color();
		c.setAlpha(boxBackgroundTransparency->value());

		emit values(textEditBox->toPlainText(), angle(), backgroundBox->currentIndex(), selectedFont, colorBtn->color(), c);
	}
}

void TextDialog::setDefaultValues()
{
	QColor c = backgroundBtn->color();
	c.setAlpha(boxBackgroundTransparency->value());
	emit defaultValues(backgroundBox->currentIndex(), selectedFont, colorBtn->color(), c);
}

void TextDialog::accept()
{
	apply();
	close();
}

void TextDialog::setBackgroundType(int bkg)
{
	backgroundBox->setCurrentIndex(bkg);
}

int TextDialog::alignment()
{
	int align=-1;
	switch (alignmentBox->currentIndex())
	{
		case 0:
			align = Qt::AlignHCenter;
			break;

		case 1:
			align = Qt::AlignLeft;
			break;

		case 2:
			align = Qt::AlignRight;
			break;
	}
	return align;
}

void TextDialog::setAlignment(int align)
{	
	switch(align)
	{
		case Qt::AlignHCenter:
			alignmentBox->setCurrentIndex(0);
			break;
		case Qt::AlignLeft:
			alignmentBox->setCurrentIndex(1);
			break;
		case Qt::AlignRight:
			alignmentBox->setCurrentIndex(2);
			break;
	}
}

void TextDialog::customFont()
{
	bool okF;
	QFont fnt = QFontDialog::getFont( &okF, selectedFont, this);
	if (okF && fnt != selectedFont)
	{
		selectedFont = fnt;		
		emit changeFont (fnt);
	}
}

void TextDialog::setAngle(int /*angle*/)
{
	//TODO: Implement angle feature 
//X	rotateBox-> ...
}

int TextDialog::angle()
{
	//TODO: Implement angle feature
//X	return rotateBox-> ...
	return 0;
}

void TextDialog::setText(const QString & t)
{
	QTextCursor cursor = textEditBox->textCursor();
	// select the whole (old) text 
	cursor.movePosition(QTextCursor::Start);
	cursor.movePosition(QTextCursor::End,QTextCursor::KeepAnchor);
	// replace old text
	cursor.insertText(t);
	// select the whole (new) text
	cursor.movePosition(QTextCursor::Start);
	cursor.movePosition(QTextCursor::End,QTextCursor::KeepAnchor);
	// this line makes the selection visible to the user 
	// (the 2 lines above only change the selection in the
	// underlying QTextDocument)
	textEditBox->setTextCursor(cursor);
	// give focus back to text edit
	textEditBox->setFocus();
}

void TextDialog::setTextColor(QColor c)
{
	colorBtn->setColor(c);
}

void TextDialog::pickTextColor()
{
	QColor c = QColorDialog::getColor( colorBtn->color(), this);
	if ( !c.isValid() || c ==  colorBtn->color() )
		return;

	colorBtn->setColor ( c ) ;
}

void TextDialog::setBackgroundColor(QColor c)
{
	boxBackgroundTransparency->setValue(c.alpha());
	backgroundBtn->setEnabled(c.alpha());
	c.setAlpha(255);
	
	backgroundBtn->setColor(c);
}

void TextDialog::pickBackgroundColor()
{
	QColor c = QColorDialog::getColor( backgroundBtn->color(), this);
	if ( !c.isValid() || c ==  backgroundBtn->color() )
		return;

	backgroundBtn->setColor ( c ) ;
}

void TextDialog::setFont(const QFont & fnt)
{
	selectedFont = fnt;
}

void TextDialog::updateTransparency(int alpha)
{
backgroundBtn->setEnabled(alpha);
}
