#include "TexRenderer.h"

#include <KTempDir>
#include <KProcess>
#include <KDebug>
#include <QDir>


//TODO make this function using Qt only?
bool TexRenderer::createImage( const QString& texString, QImage& image){
	kDebug()<<""<<endl;
	KTempDir *tmpDir = new KTempDir();
	QString dirName = tmpDir->name();
// 	kDebug()<<"temporary directory "<<dirName<<" is used"<<endl;

	KProcess *proc = new KProcess;
	*proc<<"texvc";
	*proc<<"/tmp"<<dirName<<texString;

	int exitCode=proc->execute();
	kDebug()<<"texvc's exit code "<<exitCode<<endl;
	if( exitCode==-2 ) {
		kDebug()<<"Couldn't find texvc."<<endl;
		return false;
	}else if (exitCode==-1){
		kDebug()<<"Texvc crashed."<<endl;
		return false;
	}

	// take resulting image and show it
 	QDir d(dirName);
 	if (d.count()!=3){
 		kDebug()<<"No file created. Check the syntax."<<endl;
 		return false;

	}

 	QString fileName = dirName+QString(d[2]);
// 	kDebug()<<"file name "<<fileName<<" is used"<<endl;
	if ( !image.load(fileName) ){
		kDebug()<<"Error on loading the tex-image"<<endl;
		tmpDir->unlink();
		return false;
	}

 	tmpDir->unlink();
	kDebug()<<"image created."<<endl;
	return true;
}
