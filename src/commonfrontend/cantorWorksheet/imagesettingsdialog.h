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

#ifndef IMAGESETTINGSDIALOG_H
#define IMAGESETTINGSDIALOG_H

#include <KDialog>

#include <ui_imagesettings.h>

struct ImageSize
{
    enum {Auto = 0, Pixel = 1, Percent = 2};
    double width;
    double height;
    int widthUnit;
    int heightUnit;
};

class ImageSettingsDialog : public KDialog
{
    Q_OBJECT
  public:
    ImageSettingsDialog(QWidget* parent);
    ~ImageSettingsDialog();

    void setData(const QString& file, const ImageSize& displaySize, const ImageSize& printSize, bool useDisplaySizeForPrinting);

  Q_SIGNALS:
    void dataChanged(const QString& file, const ImageSize& displaySize, const ImageSize& printSize, bool useDisplaySizeForPrinting);

  private Q_SLOTS:
    void sendChangesAndClose();
    void sendChanges();

    void openDialog();
    void updatePreview();
    void updateInputWidgets();
    void updatePrintingGroup(int b);

  private:
    QList<QString> m_unitNames;
    Ui_ImageSettingsBase m_ui;

};

#endif //IMAGESETTINGSDIALOG_H
