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
	RenderGLView(){}

	void setGLScene( QGraphicsScene* iScene )
	{
		QGLWidget * glWidget = new QGLWidget(QGLFormat(QGL::DoubleBuffer));
		glWidget->makeCurrent();

		this->setViewport( glWidget );
		this->setViewportUpdateMode (QGraphicsView::FullViewportUpdate);
		//this->setOptimizationFlag (QGraphicsView::DontAdjustForAntialiasing);

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
			this->fitInView( QRect(QPoint(0,0), e->size()) );

			scene()->setSceneRect( QRect(QPoint(0,0), e->size()));

			if(mScene->isSceneReady())
			{
				mScene->resizeGL(e->size().width(),e->size().height());
			}
			else
			{
				mScene->resize(e->size().width(),e->size().height());
			}
		}

		QGraphicsView::resizeEvent(e);
	}

	void showEvent(QShowEvent*e)
	{
		if( scene() )
		{
			scene()->setSceneRect( QRect(QPoint(0,0), QPoint(this->width(),this->height())) );

			if(mScene->isSceneReady()){
				mScene->resizeGL(this->width(),this->height());
			}
			else{
				mScene->resize(this->width(),this->height());
			}
		}
	}

private:
	GLViewer* mScene;
};


#endif /* RENDERGLVIEW_H_ */
