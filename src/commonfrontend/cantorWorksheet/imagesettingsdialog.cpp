/*
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation; either version 2
    of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA  02110-1301, USA.

    ---
    Copyright (C) 2011 Martin Kuettler <martin.kuettler@gmail.com>
 */

#include <KCompletion>
#include <KLocale>
#include <KUrl>
#include <KUrlCompletion>

#include "imagesettingsdialog.h"
#include "qimagereader.h"
#include "qfiledialog.h"

ImageSettingsDialog::ImageSettingsDialog(QWidget* parent) : KDialog(parent)
{
    QWidget *w = new QWidget(this);
    m_ui.setupUi(w);
    setMainWidget(w);
    setButtons( KDialog::Ok | KDialog::Cancel | KDialog::Apply );

    m_unitNames << i18n("(auto)") << i18n("px") << i18n("%");

    m_ui.displayWidthCombo->addItems(m_unitNames);
    m_ui.displayHeightCombo->addItems(m_unitNames);
    m_ui.printWidthCombo->addItems(m_unitNames);
    m_ui.printHeightCombo->addItems(m_unitNames);

    KUrlCompletion* completer = new KUrlCompletion(KUrlCompletion::FileCompletion);
    completer->setCompletionMode(KCompletion::CompletionMan);
    m_ui.pathEdit->setCompletionObject(completer);
    m_ui.pathEdit->setAutoDeleteCompletionObject( true );

    m_ui.displayWidthInput->setMinimum(0);
    m_ui.displayHeightInput->setMinimum(0);
    m_ui.printWidthInput->setMinimum(0);
    m_ui.printHeightInput->setMinimum(0);
    m_ui.displayWidthInput->setSingleStep(1);
    m_ui.displayHeightInput->setSingleStep(1);
    m_ui.printWidthInput->setSingleStep(1);
    m_ui.printHeightInput->setSingleStep(1);

    connect(this, &ImageSettingsDialog::okClicked, this, &ImageSettingsDialog::sendChangesAndClose);
    connect(this, &ImageSettingsDialog::applyClicked, this, &ImageSettingsDialog::sendChanges);
    connect(this, &ImageSettingsDialog::cancelClicked, this, &ImageSettingsDialog::close);

    connect(m_ui.openDialogButton, &KPushButton::clicked, this, &ImageSettingsDialog::openDialog);
    //connect(m_fileDialog, SIGNAL(accepted()), this, SLOT(updatePath()));

    connect(m_ui.pathEdit, &KLineEdit::editingFinished, this, &ImageSettingsDialog::updatePreview);

    connect(m_ui.displayWidthCombo, static_cast<void (KComboBox::*)(int)>(&KComboBox::currentIndexChanged), this, &ImageSettingsDialog::updateInputWidgets);
    connect(m_ui.displayHeightCombo, static_cast<void (KComboBox::*)(int)>(&KComboBox::currentIndexChanged), this, &ImageSettingsDialog::updateInputWidgets);
    connect(m_ui.printWidthCombo, static_cast<void (KComboBox::*)(int)>(&KComboBox::currentIndexChanged), this, &ImageSettingsDialog::updateInputWidgets);
    connect(m_ui.printHeightCombo, static_cast<void (KComboBox::*)(int)>(&KComboBox::currentIndexChanged), this, &ImageSettingsDialog::updateInputWidgets);

    connect(m_ui.useDisplaySize, &QCheckBox::stateChanged, this, &ImageSettingsDialog::updatePrintingGroup);
}

ImageSettingsDialog::~ImageSettingsDialog()
{

}

void ImageSettingsDialog::setData(const QString& file, const ImageSize& displaySize, const ImageSize& printSize, bool useDisplaySizeForPrinting)
{
    m_ui.pathEdit->setText(file);
    if (displaySize.width >= 0)
	m_ui.displayWidthInput->setValue(displaySize.width);
    if (displaySize.height >= 0)
	m_ui.displayHeightInput->setValue(displaySize.height);
    if (printSize.width >= 0)
	m_ui.printWidthInput->setValue(printSize.width);
    if (printSize.height >= 0)
	m_ui.printHeightInput->setValue(printSize.height);
    m_ui.displayWidthCombo->setCurrentIndex(displaySize.widthUnit);
    m_ui.displayHeightCombo->setCurrentIndex(displaySize.heightUnit);
    m_ui.printWidthCombo->setCurrentIndex(printSize.widthUnit);
    m_ui.printHeightCombo->setCurrentIndex(printSize.heightUnit);
    if (useDisplaySizeForPrinting)
	m_ui.useDisplaySize->setCheckState(Qt::Checked);
    else
	m_ui.useDisplaySize->setCheckState(Qt::Unchecked);

    updatePreview();
    updatePrintingGroup(useDisplaySizeForPrinting);
    //updateInputWidgets();

}

void ImageSettingsDialog::sendChangesAndClose()
{
    sendChanges();
    close();
}

void ImageSettingsDialog::sendChanges()
{
    ImageSize displaySize, printSize;
    displaySize.width = m_ui.displayWidthInput->value();
    displaySize.height = m_ui.displayHeightInput->value();
    displaySize.widthUnit = m_ui.displayWidthCombo->currentIndex();
    displaySize.heightUnit = m_ui.displayHeightCombo->currentIndex();
    printSize.width = m_ui.printWidthInput->value();
    printSize.height = m_ui.printHeightInput->value();
    printSize.widthUnit = m_ui.printWidthCombo->currentIndex();
    printSize.heightUnit = m_ui.printHeightCombo->currentIndex();

    emit dataChanged
	(m_ui.pathEdit->text(), displaySize, printSize,
	 (m_ui.useDisplaySize->checkState() == Qt::Checked));
}

void ImageSettingsDialog::openDialog()
{
    QList<QByteArray> formats = QImageReader::supportedImageFormats();
    QString formatString = QLatin1String("Images(");
    foreach(QByteArray format, formats)
    {
    formatString += QLatin1String("*.") + QString::fromLatin1(format).toLower() + QLatin1String(" ");
    }
    formatString += QLatin1String(")");
    QString file = QFileDialog::getOpenFileName(this, i18n("Open image file"), m_ui.pathEdit->text(), formatString);
    if (!file.isEmpty())
    {
	m_ui.pathEdit->setText(file);
	updatePreview();
    }
}

void ImageSettingsDialog::updatePreview()
{
    m_ui.imagePreview->showPreview(KUrl(m_ui.pathEdit->text()));
}

void ImageSettingsDialog::updateInputWidgets()
{
    if (m_ui.displayWidthCombo->currentIndex() == 0)
	m_ui.displayWidthInput->setEnabled(false);
    else
	m_ui.displayWidthInput->setEnabled(true);

    if (m_ui.displayHeightCombo->currentIndex() == 0)
	m_ui.displayHeightInput->setEnabled(false);
    else
	m_ui.displayHeightInput->setEnabled(true);

    if (m_ui.printWidthCombo->currentIndex() == 0 || !m_ui.printWidthCombo->isEnabled())
	m_ui.printWidthInput->setEnabled(false);
    else
	m_ui.printWidthInput->setEnabled(true);

    if (m_ui.printHeightCombo->currentIndex() == 0 || !m_ui.printHeightCombo->isEnabled())
	m_ui.printHeightInput->setEnabled(false);
    else
	m_ui.printHeightInput->setEnabled(true);
}

void ImageSettingsDialog::updatePrintingGroup(int b)
{

    m_ui.printWidthCombo->setEnabled(!b);
    m_ui.printHeightCombo->setEnabled(!b);

    updateInputWidgets();
}
