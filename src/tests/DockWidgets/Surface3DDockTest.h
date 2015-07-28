#ifndef SURFACE3DDOCKTEST_H
#define SURFACE3DDOCKTEST_H

#include <QObject>

class Surface3DDockTest : public QObject {
		Q_OBJECT
	protected slots:
		void onElementVisibilityChanged();

	private slots:
		void test_colorFillingTypeChangedCall();

	private:
		int visibilityCounter;
};

#endif