/*
 * renderGLview.h
 *
 *  Created on: Nov 25, 2012
 *      Author: max
 */

#ifndef RENDERGLVIEW_H_
#define RENDERGLVIEW_H_

#include <QtGui/QGraphicsView>
#include "QtOpenGL/QGLWidget"
#include <QResizeEvent>

#include "glviewer.h"
/*
CC            = icc
CXX           = icpc
DEFINES       = -DQT_WEBKIT -DQT_OPENGL_LIB -DQT_GUI_LIB -DQT_CORE_LIB -DQT_SHARED
CFLAGS        = -m64 -pipe -g -Wall -W -D_REENTRANT $(DEFINES)
CXXFLAGS      = -m64 -pipe -g -Wall -W -D_REENTRANT $(DEFINES)
INCPATH       = -I/usr/share/qt4/mkspecs/default -I. -I/usr/include/QtCore -I/usr/include/QtGui -I/usr/include/QtOpenGL -I/usr/include -I/usr/X11R6/include -Idebug -I. -I/home/max/git/embree/embree -I/home/max/git/embree/examples/common  -I/home/max/git/embree/examples/renderer
LINK          = icpc
LFLAGS        = -m64
LIBS          = $(SUBLIBS)  -L/usr/lib64 -L/usr/X11R6/lib64 -lQtOpenGL -L/usr/lib64 -L/usr/X11R6/lib64 -L/home/max/git/embree/examples/renderer/device/Release -L/home/max/git/embree/examples/common/image/Release -L/home/max/git/embree/examples/common/lexers/Release -L/home/max/git/embree/embree/sys/Release -lQtGui -lQtCore -lGL -lpthread -lembree_device -lembree_image -lembree_lexers -lembree_sys
AR            = ar cqs
*/

/*
CC            = icc
CXX           = icpc
DEFINES       = -DQT_WEBKIT -DQT_NO_DEBUG -DQT_OPENGL_LIB -DQT_GUI_LIB -DQT_CORE_LIB -DQT_SHARED
CFLAGS        = -m64 -pipe -fno-rtti -DNORTTI -O3 -axAVX -xAVX -Wall -W -D_REENTRANT $(DEFINES)
CXXFLAGS      = -m64 -pipe -fno-rtti -DNORTTI -O3 -axAVX -xAVX -Wall -W -D_REENTRANT $(DEFINES)
INCPATH       = -I/usr/share/qt4/mkspecs/default -I. -I/usr/include/QtCore -I/usr/include/QtGui -I/usr/include/QtOpenGL -I/usr/include -I/usr/X11R6/include -Irelease -I. -I/home/max/git/embree/embree -I/home/max/git/embree/examples/common  -I/home/max/git/embree/examples/renderer
LINK          = icpc
LFLAGS        = -m64 -Wl,-O3 -static-intel
LIBS          = $(SUBLIBS)  -L/usr/lib64 -L/usr/X11R6/lib64 -L/usr/lib64 -L/usr/X11R6/lib64 -L/home/max/git/embree/examples/renderer/device/Release -L/home/max/git/embree/examples/common/image/Release -L/home/max/git/embree/examples/common/lexers/Release -L/home/max/git/embree/embree/sys/Release -lQtGui -lQtCore -lQtOpenGL -lGL -lpthread -lembree_device -lembree_image -lembree_lexers -lembree_sys
AR            = ar cqs
*/

class RenderGLView : public QGraphicsView
{
	Q_OBJECT
	friend GLViewer;

public:
	RenderGLView();

	void setGLScene( QGraphicsScene* iScene );
	void setGLScene();
	void setGLScenePointer( QGraphicsScene* iScene );

public slots:
	void doresizing ()
	{
		std::cout << "glmanager force resize slots called: " << mScene->getWidgetWidth() << ", " << mScene->getWidgetHeight() << std::endl;

		if(scene())
		{
			//invalidateScene();
			//scene()->update();

			QSize tempSize (mScene->getWidgetWidth(), mScene->getWidgetHeight());
			this->resizeEvent(new QResizeEvent(tempSize,tempSize));
		}else
		{
			std::cout << "glmanager setting scene: " << std::endl;
			setGLScene();
		}
	}

protected:
	void resizeEvent( QResizeEvent* e);
	void showEvent(QShowEvent*e);

private:
	GLViewer* mScene;
};


#endif /* RENDERGLVIEW_H_ */
