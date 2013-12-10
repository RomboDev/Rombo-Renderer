/*
 * glmanager.cpp
 *
 *  Created on: Dec 7, 2013
 *      Author: max
 */

#include "glmanager.h"
RenderGLView::RenderGLView():mScene(NULL) {}

void RenderGLView::setGLScene( QGraphicsScene* iScene )
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
void RenderGLView::setGLScene()
{
	QGLWidget * glWidget = new QGLWidget(QGLFormat(QGL::DoubleBuffer));
	glWidget->makeCurrent();

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

#ifdef GFXVIEW //or it won't compile in QGLWidget mode
	if(mScene)
	this->setScene( mScene );
#endif

	scene()->setSceneRect( QRect(QPoint(0,0), QSize(this->width(), this->height())) );
	this->show();
}

void RenderGLView::setGLScenePointer( QGraphicsScene* iScene )
{
	mScene = qobject_cast<GLViewer*>(iScene);
}

void RenderGLView::resizeEvent( QResizeEvent* e)
{
	if( scene() )
	{
		std::cout << "QGraphicsView->resizeEvent: " << e->size().width() << ", " << e->size().height() << std::endl;
		scene()->setSceneRect( QRect(QPoint(0,0), e->size()) );

#ifdef GFXVIEW //or it won't compile in QGLWidget mode
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
#endif
	}

	QGraphicsView::resizeEvent(e);
}
void RenderGLView::showEvent(QShowEvent*e)
{
	if( scene() )
	{
		std::cout << "QGraphicsView->showEvent: " << this->width() << ", " << this->height() << std::endl;
		scene()->setSceneRect( QRect(QPoint(0,0), QPoint(this->width(),this->height())) );

#ifdef GFXVIEW //or it won't compile in QGLWidget mode
		if(mScene->isSceneReady()){
			std::cout << "QGraphicsView->showEvent->resizeGL" << std::endl;
			mScene->resizeGL(this->width(),this->height());
		}
		else{
			std::cout << "QGraphicsView->showEvent->resize" << std::endl;
			mScene->resize(this->width(),this->height());
		}
#endif
	}
}
