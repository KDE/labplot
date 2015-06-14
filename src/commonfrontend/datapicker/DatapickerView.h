#ifndef DATAPICKERVIEW_H
#define DATAPICKERVIEW_H

#include <QWidget>

class AbstractAspect;
class Datapicker;
class QAction;
class QMenu;
class QPrinter;
class QTabWidget;
class QToolBar;

class DatapickerView : public QWidget {
    Q_OBJECT

	public:
		explicit DatapickerView(Datapicker*);
		int currentIndex() const;

	private:
		QTabWidget* m_tabWidget;
		Datapicker* m_datapicker;
		int lastSelectedIndex;


	private  slots:
		void showTabContextMenu(const QPoint&);
		void addSpreadsheet();
        void addWorksheet();
		void tabChanged(int);
		void tabMoved(int,int);
		void handleAspectAdded(const AbstractAspect*);
		void handleAspectAboutToBeRemoved(const AbstractAspect*);

};

#endif
