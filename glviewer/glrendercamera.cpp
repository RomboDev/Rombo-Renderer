/*
 * glrendercamera.cpp
 *
 *  Created on: Jan 18, 2013
 *      Author: max
 */


#include <glviewer/glrendercamera.h>

//***********************************************************************************************************//
// Render Camera ********************************************************************************************//
//***********************************************************************************************************//
RenderCamera::RenderCamera (GLViewer *widget)
: m_mouseMode (0)
, m_clickX (0)
, m_clickY (0)
, isMoving (false)

//, QObject (widget)
{
    m_widget = widget;
    //widget->installEventFilter(this);
	registerDevice ();
}

void RenderCamera::registerDevice ()
{
    //install event filter on host widget
	m_widget->installEventFilter(this);

	//connect (m_widget, 	SIGNAL (init()),
	//		this, 		SLOT (viewerInit()) );
	connect (m_widget, 	SIGNAL (resized()),
			this, 		SLOT (viewerResized()) );
	connect (m_widget, 	SIGNAL (painting(QPainter*)),
			this, 		SLOT (viewerPaint(QPainter*)) );
}

void RenderCamera::setWidget (GLViewer *widget)
{
    m_widget = widget;
    widget->installEventFilter(this);

	m_mouseMode = 0;
	m_clickX = 0;
	m_clickY = 0;

	isMoving = false;
}

bool RenderCamera::eventFilter (QObject *object, QEvent *event)
{
    if (object == m_widget && !m_widget->isRenderRegion() && !m_widget->renderIsStopped())
    {
        switch (event->type())
        {

        case QEvent::KeyPress:
        {
			QKeyEvent *e = (QKeyEvent *) event;

			//!< Camera focus increase
			if(e->key()==Qt::Key_L && e->modifiers()==Qt::ShiftModifier)
			{
				m_widget->incRenderCameraRadius();
				m_widget->updateRenderCamera();

				m_widget->resetAccumulation();
			}
			//!< Camera focus decrease
			else if(e->key()==Qt::Key_L)
			{
				float tCR = std::max(0.0f, m_widget->getRenderCameraRadius()-1.f);
				m_widget->setRenderCameraRadius(tCR);
				m_widget->updateRenderCamera();

				m_widget->resetAccumulation();
			}
			//!> Camera transform, print to console //////////////////////////////////
			else if(e->key()==Qt::Key_C)
			{
				m_widget->printRenderCameraTransformStr();
			}
        } break;

#ifndef GFXVIEW
        case QEvent::MouseButtonPress:
#else
        case QEvent::GraphicsSceneMousePress:
#endif
        {
        	if(this->isMoving) return false;

#ifndef GFXVIEW
        	QMouseEvent *e = (QMouseEvent *) event;
        	int x = e->pos().x() - m_widget->getOffsetWidth();
        	int y = e->pos().y() - m_widget->getOffsetHeight();
        	//std::cout << "x: " << x << ", y: " << y << std::endl;
#else
        	QGraphicsSceneMouseEvent *e = (QGraphicsSceneMouseEvent *) event;
        	int x = e->scenePos().x();
        	int y = e->scenePos().y();
#endif

        	//!< Camera mouse mode //////////////////////////////////////////////////
        	m_mouseMode = 0;	   //!< reset camera mouse mode

        	if( e->button()==Qt::LeftButton && e->modifiers()==(Qt::ControlModifier | Qt::ShiftModifier) )
        	{
        		//!< center camera
        		m_widget->focusRenderCamera (x,y);
        		m_widget->resetAccumulation();
        	}
        	else
        	if( e->button()==Qt::LeftButton && e->modifiers()==Qt::ControlModifier )
        	{
        		//!< focus camera
        		m_widget->centerRenderCamera (x,y);
        		m_widget->resetAccumulation();
        	}
            else
        	{
            	//!< Rotate-1, zoom-2, pan-3, roll-4
        		switch( e->button() )
        		{
        		case Qt::LeftButton:
        			if(e->modifiers()==Qt::ShiftModifier)
        								m_mouseMode = 4;
        			else 				m_mouseMode = 1; break;
        		case Qt::MidButton: 	m_mouseMode = 2; break;
        		case Qt::RightButton: 	m_mouseMode = 3; break;
        		case Qt::NoButton:
        		case Qt::XButton1:
        		case Qt::XButton2:
        		case Qt::MouseButtonMask: break;
        		}

        		if(m_mouseMode>0)
        		{
        			this->isMoving = true;

        			//track last mouse postion ...............
        			//to update camera move at mousemove event
        			#ifdef RIGHTHANDLEDCOORDSYS
        			m_clickX = m_widget->getWidgetWidth() - x;
					#else
        			m_clickX = x;
        			#endif
        			m_clickY = y;
        		}
        	}
        } break;

#ifndef GFXVIEW
        case QEvent::MouseMove:
#else
        case QEvent::GraphicsSceneMouseMove:
#endif
        {
        	if(!this->isMoving)return false;

#ifndef GFXVIEW
        	QMouseEvent *e = (QMouseEvent *) event;
        	int x = e->pos().x() - m_widget->getOffsetWidth();
        	int y = e->pos().y() - m_widget->getOffsetHeight();
#else
        	QGraphicsSceneMouseEvent *e = (QGraphicsSceneMouseEvent *) event;
        	int x = e->scenePos().x();
        	int y = e->scenePos().y();
#endif
        	m_widget->updateRenderCameraMove( m_mouseMode, x, y, m_clickX, m_clickY, m_widget->getWidgetWidth() );
    		m_widget->resetAccumulation();

    		//make lastpos the current one
    		//removing this will make mouse moves kinda 'countinous'
			#ifdef RIGHTHANDLEDCOORDSYS
			m_clickX = m_widget->getWidgetWidth() - x; //TODO::CHECK HERE TO MOVE CAM IN RREGION
			#else
			m_clickX = x;
			#endif
			m_clickY = y;
        } break;

#ifndef GFXVIEW
        case QEvent::MouseButtonRelease:
#else
        case QEvent::GraphicsSceneMouseRelease:
#endif
        {
        	this->isMoving = false;
        } break;
        }
    }
	return false;
}


