#include "WorkpieceIdentification.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	IGXFactory::GetInstance().Init();//new for test
	QApplication a(argc, argv);
	WorkpieceIdentification w;
	w.show();
	return a.exec();
}
