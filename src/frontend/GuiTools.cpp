/*
	File                 : GuiTools.cpp
	Project              : LabPlot
	Description          : contains several static functions which are used frequently throughout the kde frontend
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2011-2013 Alexander Semke <alexander.semke@web.de>
	SPDX-FileCopyrightText: 2021-2025 Stefan Gerlach <stefan.gerlach@uni.kn>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "GuiTools.h"
#include "backend/core/Settings.h"
#include "backend/worksheet/plots/cartesian/Symbol.h"
#include "frontend/widgets/TextPreview.h"
#include "frontend/widgets/ExpressionTextEdit.h"

#include <KConfigGroup>
#include <KFileWidget>
#include <KLineEdit>
#include <KLocalizedString>
#include <KUrlComboBox>

#include <QActionGroup>
#include <QApplication>
#include <QColor>
#include <QComboBox>
#include <QFileDialog>
#include <QImageReader>
#include <QCheckBox>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QPainter>
#include <QScreen>
#include <QWidget>
#include <QWindow>
#include <QVBoxLayout>
#include <QPushButton>

#ifdef HAVE_POPPLER
#include <poppler-qt6.h>
#endif

#include <array>

static const int colorsCount = 26;
static std::array<QColor, colorsCount> colors = {
	QColor(255, 255, 255), QColor(0, 0, 0),		  QColor(192, 0, 0),	 QColor(255, 0, 0), QColor(255, 192, 192), // red
	QColor(0, 192, 0),	   QColor(0, 255, 0),	  QColor(192, 255, 192), // green
	QColor(0, 0, 192),	   QColor(0, 0, 255),	  QColor(192, 192, 255), // blue
	QColor(192, 192, 0),   QColor(255, 255, 0),	  QColor(255, 255, 192), // yellow
	QColor(0, 192, 192),   QColor(0, 255, 255),	  QColor(192, 255, 255), // cyan
	QColor(192, 0, 192),   QColor(255, 0, 255),	  QColor(255, 192, 255), // magenta
	QColor(192, 88, 0),	   QColor(255, 128, 0),	  QColor(255, 168, 88), // orange
	QColor(128, 128, 128), QColor(160, 160, 160), QColor(195, 195, 195) // grey
};

bool GuiTools::isDarkMode() {
	return (QApplication::palette().color(QPalette::Base).lightness() < 128);
}

/*!
	fills the ComboBox \c combobox with the six possible Qt::PenStyles, the color \c color is used.
*/
void GuiTools::updatePenStyles(QComboBox* comboBox, const QColor& color) {
	int index = comboBox->currentIndex();
	comboBox->clear();

	QPainter pa;
	int offset = 2;
	int w = 50;
	int h = 10;
	QPixmap pm(w, h);
	comboBox->setIconSize(QSize(w, h));

	// loop over six possible Qt-PenStyles, draw on the pixmap and insert it
	// TODO: avoid copy-paste in all finctions!
	static std::array<QString, 6> list =
		{i18n("No Line"), i18n("Solid Line"), i18n("Dash Line"), i18n("Dot Line"), i18n("Dash-dot Line"), i18n("Dash-dot-dot Line")};
	for (int i = 0; i < 6; i++) {
		pm.fill(Qt::transparent);
		pa.begin(&pm);
		pa.setPen(QPen(color, 1, (Qt::PenStyle)i));
		pa.drawLine(offset, h / 2, w - offset, h / 2);
		pa.end();
		comboBox->addItem(QIcon(pm), list[i]);
	}
	comboBox->setCurrentIndex(index);
}

