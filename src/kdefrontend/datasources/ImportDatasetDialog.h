#ifndef IMPORTDATASETDIALOG_H
#define IMPORTDATASETDIALOG_H

#include "ImportDialog.h"

class MainWin;
class ImportDatasetWidget;
class QMenu;
class DatasetHandler;

class ImportDatasetDialog : public ImportDialog {
    Q_OBJECT

public:
    explicit ImportDatasetDialog(MainWin*, const QString& fileName = QString());
    ~ImportDatasetDialog() override;

    QString selectedObject() const override;
    void importToDataset(DatasetHandler*, QStatusBar*) const;

    void importTo(QStatusBar*) const override;


   private:
	ImportDatasetWidget* m_importDatasetWidget;

protected  slots:
        void checkOkButton() override;

};

#endif // IMPORTDATASETDIALOG_H
