#include "rombo.h"

#include <QtGui>
#include <QtGui/QApplication>

int main(int argc, char *argv[])
{
	//QGL::setPreferredPaintEngine(QPaintEngine::OpenGL);

    QApplication app(argc, argv);
	//app.setPalette( MainWindow::getMyPalette() );
	app.setStyleSheet(	"QMenu::separator{ background-color: #444444; height: 1px; margin-left:12px; margin-right: 12px; }"
						"QMenu::item{ color: #CCCCCC; background-color: #545454; padding: 6px 28px 6px 26px; border: none; border-color: #545454}"
						"QMenu::item:selected{ color: white; background-color: #545454; padding: 4px 26px 4px 24px; border: 0px solid yellow; border-color: #545454; }" );


    QSplashScreen *splash = new QSplashScreen;
    splash->setPixmap(QPixmap(":/images/splash.png"));
    splash->show();
    Qt::Alignment topRight = Qt::AlignRight | Qt::AlignTop;
    splash->showMessage(QObject::tr("Setting up application ..."),
                        topRight, Qt::white);

	MainWindow win( argc, argv );
    splash->showMessage(QObject::tr("Initializing renderer ..."),
                        topRight, Qt::white);

    splash->finish(&win);
    delete splash;

    win.show();
    return app.exec();
}