/*!
	fills the QMenu \c menu with the six possible Qt::PenStyles, the color \c color is used.
	QActions are created with \c actionGroup as the parent, if not available.
	If already available, onle the color in the QAction's icons is updated.
*/
void GuiTools::updatePenStyles(QMenu* menu, QActionGroup* actionGroup, const QColor& color) {
	QPainter pa;
	int offset = 2;
	int w = 50;
	int h = 10;
	QPixmap pm(w, h);

	// loop over six possible Qt-PenStyles, draw on the pixmap and insert it
	static std::array<QString, 6> list =
		{i18n("No Line"), i18n("Solid Line"), i18n("Dash Line"), i18n("Dot Line"), i18n("Dash-dot Line"), i18n("Dash-dot-dot Line")};

	QAction* action;
	if (actionGroup->actions().isEmpty()) {
		// TODO setting of the icon size doesn't work here
		menu->setStyleSheet(QLatin1String("QMenu::icon { width:50px; height:10px; }"));

		for (int i = 0; i < 6; i++) {
			pm.fill(Qt::transparent);
			pa.begin(&pm);
			pa.setPen(QPen(color, 1, (Qt::PenStyle)i));
			pa.drawLine(offset, h / 2, w - offset, h / 2);
			pa.end();
			action = new QAction(QIcon(pm), list[i], actionGroup);
			action->setCheckable(true);
			menu->addAction(action);
		}
	} else {
		for (int i = 0; i < 6; i++) {
			pm.fill(Qt::transparent);
			pa.begin(&pm);
			pa.setPen(QPen(color, 1, (Qt::PenStyle)i));
			pa.drawLine(offset, h / 2, w - offset, h / 2);
			pa.end();
			action = actionGroup->actions().at(i);
			action->setIcon(QIcon(pm));
		}
	}
}

void GuiTools::selectPenStyleAction(QActionGroup* actionGroup, Qt::PenStyle style) {
	int index = (int)style;
	Q_ASSERT(index < actionGroup->actions().size());
	actionGroup->actions().at(index)->setChecked(true);
}

Qt::PenStyle GuiTools::penStyleFromAction(QActionGroup* actionGroup, QAction* action) {
	int index = actionGroup->actions().indexOf(action);
	return Qt::PenStyle(index);
}

/*!
	fills the ComboBox for the symbol filling patterns with the 14 possible Qt::BrushStyles.
*/
void GuiTools::updateBrushStyles(QComboBox* comboBox, const QColor& color) {
	int index = comboBox->currentIndex();
	comboBox->clear();

	int offset = 2;
	int w = 50;
	int h = 20;
	QPixmap pm(w, h);
	comboBox->setIconSize(QSize(w, h));

	static std::array<QString, 15> list = {i18n("None"),
										   i18n("Uniform"),
										   i18n("Extremely Dense"),
										   i18n("Very Dense"),
										   i18n("Somewhat Dense"),
										   i18n("Half Dense"),
										   i18n("Somewhat Sparse"),
										   i18n("Very Sparse"),
										   i18n("Extremely Sparse"),
										   i18n("Horiz. Lines"),
										   i18n("Vert. Lines"),
										   i18n("Crossing Lines"),
										   i18n("Backward Diag. Lines"),
										   i18n("Forward Diag. Lines"),
										   i18n("Crossing Diag. Lines")};
	const QColor& borderColor = GuiTools::isDarkMode() ? Qt::white : Qt::black;
	QPen pen(Qt::SolidPattern, 1);
	pen.setColor(borderColor);

	for (int i = 0; i < 15; i++) {
		QPainter pa;

		pm.fill(Qt::transparent);
		pa.begin(&pm);
		pa.setPen(pen);
		pa.setRenderHint(QPainter::Antialiasing);
		pa.setBrush(QBrush(color, (Qt::BrushStyle)i));
		pa.drawRect(offset, offset, w - 2 * offset, h - 2 * offset);
		pa.end();
		comboBox->addItem(QIcon(pm), list[i]);
	}

	comboBox->setCurrentIndex(index);
}

