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

class RenderGLView : public QGraphicsView
{

public:
	RenderGLView():mScene(NULL) {}

	void setGLScene( QGraphicsScene* iScene )
	{
		QGLWidget * glWidget = new QGLWidget(QGLFormat(QGL::DoubleBuffer));

		this->setViewport (glWidget);
		this->setViewportUpdateMode (QGraphicsView::FullViewportUpdate);
		this->setCacheMode (QGraphicsView::CacheNone);
		this->setOptimizationFlag (QGraphicsView::DontAdjustForAntialiasing);
		//this->setOptimizationFlag (QGraphicsView::DontSavePainterState);
		this->setStyleSheet (	"border: none; "
								"border-style: none; "
								"padding: 0px 0px 0px 0px; "
								"margin-left: -3px; "
								"margin-right: -3px; "
								"margin-top: -3px; "
								"margin-bottom: -1px;" );

		mScene = qobject_cast<GLViewer*>(iScene);

#ifdef GFXVIEW //or it won't compile in QGLWidget mode
		if(mScene)
		this->setScene( mScene );
#endif

		this->show();
	}

protected:
	void resizeEvent( QResizeEvent* e)
	{
		if( scene() )
		{
			std::cout << "QGraphicsView->resizeEvent: " << e->size().width() << ", " << e->size().height() << std::endl;
			//scene()->setSceneRect( QRect(QPoint(0,0), e->size()) );

			if(mScene->isSceneReady())
			{
				std::cout << "QGraphicsView->resizeEvent->resizeGL" << std::endl;
				mScene->resizeGL(e->size().width(),e->size().height());
			}
			else
			{
				std::cout << "QGraphicsView->resizeEvent->resize" << std::endl;
				mScene->resize(e->size().width(),e->size().height());
			}
		}

		//QGraphicsView::resizeEvent(e);
	}

	void showEvent(QShowEvent*e)
	{
		if( scene() )
		{
			std::cout << "QGraphicsView->showEvent: " << this->width() << ", " << this->height() << std::endl;
			scene()->setSceneRect( QRect(QPoint(0,0), QPoint(this->width(),this->height())) );

			if(mScene->isSceneReady()){
				std::cout << "QGraphicsView->showEvent->resizeGL" << std::endl;
				mScene->resizeGL(this->width(),this->height());
			}
			else{
				std::cout << "QGraphicsView->showEvent->resize" << std::endl;
				mScene->resize(this->width(),this->height());
			}
		}
	}

private:
	GLViewer* mScene;
};


#endif /* RENDERGLVIEW_H_ */
