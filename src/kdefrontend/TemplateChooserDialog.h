#ifndef TEMPLATECHOOSERDIALOG_H
#define TEMPLATECHOOSERDIALOG_H

#include <QAbstractListModel>
#include <QDialog>
#include <QDir>

class Project;
class Worksheet;
class TemplateListModel;
class CartesianPlot;

namespace Ui {
class TemplateChooserDialog;
}

class TemplateChooserDialog : public QDialog {
	Q_OBJECT

public:
	explicit TemplateChooserDialog(QWidget* parent = nullptr);
	void updateErrorMessage(const QString& message);
	~TemplateChooserDialog();

	QString templatePath() const;
	void showPreview();
	static const QString format;
	static QString defaultTemplateInstallPath();
	CartesianPlot* generatePlot();

private:
	void chooseTemplate();
	void listViewTemplateChanged(const QModelIndex& current, const QModelIndex& previous);
	void changePreviewSource(bool custom);
	void customTemplatePathChanged(const QString& filename);

private:
	Ui::TemplateChooserDialog* ui;
	Project* m_project; // TODO: use smart pointer!
	Worksheet* m_worksheet;
	QWidget* m_worksheetView;
	TemplateListModel* mTemplateListModel;
	bool mLoading{false};
	QString mCurrentTemplateFilePath;
};

class TemplateListModel : public QAbstractListModel {
public:
	TemplateListModel(const QString& searchPath, QObject* parent = nullptr);
	virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
	virtual int rowCount(const QModelIndex& parent = QModelIndex()) const override;

	enum Roles {
		FilenameRole = Qt::ItemDataRole::UserRole + 1,
		FilePathRole,
	};

	struct File {
		QString path; // path with name and extension
		QString filename;
	};

private:
	QVector<File> mFiles;
};

#endif // TEMPLATECHOOSERDIALOG_H