void GuiTools::fillColorMenu(QMenu* menu, QActionGroup* actionGroup) {
	static const std::array<QString, colorsCount> colorNames = {
		i18n("White"),		 i18n("Black"),		   i18n("Dark Red"),   i18n("Red"),			 i18n("Light Red"),	  i18n("Dark Green"),	 i18n("Green"),
		i18n("Light Green"), i18n("Dark Blue"),	   i18n("Blue"),	   i18n("Light Blue"),	 i18n("Dark Yellow"), i18n("Yellow"),		 i18n("Light Yellow"),
		i18n("Dark Cyan"),	 i18n("Cyan"),		   i18n("Light Cyan"), i18n("Dark Magenta"), i18n("Magenta"),	  i18n("Light Magenta"), i18n("Dark Orange"),
		i18n("Orange"),		 i18n("Light Orange"), i18n("Dark Grey"),  i18n("Grey"),		 i18n("Light Grey")};

	QPixmap pix(16, 16);
	QPainter p(&pix);
	for (int i = 0; i < colorsCount; ++i) {
		p.fillRect(pix.rect(), colors[i]);
		auto* action = new QAction(QIcon(pix), colorNames[i], actionGroup);
		action->setCheckable(true);
		menu->addAction(action);
	}
}

/*!
 * Selects (checks) the action in the group \c actionGroup hat corresponds to the color \c color.
 * Unchecks the previously checked action if the color
 * was not found in the list of predefined colors.
 */
void GuiTools::selectColorAction(QActionGroup* actionGroup, const QColor& color) {
	int index;
	for (index = 0; index < colorsCount; ++index) {
		if (color == colors[index]) {
			actionGroup->actions().at(index)->setChecked(true);
			break;
		}
	}

	if (index == colorsCount) {
		// the color was not found in the list of predefined colors
		//  -> uncheck the previously checked action
		QAction* checkedAction = actionGroup->checkedAction();
		if (checkedAction)
			checkedAction->setChecked(false);
	}
}

QColor& GuiTools::colorFromAction(QActionGroup* actionGroup, QAction* action) {
	int index = actionGroup->actions().indexOf(action);
	if (index == -1 || index >= colorsCount)
		index = 0;

	return colors[index];
}

// Returns current screen DPI as { physicalDotsPerInchX, physicalDotsPerInchY }
QPair<float, float> GuiTools::dpi(const QWidget* widget) {
	QScreen* screen = nullptr;

	if (widget != nullptr) {
		if (widget->window() && widget->window()->windowHandle() && widget->window()->windowHandle()->screen())
			screen = widget->window()->windowHandle()->screen();
		else
			WARN("Widget or related window/screen is null, falling back to primary screen");

	} else
		WARN("Widget is null, falling back to primary screen");

	// Fallback to the primary screen if the widget's screen is not valid
	if (!screen)
		screen = QApplication::primaryScreen();

	// Return the DPI values
	float dpiX = screen->physicalDotsPerInchX();
	float dpiY = screen->physicalDotsPerInchY();
	return {dpiX, dpiY};
}

// ComboBox with colors
// 	QImage img(16,16,QImage::Format_RGB32);
// 	QPainter p(&img);
// 	QRect rect = img.rect().adjusted(1,1,-1,-1);
// 	p.fillRect(rect, Qt::red);
// 	comboBox->setItemData(0, QPixmap::fromImage(img), Qt::DecorationRole);

void GuiTools::highlight(QWidget* widget, bool invalid) {
	if (invalid)
		SET_WARNING_STYLE(widget)
	else
		widget->setStyleSheet(QString());
}

void GuiTools::addSymbolStyles(QComboBox* cb) {
	QPen pen(Qt::SolidPattern, 0);
	const QColor& color = GuiTools::isDarkMode() ? Qt::white : Qt::black;
	pen.setColor(color);

	int iconSize = 20;
	QPixmap pm(iconSize, iconSize);
	cb->setIconSize(QSize(iconSize, iconSize));
	QTransform trafo;
	trafo.scale(15, 15);

	for (int i = 0; i < Symbol::stylesCount(); ++i) {
		QPainter pa;
		// get styles in order
		const auto style = Symbol::indexToStyle(i);

		pm.fill(Qt::transparent);
		pa.begin(&pm);
		pa.setPen(pen);
		pa.setRenderHint(QPainter::Antialiasing);
		pa.translate(iconSize / 2, iconSize / 2);
		pa.drawPath(trafo.map(Symbol::stylePath(style)));
		pa.end();
		cb->addItem(QIcon(pm), Symbol::styleName(style), (int)style);
	}
}

