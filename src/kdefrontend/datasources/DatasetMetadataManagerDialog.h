/*
    File                 : DatasetMetadataManagerDialog.h
    Project              : LabPlot
    Description          : Dialog for managing a metadata file of a dataset
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2019 Ferencz Kovacs <kferike98@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef DATASETMETADATAMANAGERDIALOG_H
#define DATASETMETADATAMANAGERDIALOG_H

#include <QDialog>

class QDialogButtonBox;
class DatasetMetadataManagerWidget;

class DatasetMetadataManagerDialog : public QDialog {
    Q_OBJECT

public:
	explicit DatasetMetadataManagerDialog(QWidget*, const QMap< QString, QMap<QString, QMap<QString, QVector<QString>>>>&);
    virtual ~DatasetMetadataManagerDialog() override;
	void updateDocument(const QString& fileName);
    QString getMetadataFilePath() const;

	void setCollection(const QString&);
	void setCategory(const QString&);
	void setSubcategory(const QString&);
	void setShortName(const QString&);
	void setFullName(const QString&);
	void setDescription(const QString&);
	void setURL(const QString&);

private:
    DatasetMetadataManagerWidget* m_mainWidget;
    QDialogButtonBox* m_buttonBox;
    QPushButton* m_okButton;

protected Q_SLOTS:
    void checkOkButton();

};
#endif // DATASETMETADATAMANAGERDIALOG_H
