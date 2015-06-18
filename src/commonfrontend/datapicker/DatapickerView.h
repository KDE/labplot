#ifndef DATAPICKERVIEW_H
#define DATAPICKERVIEW_H

#include <QWidget>
#include <QTabBar>

class AbstractAspect;
class Datapicker;
class QAction;
class QMenu;
class QPrinter;
class QToolBar;
class TabWidget;


class DatapickerView : public QWidget {
    Q_OBJECT

    public:
        explicit DatapickerView(Datapicker*);
        virtual ~DatapickerView();

        int currentIndex() const;

    private:
        TabWidget* m_tabWidget;
        Datapicker* m_datapicker;
        int lastSelectedIndex;

    private  slots:
        void showTabContextMenu(const QPoint&);
        void itemSelected(int);
        void tabChanged(int);
        void tabMoved(int,int);
        void handleAspectAdded(const AbstractAspect*);
        void handleAspectAboutToBeRemoved(const AbstractAspect*);

};

#endif