QString GuiTools::openImageFile(const QString& className) {
	KConfigGroup conf = Settings::group(className);
	const QString& dir = conf.readEntry(QLatin1String("LastImageDir"), QString());

	QString formats;
	for (const QByteArray& format : QImageReader::supportedImageFormats()) {
		QString f = QLatin1String("*.") + QLatin1String(format.constData());
		if (f == QLatin1String("*.svg"))
			continue;
		formats += f + QLatin1Char(' ');
	}

	const QString& path = QFileDialog::getOpenFileName(nullptr, i18nc("@title:window", "Open Image File"), dir, i18n("Images (%1)", formats));
	if (!path.isEmpty()) {
		int pos = path.lastIndexOf(QLatin1String("/"));
		if (pos != -1) {
			QString newDir = path.left(pos);
			if (newDir != dir)
				conf.writeEntry(QLatin1String("LastImageDir"), newDir);
		}
	}

	return path;
}

// convert PDF to QImage using Poppler
QImage GuiTools::importPDFFile(const QString& fileName) {
	// DEBUG(Q_FUNC_INFO << ", PDF file name = " << STDSTRING(fileName));
#ifdef HAVE_POPPLER
	auto document = Poppler::Document::load(fileName);
	if (!document) {
		WARN("Failed to process PDF file" << STDSTRING(fileName));
		return {};
	}

	auto page = document->page(0);
	if (!page) {
		WARN("Failed to process the first page in the PDF file.")
		return {};
	}

	document->setRenderHint(Poppler::Document::TextAntialiasing);
	document->setRenderHint(Poppler::Document::Antialiasing);
	document->setRenderHint(Poppler::Document::TextHinting);
	document->setRenderHint(Poppler::Document::TextSlightHinting);
	document->setRenderHint(Poppler::Document::ThinLineSolid);

	const static int dpi = QGuiApplication::primaryScreen()->logicalDotsPerInchX();
	auto image = page->renderToImage(dpi, dpi);

	return image;
#else
	Q_UNUSED(fileName)
	DEBUG(Q_FUNC_INFO << ", POPPLER not available!")
	return {};
#endif
}

QImage GuiTools::imageFromPDFData(const QByteArray& data, double zoomFactor) {
#ifdef HAVE_POPPLER
	auto document = Poppler::Document::loadFromData(data);
	if (!document) {
		WARN("Failed to process the byte array");
		return {};
	}

	auto page = document->page(0);
	if (!page) {
		WARN("Failed to process the first page in the PDF file.")
		return {};
	}

	document->setRenderHint(Poppler::Document::TextAntialiasing);
	document->setRenderHint(Poppler::Document::Antialiasing);
	document->setRenderHint(Poppler::Document::TextHinting);
	document->setRenderHint(Poppler::Document::TextSlightHinting);
	document->setRenderHint(Poppler::Document::ThinLineSolid);

	const static int dpi = QGuiApplication::primaryScreen()->logicalDotsPerInchX();
	auto image = page->renderToImage(zoomFactor * dpi, zoomFactor * dpi);

	return image;
#else
	Q_UNUSED(data)
	Q_UNUSED(zoomFactor)
	return {};
#endif
}

/*
 * replaces the file extension in the path name \c path with \c extension or adds it if not extension available yet.
 */
QString GuiTools::replaceExtension(const QString& path, const QString& extension) {
	const int lastSeparatorIndex = path.lastIndexOf(QDir::separator());
	const int index = path.lastIndexOf(QLatin1Char('.'));
	QString newPath;
	if (index < lastSeparatorIndex) // dot in front of the last separator, is part of the path and not of the file name -> no extension available yet
		newPath = path + extension;
	else
		newPath = path.left(index) + extension;

	return newPath;
}

/*
 * load custom function from file
 * using TextEdit. Only FitCurve used cbCategory and cbModel
 */
