/***************************************************************************
File                 : DatasetMetadataManagerDialog.cpp
Project              : LabPlot
Description          : dialog for managing a metadata file of a dataset
--------------------------------------------------------------------
Copyright            : (C) 2019 Ferencz Kovacs (kferike98@gmail.com)

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

#ifndef DATASETMETADATAMANAGERDIALOG_H
#define DATASETMETADATAMANAGERDIALOG_H

#include <QDialog>

class QDialogButtonBox;
class DatasetMetadataManagerWidget;

class DatasetMetadataManagerDialog : public QDialog {
    Q_OBJECT

public:
    explicit DatasetMetadataManagerDialog(QWidget*, const QMap<QString, QMap<QString, QVector<QString>>>&);
    ~DatasetMetadataManagerDialog() override;
    void updateDocument(const QString& fileName);
    void createNewMetadata(const QString& dirPath);

private:
    DatasetMetadataManagerWidget* m_mainWidget;
    QDialogButtonBox* m_buttonBox;
    QPushButton* okButton{nullptr};

protected  slots:
    void checkOkButton();

};
#endif // DATASETMETADATAMANAGERDIALOG_H