void GuiTools::loadFunction(ExpressionTextEdit* te, QComboBox* cbCategory, KComboBox* cbModel) {
	//easy alternative: const QString& fileName = QFileDialog::getOpenFileName(this, i18nc("@title:window", "Select file to load function definition"), dir, filter);

	QDialog dialog;
	dialog.setWindowTitle(i18n("Select file to load function definition"));
	auto* layout = new QVBoxLayout(&dialog);

	// use last open dir from MainWin (project dir)
	KConfigGroup mainGroup = Settings::group(QStringLiteral("MainWin"));
	const QString& dir = mainGroup.readEntry("LastOpenDir", "");

	//using KFileWidget to add custom widgets
	auto* fileWidget = new KFileWidget(QUrl(dir), &dialog);
	fileWidget->setOperationMode(KFileWidget::Opening);
	fileWidget->setMode(KFile::File);

	// preview
	auto* preview = new TextPreview();
	fileWidget->setPreviewWidget(preview);

	auto filterList = QList<KFileFilter>();
	filterList << KFileFilter(i18n("LabPlot Function Definition"), {QLatin1String("*.lfd"), QLatin1String("*.LFD")}, {});
	fileWidget->setFilters(filterList);

	fileWidget->okButton()->show();
	fileWidget->okButton()->setEnabled(false);
	fileWidget->cancelButton()->show();
	QObject::connect(fileWidget->okButton(), &QPushButton::clicked, &dialog, &QDialog::accept);
	QObject::connect(fileWidget, &KFileWidget::selectionChanged, &dialog, [=]() {
		QString fileName = fileWidget->locationEdit()->currentText();
		auto currentDir = fileWidget->baseUrl().toLocalFile();
		fileName.prepend(currentDir);
		if (QFile::exists(fileName))
			fileWidget->okButton()->setEnabled(true);
	});
	QObject::connect(fileWidget->cancelButton(), &QPushButton::clicked, &dialog, &QDialog::reject);
	layout->addWidget(fileWidget);

	if (dialog.exec() == QDialog::Accepted) {
		QString fileName = fileWidget->locationEdit()->currentText();
		auto currentDir = fileWidget->baseUrl().toLocalFile();
		fileName.prepend(currentDir);

		//load config from file if accepted
		QDEBUG(Q_FUNC_INFO << ", load function from file" << fileName)

		KConfig config(fileName);
		auto general = config.group(QLatin1String("General"));
		te->setPlainText(general.readEntry("Function", ""));
		// switch to custom model
		if (cbCategory)
			cbCategory->setCurrentIndex(cbCategory->count() - 1);

		auto description = general.readEntry("Description", "");
		auto comment = general.readEntry("Comment", "");
		QDEBUG(Q_FUNC_INFO << ", description:" << description)
		QDEBUG(Q_FUNC_INFO << ", comment:" << comment)
		if (cbModel) {	// model ComboBox available (FitCurveDock style)
			if (!description.isEmpty()) {
				cbModel->clear();
				cbModel->addItem(description);
			}
			if (!comment.isEmpty())
				te->viewport()->setToolTip(comment);
		} else {	// normal style (FunctionValuesDialog)
			if (!description.isEmpty())
				te->viewport()->setToolTip(description);
			if (!comment.isEmpty())
				te->viewport()->setWhatsThis(comment);
		}
	}
}

/*
 * save custom function to file
 * using TextEdit. Only FitCurve used cb
 */
void GuiTools::saveFunction(ExpressionTextEdit* te, KComboBox* cb) {
	QDialog dialog;
	dialog.setWindowTitle(i18n("Select file to save function definition"));
	auto* layout = new QVBoxLayout(&dialog);

	// use last open dir from MainWin (project dir)
	KConfigGroup mainGroup = Settings::group(QStringLiteral("MainWin"));
	const QString& dir = mainGroup.readEntry("LastOpenDir", "");

	//using KFileWidget to add custom widgets
	auto* fileWidget = new KFileWidget(QUrl(dir), &dialog);
	fileWidget->setOperationMode(KFileWidget::Saving);
	fileWidget->setMode(KFile::File);
	// preview
	auto* preview = new TextPreview();
	fileWidget->setPreviewWidget(preview);

	auto filterList = QList<KFileFilter>();
	filterList << KFileFilter(i18n("LabPlot Function Definition"), {QLatin1String("*.lfd"), QLatin1String("*.LFD")}, {});
	fileWidget->setFilters(filterList);

	fileWidget->okButton()->show();
	fileWidget->okButton()->setEnabled(false);
	fileWidget->cancelButton()->show();
	QObject::connect(fileWidget->okButton(), &QPushButton::clicked, &dialog, &QDialog::accept);
	QObject::connect(fileWidget->cancelButton(), &QPushButton::clicked, &dialog, &QDialog::reject);
	layout->addWidget(fileWidget);

	// custom widgets
	auto* lDescription = new QLabel(i18n("Description:"));
	auto* lComment = new QLabel(i18n("Comment:"));
	// normal style (FunctionValuesDialog)
	auto* leDescription = new KLineEdit(te->viewport()->toolTip());
	auto* leComment = new KLineEdit(te->viewport()->whatsThis());
	if (cb) {	// FitCurveDock style
		leDescription->setText(cb->currentText());
		leComment->setText(te->viewport()->toolTip());
	}

	// update description and comment when selection changes
	QObject::connect(fileWidget, &KFileWidget::fileHighlighted, fileWidget, [=]() {
		QString fileName = fileWidget->locationEdit()->currentText();
		auto currentDir = fileWidget->baseUrl().toLocalFile();
		fileName.prepend(currentDir);
		QDEBUG(Q_FUNC_INFO << ", file selected:" << fileName)
		if (QFile::exists(fileName)) {
			KConfig config(fileName);
			auto group = config.group(QLatin1String("General"));
			const QString& description = group.readEntry("Description", "");
			const QString& comment = group.readEntry("Comment", "");
			if (!description.isEmpty())
				leDescription->setText(description);
			if (!comment.isEmpty())
				leComment->setText(comment);
		}
	});

	auto* grid = new QGridLayout;
	grid->addWidget(lDescription, 0, 0);
	grid->addWidget(leDescription, 0, 1);
	grid->addWidget(lComment, 1, 0);
	grid->addWidget(leComment, 1, 1);
	layout->addLayout(grid);

	dialog.adjustSize();
	if (dialog.exec() == QDialog::Accepted) {
		fileWidget->slotOk();

		QString fileName = fileWidget->selectedFile();
		if (fileName.isEmpty()) {	// if entered directly and not selected (also happens when selected!)
			// DEBUG(Q_FUNC_INFO << ", no file selected")
			fileName = fileWidget->locationEdit()->currentText();
			auto* cbExtension = fileWidget->findChild<QCheckBox*>();
			if (cbExtension) {
				bool checked = cbExtension->isChecked();
				if (checked && ! (fileName.endsWith(QLatin1String(".lfd")) || fileName.endsWith(QLatin1String(".LFD"))))
							fileName.append(QLatin1String(".lfd"));
			}
			// add current folder
			auto currentDir = fileWidget->baseUrl().toLocalFile();
			fileName.prepend(currentDir);
		}
		// save current model (with description and comment)
		// FORMAT: LFD - LabPlot Function Definition
		KConfig config(fileName);	// selected lfd file
		auto group = config.group(QLatin1String("General"));
		auto description = leDescription->text();
		auto comment = leComment->text();
		group.writeEntry("Function", te->toPlainText());	// model function
		group.writeEntry("Description", description);
		group.writeEntry("Comment", comment);
		config.sync();
		QDEBUG(Q_FUNC_INFO << ", saved function to" << fileName)

		// set description and comment in Dock (even when empty)
		if (cb) {	// FitCurveDock style
			cb->clear();
			cb->addItem(description);
			te->viewport()->setToolTip(comment);
		} else {	// normal style (FunctionValuesDialog)
			te->viewport()->setToolTip(description);
			te->viewport()->setWhatsThis(comment);
		}
	}
}
