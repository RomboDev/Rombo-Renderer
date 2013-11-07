/*
 * renderregion.cpp
 *
 *  Created on: Dec 20, 2012
 *      Author: max
 */

#include"glrenderregion.h"



//***********************************************************************************************************//
// RenderRegion	Divider *************************************************************************************//
//***********************************************************************************************************//
RRDivider::RRDivider(RenderRegion *rregion)
: m_isMovingDivider (false)
, m_rr_divpixels (0)
{
	m_rregion = rregion;
	m_rregion->getHostWidget()->installEventFilter(this);
}

void RRDivider::setDivider(const QRectF &rr_bbox, bool isReset, const QPointF& offset)
{
	if(isReset || (offset.x()==-666 && offset.y()==-666)){
		//get half rregion x point
		int x = rr_bbox.left() + (rr_bbox.width()/2);

		//build up a qline
		m_rr_divline.setP1 ( QPointF(x, rr_bbox.top() +2 ));
		m_rr_divline.setP2 ( QPointF(x, rr_bbox.bottom() ));
		m_rr_linecenter = QPointF(m_rr_divline.p1().x(),m_rr_divline.p1().y()+m_rr_divline.length()/2);

		//set initial div at half rregion
		m_rr_divpixels = abs(m_rregion->getBBoxRect().width()) /2;
	}
	else
	{
		//update line and center with offset
		//so we maintain the user position
		QPointF oP1 (m_rr_divline.p1());
		QPointF oP2 (m_rr_divline.p2());
		oP1.setX (oP1.x() - offset.x());
		oP1.setY (oP1.y() - offset.y());
		oP2.setX (oP2.x() - offset.x());
		oP2.setY (oP2.y() - offset.y());
		m_rr_divline.setPoints (oP1,oP2);

		m_rr_linecenter.setX (m_rr_linecenter.x() - offset.x());
		m_rr_linecenter.setY (m_rr_linecenter.y() - offset.y());

		//m_rr_divpixels -= offset.x(); //that'd be for the viewer to display cropped fb
										//but we already update there
	}
}

void RRDivider::paint(QPainter* iPainter)
{
	if( !m_rregion->hasActiveSlot() ) return;

	//hide divider while drawing or resizing rregion
	if( m_rregion->isMovingPoints() ||
		m_rregion->isDragging() ||
		m_rregion->isDrawingPoints()
	  ) return;

	iPainter->setPen( QColor(255,255,0) );
	iPainter->drawLine (m_rr_divline);

	if(m_rregion->isOverBBox())
	{
		QRectF pRect;
		int pLenght = 3;
		pRect.setTopLeft (QPointF(m_rr_linecenter.x()-1-pLenght, m_rr_linecenter.y()-pLenght));
		pRect.setBottomRight (QPointF(m_rr_linecenter.x()-1+pLenght, m_rr_linecenter.y()+pLenght));

		/*QPen pen;
		pen.setColor( QColor(255,255,0) );
		pen.setWidth(3);
		iPainter->setPen( pen );*/

		iPainter->drawRect(pRect);

		if(m_isMovingDivider)
		iPainter->fillRect(pRect, QColor(200,200,0));
	}
}

bool RRDivider::eventFilter(QObject *object, QEvent *event)
{
    if (object == m_rregion->getHostWidget() && m_rregion->isActive()
    		&& m_rregion->isOverBBox() && m_rregion->hasActiveSlot())
    {
        switch (event->type())
        {

#ifndef GFXVIEW
        case QEvent::MouseButtonPress:
#else
        case QEvent::GraphicsSceneMousePress:
#endif
        {
#ifndef GFXVIEW
        	QMouseEvent *me = (QMouseEvent *) event;
        	int x = me->pos().x();
        	int y = me->pos().y();
#else
        	QGraphicsSceneMouseEvent *me = (QGraphicsSceneMouseEvent *) event;
        	int x = me->scenePos().x();
        	int y = me->scenePos().y();
#endif

        	int m_pointSize = 8;
			qreal d = QLineF(me->pos(), m_rr_linecenter).length();

			if (d < m_pointSize)
			{
				m_isMovingDivider = true;
				m_rregion->setIsMovingDivider(true);
			}

        } break;

#ifndef GFXVIEW
        case QEvent::MouseMove:
#else
        case QEvent::GraphicsSceneMouseMove:
#endif
        {
        	if(m_isMovingDivider)
        	{
#ifndef GFXVIEW
				QMouseEvent *me = (QMouseEvent *) event;
				int x = me->pos().x();
				int y = me->pos().y();
#else
				QGraphicsSceneMouseEvent *me = (QGraphicsSceneMouseEvent *) event;
				int x = me->scenePos().x();
				int y = me->scenePos().y();
#endif

				if(x>m_rregion->getBBoxRect().left()) x=m_rregion->getBBoxRect().left();
				if(x<m_rregion->getBBoxRect().right()) x=m_rregion->getBBoxRect().right()+1;

				//re set divline to new pos
				qreal y_org = m_rr_divline.p1().y();
				m_rr_divline.setP1 ( QPointF(x, m_rr_divline.p1().y()));
				m_rr_divline.setP2 ( QPointF(x, m_rr_divline.p2().y()));
				m_rr_linecenter = QPointF(x, m_rr_linecenter.y());

				//get nb pixels from right to divline
				m_rr_divpixels =  abs(m_rregion->getBBoxRect().width()) - abs(m_rregion->getBBoxRect().right() - x);
				if(m_rr_divpixels<0)m_rr_divpixels=0;
        	}
        } break;

        case QEvent::MouseButtonRelease:
        {
			m_isMovingDivider = false;
			m_rregion->setIsMovingDivider(false);

        } break;
        }
    }
    return false;
}


//***********************************************************************************************************//
// RenderRegion	FB Slot Points ******************************************************************************//
//***********************************************************************************************************//
RRSlotPoints::RRSlotPoints(RenderRegion *rregion)
: m_isTweakingSlots (false)
, m_activePoint (-1)
{
	m_rregion = rregion;
	m_rregion->getHostWidget()->installEventFilter(this);
}

void RRSlotPoints::setPoints(const QRectF &rr_bbox, bool isReset, const QPointF& offset )
{
	if(isReset)
	{
		for (int i=0; i<m_slots.size(); ++i)
			if(m_slots.at(i))
				delete m_slots.at(i);

		m_slots.clear();

		QPointF topLeft (rr_bbox.topRight().x()+12, rr_bbox.topRight().y()+12);

		m_slots.push_back (new RRSlot (topLeft));
		m_slots.push_back (new RRSlot (QPointF(topLeft.x()+16, topLeft.y())));
		m_slots.push_back (new RRSlot (QPointF(topLeft.x()+32, topLeft.y())));
		m_slots.push_back (new RRSlot (QPointF(topLeft.x()+48, topLeft.y())));

		//set reset slots point
		m_reset_slots = QPointF(topLeft.x()+64, topLeft.y());

		m_activePoint = -1;
	}
	else
	{
		//this may work also instead of using the offset
		//we still have it because the divider needs it to
		//record its actual position, so we leave it here too
		if(offset.x()==-666 && offset.y()==-666)
		{
			QPointF topLeft (rr_bbox.topRight().x()+12, rr_bbox.topRight().y()+12);
			for (int i=0; i<m_slots.size(); ++i)
			{
				if(m_slots.at(i))
				{
					m_slots[i]->center = QPointF (topLeft.x() +16*i, topLeft.y() );
				}
			}
			//set reset slots point
			m_reset_slots = QPointF(topLeft.x()+64, topLeft.y());
		}
		else
		{
			//just update the geo center to be able to redraw correctly
			for (int i=0; i<m_slots.size(); ++i)
			{
				if(m_slots.at(i))
				{
					QPointF oC (	(int)m_slots[i]->center.x(),
								(int)m_slots[i]->center.y() );
					oC.setX (oC.x() - offset.x());
					oC.setY (oC.y() - offset.y());
					m_slots[i]->center = oC;
				}
			}

			//reset slot too
			m_reset_slots.setX (m_reset_slots.x() - offset.x());
			m_reset_slots.setY (m_reset_slots.y() - offset.y());
		}
	}
}

void RRSlotPoints::paint(QPainter* iPainter)
{
	//hide slots while drawing or resizing rregion
	if( m_rregion->isMovingPoints() ||
		m_rregion->isDragging() ||
		m_rregion->isDrawingPoints() ||
		!m_rregion->isOverBBox() ) return;

	iPainter->setRenderHint(QPainter::Antialiasing);

	int pLenght = 4;
	QRectF pRect; QPointF hPoint;

	QColor defActiveCol (255, 255, 0, 250);
	QColor defFillCol (200, 200, 0, 180);
	QColor defDrawCol (255, 255, 0, 200);

	for (int i=0; i<m_slots.size(); ++i)
	{
		hPoint = m_slots.at(i)->center;
		pRect.setTopLeft (QPointF(hPoint.x()-pLenght, hPoint.y()-pLenght));
		pRect.setBottomRight (QPointF(hPoint.x()+pLenght, hPoint.y()+pLenght));

		if(m_slots.at(i)->state==RRSlot::FILLED)
			//iPainter->fillRect(pRect, defFillCol);
			iPainter->setBrush(defFillCol);
		if(i==m_activePoint)
			//iPainter->fillRect(pRect, defActiveCol);
			iPainter->setBrush(defActiveCol);

		QPen pen;
		pen.setColor( defDrawCol );
		pen.setWidth(2);
		iPainter->setPen( pen );
		//iPainter->setPen(defDrawCol);
		//iPainter->drawRect(pRect);
		iPainter->drawEllipse(pRect.center(),4,4);
		iPainter->setBrush(Qt::NoBrush);
	}

	//paint reset slots point
	if(m_rregion->isBlocked()) //that also means we have something to reset
	{
		hPoint = m_reset_slots;
		pRect.setTopLeft (QPointF(hPoint.x()-pLenght, hPoint.y()-pLenght));
		pRect.setBottomRight (QPointF(hPoint.x()+pLenght, hPoint.y()+pLenght));

		//iPainter->setPen(QColor(255,0,0,255));
		//iPainter->drawRect(pRect);
		QPen pen;
		pen.setColor( QColor(255,0,0,250) );
		pen.setWidth(2);
		iPainter->setPen( pen );
		iPainter->setBrush(QColor(130,0,0,220));
		iPainter->drawEllipse(pRect.center(),4,4);
		iPainter->setBrush(Qt::NoBrush);
	}
}

void RRSlotPoints::clearSlots()
{
	for (int i=0; i<m_slots.size(); ++i)
	{
		m_slots.at(i)->state = RRSlot::EMPTY;
		m_rregion->emit rrSlotHasChanged (i, RRSlot::EMPTY);
	}
	m_activePoint = -1;
	m_rregion->setIsBlocked (false);	//!< Unlock RRegion
}

bool RRSlotPoints::eventFilter(QObject *object, QEvent *event)
{
    if (object == m_rregion->getHostWidget() && m_rregion->isActive() && m_rregion->isOverBBox() )
    {
        switch (event->type())
        {

/*        case QEvent::MouseButtonDblClick:
        {
        	std::cout << "DOUBLE CLICK" << std::endl;
        } break;
*/

#ifndef GFXVIEW
        case QEvent::MouseButtonPress:
#else
        case QEvent::GraphicsSceneMousePress:
#endif
        {
#ifndef GFXVIEW
        	QMouseEvent *me = (QMouseEvent *) event;
        	int x = me->pos().x();
        	int y = me->pos().y();
#else
        	QGraphicsSceneMouseEvent *me = (QGraphicsSceneMouseEvent *) event;
        	int x = me->scenePos().x();
        	int y = me->scenePos().y();
#endif

        	int m_slotSize = 8;
            qreal distance = -1;

            //!< LEFTBUTTON
        	if(me->button() == Qt::LeftButton)
        	{
				for (int i=0; i<m_slots.size(); ++i)
				{
					qreal d = QLineF(me->pos(), m_slots.at(i)->center).length();
					if ((distance < 0 && d < m_slotSize) || d < distance)
					{
						m_isTweakingSlots = true;
			        	m_rregion->setIsTweakingSlots (true);

						distance = d;
						m_activePoint = i;

						//if EMPTY it's gonna be FILLED (and automatically ACTIVE)
						if(m_slots.at(i)->state==RRSlot::EMPTY)
						{
							for(int x=0;x<m_slots.size();x++)
								if(x!=m_activePoint)
									if(m_slots[x]->state==RRSlot::ACTIVE)
										m_slots[x]->state = RRSlot::FILLED;

							m_slots.at(i)->state = RRSlot::ACTIVE;
							m_rregion->emit rrSlotHasChanged (i, RRSlot::FILLED);

							m_rregion->setIsBlocked (true);	//!< Block RRegion
						}
						//if already FILLED it's gonna be the ACTIVE one
						else if (m_slots.at(i)->state == RRSlot::FILLED)
						{
							for(int x=0;x<m_slots.size();x++)
								if(x!=m_activePoint)
									if(m_slots[x]->state==RRSlot::ACTIVE)
										m_slots[x]->state = RRSlot::FILLED;

							m_slots.at(i)->state = RRSlot::ACTIVE;
							m_rregion->emit rrSlotHasChanged (i, RRSlot::ACTIVE);
						}
						//if already ACTIVE it's gonna be IN-ACTIVE
						else if (m_slots.at(i)->state == RRSlot::ACTIVE)
						{
							m_slots.at(i)->state = RRSlot::FILLED;
							m_rregion->emit rrSlotHasChanged (i, RRSlot::INACTIVE);
							m_activePoint = -1;
						}

					}
				}

				//check reset slots point
				if(m_rregion->isBlocked())
				{
					qreal d = QLineF(me->pos(), m_reset_slots).length();
					if (d < m_slotSize)
					{
						this->clearSlots();
						/*
						for (int i=0; i<m_slots.size(); ++i)
						{
							m_slots.at(i)->state = RRSlot::EMPTY;
							m_rregion->emit rrSlotHasChanged (i, RRSlot::EMPTY);
						}
						m_activePoint = -1;
						m_rregion->setIsBlocked (false);	//!< Unlock RRegion */
					}
				}
        	}

        	//!< MID BUTTON
        	if(me->button() == Qt::MidButton)
        	{
        		bool isBlocked = false;
				for (int i=0; i<m_slots.size(); ++i)
				{
					qreal d = QLineF(me->pos(), m_slots.at(i)->center).length();
					if ((distance < 0 && d < m_slotSize) || d < distance)
					{
						m_isTweakingSlots = true;
			        	m_rregion->setIsTweakingSlots (true);

						distance = d;

						//whatever the slot was set to, it is now EMPTY
						if(m_activePoint==i)
							m_activePoint = -1; //if active none will be then

						m_slots.at(i)->state = RRSlot::EMPTY;
						m_rregion->emit rrSlotHasChanged (i, RRSlot::EMPTY);
					}

					//while we're here, check also if any filled slot
					//if none free the rregion lock
					if(m_slots.at(i)->state != RRSlot::EMPTY)
						isBlocked = true;
				}

				m_rregion->setIsBlocked (isBlocked);	//!< (maybe) Unlock RRegion
        	}
        } break;

#ifndef GFXVIEW
        case QEvent::MouseButtonRelease:
#else
        case QEvent::GraphicsSceneMouseRelease:
#endif
        {
        	if(m_isTweakingSlots)
        	{
        		m_isTweakingSlots = false;
        		m_rregion->setIsTweakingSlots (false);
        	}
        } break;
        }
    }
    return false;
}

//***********************************************************************************************************//
// RenderRegion	Dragging Points *****************************************************************************//
//***********************************************************************************************************//
RRCornerPoints::RRCornerPoints(RenderRegion *rregion)
: m_isMovingPoints (false)
, m_activePoint (-1)
, m_dX (-1)
, m_dY (-1)
{
	m_rregion = rregion;
	m_rregion->getHostWidget()->installEventFilter(this);
}

void RRCornerPoints::setPoints(const QRectF &rr_bbox)
{
    m_points.clear();
    m_points.push_back(rr_bbox.topRight());
    m_points.push_back(rr_bbox.topLeft());
    m_points.push_back(rr_bbox.bottomLeft());
    m_points.push_back(rr_bbox.bottomRight());
}


void RRCornerPoints::paint(QPainter* iPainter)
{
	if (m_rregion->isBlocked()) return;

	iPainter->setRenderHint(QPainter::Antialiasing);

	int pLenght = 4;
	QRectF pRect; QPointF hPoint;

	QColor defFillCol (200, 200, 200, 120);
	//QColor defDrawCol (0, 100, 255, 180);  //kinda cyan color!?
	QColor defDrawCol (61, 166, 244, 220);  //kinda cyan color!?

	//Draw the four corner pts
	for (int i=0; i<m_points.size(); ++i)
	{
		hPoint = m_points.at(i);
		pRect.setTopLeft(QPointF(hPoint.x()-pLenght, hPoint.y()-pLenght));
		pRect.setBottomRight(QPointF(hPoint.x()+pLenght, hPoint.y()+pLenght));

		if(i==m_activePoint || (i==m_nbPivotPt && m_isMovingPoints))
		{
			//highlight the selected point
			defFillCol = QColor(200, 200, 200, 255);
		}

		iPainter->fillRect(pRect, defFillCol);

		iPainter->setPen(defDrawCol);
		iPainter->drawRect(pRect);
	}

	//display info while draggin points
	if(m_isMovingPoints)
	{
		int xOffs = 8;
		int yOffs = 8;
		int bOffs = 24;

		//XY Pivot Point
		QPointF pvXY = m_pivotPoint;

		int baseHeight = 16;
		int baseWidth = 50;		// inc rect size to accomodate coords
		if(m_pivotPoint.x()>10 && m_pivotPoint.x()<100) baseWidth += 6;
		else if(m_pivotPoint.x()>100 && m_pivotPoint.x()<1000) baseWidth += 12;
		else if(m_pivotPoint.x()>1000) baseWidth += 18;
		if(m_pivotPoint.y()>10 && m_pivotPoint.y()<100) baseWidth += 6;
		else if(m_pivotPoint.y()>100 && m_pivotPoint.y()<1000) baseWidth += 12;
		else if(m_pivotPoint.y()>1000) baseWidth += 18;

		//Width/Height
		QPointF pvWHXY( m_pivotPoint.x() + baseWidth + xOffs , pvXY.y());
		QRectF rrRect = m_rregion->getBBoxRect();
		int rrWidth = abs(rrRect.width());
		int rrHeight = abs(rrRect.height());

		int baseWH_Width = 54;	//hugly inc rect size to accomodate coords
		if(rrWidth>10 && rrWidth<100) 			baseWH_Width += 6;
		else if(rrWidth>100 && rrWidth<1000) 	baseWH_Width += 12;
		else if(rrWidth>1000) 					baseWH_Width += 18;
		if(rrHeight>10 && rrHeight<100) 		baseWH_Width += 6;
		else if(rrHeight>100 && rrHeight<1000) 	baseWH_Width += 12;
		else if(rrHeight>1000) 					baseWH_Width += 18;


		//XY rect coords
		QRectF pvRect;

		if(m_nbPivotPt==0)
			pvRect = QRectF(	pvXY.x()+xOffs,
							pvXY.y()>bOffs ? pvXY.y()-(yOffs*3-2) : pvXY.y()+yOffs, //swap side if near the edge
							baseWidth, baseHeight);
		else if(m_nbPivotPt==1)
			pvRect = QRectF(	pvXY.x()-(baseWidth+xOffs)-(baseWH_Width+xOffs),
							pvXY.y()>bOffs ? pvXY.y()-(yOffs*3-2) : pvXY.y()+yOffs,
							baseWidth, baseHeight);
		else if(m_nbPivotPt==2)
			pvRect = QRectF(	pvXY.x()-(baseWidth+xOffs)-(baseWH_Width+xOffs),
							pvXY.y()<m_rregion->getFullViewSize().height()-bOffs ? pvXY.y()+yOffs : pvXY.y()-(yOffs*3-2),
							baseWidth, baseHeight);
		else if(m_nbPivotPt==3)	//check correct side
			pvRect = QRectF(	pvXY.x()+xOffs,
							pvXY.y()<m_rregion->getFullViewSize().height()-bOffs ? pvXY.y()+yOffs : pvXY.y()-(yOffs*3-2),
							baseWidth, baseHeight);

		//drawing
		iPainter->setBrush(QColor(100, 100, 100, 228));
		iPainter->setPen(Qt::gray);
		iPainter->drawRect(pvRect/*, 8, 8, Qt::RelativeSize*/);
		iPainter->setBrush(Qt::NoBrush);

		//iPainter->setPen(Qt::white);
		iPainter->setPen(QColor(16,16,16));
		QString text = tr("x: ") + QString::number(m_pivotPoint.x())
					 + tr(", y: ") + QString::number(m_pivotPoint.y());

		//set font
		QFont font = QApplication::font();
		//QFont font("Arial");
		font.setPixelSize( 11 );
		iPainter->setFont( font );

		iPainter->setRenderHint(QPainter::TextAntialiasing);
		iPainter->drawText(pvRect.x()+4, pvRect.y()+12, text);

		//Width/Height
		QRectF pvHWRect;
		if(m_nbPivotPt==0)
			pvHWRect = QRectF(pvWHXY.x()+xOffs,
							pvWHXY.y()>20 ? pvWHXY.y()-(yOffs*3-2) : pvWHXY.y()+yOffs,
							baseWH_Width, baseHeight);
		else if(m_nbPivotPt==1)
			pvHWRect = QRectF(pvXY.x()-(baseWH_Width+xOffs),
							pvXY.y()>20 ? pvXY.y()-(yOffs*3-2) : pvXY.y()+yOffs,
							baseWH_Width, baseHeight);
		else if(m_nbPivotPt==2)
			pvHWRect = QRectF(pvXY.x()-(baseWH_Width+xOffs),
							pvXY.y()<m_rregion->getFullViewSize().height()-bOffs ? pvXY.y()+yOffs : pvXY.y()-(yOffs*3-2),
							baseWH_Width, baseHeight);
		else if(m_nbPivotPt==3)	//check correct side
			pvHWRect = QRectF(pvWHXY.x()+xOffs,
							pvXY.y()<m_rregion->getFullViewSize().height()-bOffs ? pvXY.y()+yOffs : pvXY.y()-(yOffs*3-2),
							baseWH_Width, baseHeight);

		//drawing
		iPainter->setRenderHint(QPainter::Antialiasing);
		iPainter->setBrush(QColor(100, 100, 100, 228));
		iPainter->setPen(Qt::gray);
		iPainter->drawRect(pvHWRect/*, 8, 8, Qt::RelativeSize*/);
		iPainter->setBrush(Qt::NoBrush);

		//text
		//iPainter->setPen(Qt::white);
		iPainter->setPen(QColor(16,16,16));
		text = tr("w: ") + QString::number( rrWidth )
					 + tr(", h: ") + QString::number( rrHeight );
		iPainter->drawText(pvHWRect.x()+4, pvHWRect.y()+12, text/*,Qt::AlignHCenter | Qt::TextSingleLine, text*/);

		//exit
		iPainter->setPen(Qt::NoPen);
	}
}


bool RRCornerPoints::eventFilter(QObject *object, QEvent *event)
{
    if (object == m_rregion->getHostWidget() && m_rregion->isActive() && !m_rregion->isBlocked())
    {
        switch (event->type())
        {

#ifndef GFXVIEW
        case QEvent::MouseButtonPress:
#else
        case QEvent::GraphicsSceneMousePress:
#endif
        {
#ifndef GFXVIEW
			QMouseEvent *me = (QMouseEvent *) event;
			int x = me->pos().x();
			int y = me->pos().y();
#else
			QGraphicsSceneMouseEvent *me = (QGraphicsSceneMouseEvent *) event;
			int x = me->scenePos().x();
			int y = me->scenePos().y();
#endif

        	int m_pointSize = 8;
            qreal distance = -1;

        	if(me->button() == Qt::LeftButton)
        	{
				for (int i=0; i<m_points.size(); ++i)
				{
					qreal d = QLineF(me->pos(), m_points.at(i)).length();
					if ((distance < 0 && d < m_pointSize) || d < distance)
					{
						distance = d;
						m_activePoint = i;
					}
				}
				if (m_activePoint != -1)
				{
					m_rregion->setIsMovingPoints( true );
					m_isMovingPoints = true;
					//QApplication::setOverrideCursor(QCursor(Qt::ClosedHandCursor));

	        		//from pivot point, based on +/- width/height
	        		//determine in what quadrant we're, later when
	        		//moving catch quadrant swicthing ............
					//basically we record pos/neg width/height ...
					m_nbPivotPt = this->getPivotPoint( &m_pivotPoint );

					//TODO:: use the pivot with shift pressed to ........
					//move point equally in x/y, ie. making bbox a square
					QSize  activeDXY;
					activeDXY.setWidth(x-m_pivotPoint.x());
					activeDXY.setHeight(y-m_pivotPoint.y());

					m_offsX = 0;
					m_offsY = 0; //other that 0, seems crashing !!
					//if(activeDXY.width()<0)  m_offsX *= -1;
					//if(activeDXY.height()<0) m_offsY *= -1;

					//m_dX=-1; m_dY=-1;
					if(activeDXY.width()>=m_offsX)  m_dX=1;
					if(activeDXY.height()>=m_offsY) m_dY=1;

				}else
				{
					m_rregion->setIsMovingPoints( false );
				}
        	}
        } break;

#ifndef GFXVIEW
        case QEvent::MouseMove:
#else
        case QEvent::GraphicsSceneMouseMove:
#endif
        {
        	if(m_activePoint != -1)
        	{
#ifndef GFXVIEW
				QMouseEvent *me = (QMouseEvent *) event;
				int x = me->pos().x();
				int y = me->pos().y();
#else
				QGraphicsSceneMouseEvent *me = (QGraphicsSceneMouseEvent *) event;
				int x = me->scenePos().x();
				int y = me->scenePos().y();
#endif

				//force square when moving pts with shift pressed
				if(me->modifiers()==(Qt::ShiftModifier))
				{
					QPointF ttX, ttY;
					int iX = getAlignedPoint(XALIGNED, &ttX);
					int iY = getAlignedPoint(YALIGNED, &ttY);

					QPointF actP (m_points[m_activePoint]);

					int rrWidth = x - ttY.x();
					int rrHeight = y - ttX.y();

					bool doCheck=true;
					if( (rrHeight<0 && rrWidth<0) ||
						(rrHeight>0 && rrWidth>0) )
					{ doCheck=false; }//if different signs ...
					int inv = 1; 	  //preserve original sign

					//square it over the major side
					if(abs(rrWidth) >= abs(rrHeight))
					{
						if(doCheck && (rrHeight<0 || rrWidth<0)) inv=-1;
						rrHeight = rrWidth *inv;
						y = rrHeight + ttX.y();
					}
					else
					{
						if(doCheck && (rrWidth<0 || rrHeight<0)) inv=-1;
						rrWidth = rrHeight *inv;
						x = rrWidth + ttY.x();
					}
				}

				//check view/widget boundaries
				int left = m_rregion->getViewOffsetLR().width();
				int right = m_rregion->getViewOffsetLR().height();
				int top = m_rregion->getViewOffsetTB().width();
				int bottom = m_rregion->getViewOffsetTB().height();

				if(x<left) x=left;
				if(x>right) x=right -1;

				if(y<top) y=top+1;
				if(y>bottom) y=bottom -2;

				//catch rect swapping ..........
				QSize curDXY;
				curDXY.setWidth(x-m_pivotPoint.x());
				curDXY.setHeight(y-m_pivotPoint.y());

				int curDX=-1; int curDY=-1;
				if(curDXY.width()>=m_offsX) curDX=1;
				if(curDXY.height()>=m_offsY) curDY=1;

				//limit move ...................
				if(m_dX!=curDX)						//we're changing quadrant
				x = m_pivotPoint.x()+ (32*m_dX);	//offse may be +/-

				if(m_dY!=curDY)
				y = m_pivotPoint.y()+ (32*m_dY);

				//rebuil rregion from the new corner point
				//will take care to rebuild midpoijnts too
				m_rregion->rebuildBBox (QPointF(x,y), m_activePoint, RenderRegion::FROMCORNERPOINT);

				//we used QRectF directly to get back a valid rect
				//and here we get a new updated point list from the rect corners
				this->setPoints(m_rregion->getBBoxRect());
        	}
        } break;

#ifndef GFXVIEW
        case QEvent::MouseButtonRelease:
#else
        case QEvent::GraphicsSceneMouseRelease:
#endif
        {
        	if (m_isMovingPoints){
    			//we've resized out rregion, set now our slots
        		//as they are not visible while resizing rregion
				m_rregion->setSlots();

				m_activePoint = -1;
				m_dX=-1; m_dY=-1;
				m_rregion->setIsMovingPoints( false );
				m_isMovingPoints = false;
        	}
        	//QApplication::restoreOverrideCursor();
        } break;
        default: break;
        }
    }
    return false;
}
int RRCornerPoints::getAlignedPoint (int axes, QPointF* o_pt)
{
	if(axes==XALIGNED)
		for(int i=0; i<m_points.size(); i++)
			if(i!=m_activePoint)
				if(m_points[i].x()==m_points[m_activePoint].x()){
					*o_pt = m_points[i];
					return i;
				}

	if(axes==YALIGNED)
		for(int i=0; i<m_points.size(); i++)
			if(i!=m_activePoint)
				if(m_points[i].y()==m_points[m_activePoint].y()){
					*o_pt = m_points[i];
					return i;
				}

	return -1;
}

int RRCornerPoints::getPivotPoint (QPointF* outP)
{
	//get active point
	QPointF actP( m_points[m_activePoint] );

	//fill vector with not active pts
	QVector<QPointF> tt_points;
	for(int i=0; i<m_points.size(); i++)
		if(i!=m_activePoint)
			tt_points.push_back( m_points[i] );

	//if either X or Y are the same of act point,
	//that's not the pivot points, remove it
	for(int i=0; i<tt_points.size(); i++)
	if(actP.x()!=tt_points[i].x() && actP.y()!=tt_points[i].y() )
		*outP = tt_points[i];

	for(int i=0; i<m_points.size(); i++)
		if(m_points.at(i) == *outP) return i;

	return -1;
}



//***********************************************************************************************************//
// RenderRegion	MidDragging Points **************************************************************************//
//***********************************************************************************************************//
RRMidPoints::RRMidPoints(RenderRegion *rregion)
: m_activePoint (-1)
, m_isMovingPoints (false)
{
	m_rregion = rregion;
	m_rregion->getHostWidget()->installEventFilter(this);
}

void RRMidPoints::setPoints(const QRectF &rr_bbox)
{
    m_points.clear();

    QPointF bboxCenter (rr_bbox.center());
    int bboxHWidth = rr_bbox.width() /2;
    int bboxHHeight = rr_bbox.height() /2;

    // ---0---
    // |     |
    // 1  c  3
    // |     |
    // ---2---

    m_points.push_back(QPointF( rr_bbox.center().x() , rr_bbox.center().y()-bboxHHeight ));
    m_points.push_back(QPointF( rr_bbox.center().x()-bboxHWidth , rr_bbox.center().y() ));
    m_points.push_back(QPointF( rr_bbox.center().x() , rr_bbox.center().y()+bboxHHeight ));
    m_points.push_back(QPointF( rr_bbox.center().x()+bboxHWidth , rr_bbox.center().y() ));
}


void RRMidPoints::paint(QPainter* iPainter)
{
	if (m_rregion->isBlocked()) return;


	int pLenght = 4;
	QRectF pRect; QPointF hPoint;

	QColor defFillCol (200, 200, 200, 120);
	//QColor defDrawCol (0, 100, 255, 180);  //kinda cyan color!?
	QColor defDrawCol (61, 166, 244, 220);  //kinda cyan color!?

	for (int i=0; i<m_points.size(); ++i)
	{
		hPoint = m_points.at(i);
		pRect.setTopLeft(QPointF(hPoint.x()-pLenght, hPoint.y()-pLenght));
		pRect.setBottomRight(QPointF(hPoint.x()+pLenght, hPoint.y()+pLenght));

		if(i==m_activePoint)
		{
			//highlight the selected point
			defFillCol = QColor(200, 200, 200, 255);
		}

		iPainter->fillRect(pRect, defFillCol);

		iPainter->setPen(defDrawCol);
		iPainter->drawRect(pRect);
	}

	//Draw width/height info
	if(m_isMovingPoints)
	{
		int xOffs = 8;
		int yOffs = 8;
		int bOffs = 24;

		//XY bottom left(right) point
		int x = m_rregion->getBBoxRect().right();
		int y = m_rregion->getBBoxRect().bottom();
		QPointF pvXY (x,y);

		int rrWidth = abs(m_rregion->getBBoxRect().width());
		int rrHeight = abs(m_rregion->getBBoxRect().height());

		int baseHeight = 16;
		int baseWH_Width = 54;	//hugly inc rect size to accomodate coords
		if(rrWidth>10 && rrWidth<100) 			baseWH_Width += 6;
		else if(rrWidth>100 && rrWidth<1000) 	baseWH_Width += 12;
		else if(rrWidth>1000) 					baseWH_Width += 18;
		if(rrHeight>10 && rrHeight<100) 		baseWH_Width += 6;
		else if(rrHeight>100 && rrHeight<1000) 	baseWH_Width += 12;
		else if(rrHeight>1000) 					baseWH_Width += 18;

		QRectF pvRect (	pvXY.x()+xOffs,
						//pvXY.y()>bOffs ? pvXY.y()-(yOffs*3-2) : pvXY.y()+yOffs, //swap side if near the edge
						pvXY.y()<m_rregion->getFullViewSize().height()-bOffs ? pvXY.y()+yOffs : pvXY.y()-(yOffs*3-2),
						baseWH_Width, baseHeight);

		//set font
		QFont font = QApplication::font();
		//QFont font("Arial");
		font.setPixelSize( 11 );
		iPainter->setFont( font );

		//drawing
		iPainter->setRenderHint(QPainter::Antialiasing);
		iPainter->setBrush(QColor(100, 100, 100, 228));
		iPainter->setPen(Qt::gray);
		iPainter->drawRect(pvRect/*, 8, 8, Qt::RelativeSize*/);
		iPainter->setBrush(Qt::NoBrush);

		//text
		//iPainter->setPen(Qt::white);
		iPainter->setPen(QColor(16,16,16));
		QString text = tr("w: ") + QString::number( rrWidth )
					 + tr(", h: ") + QString::number( rrHeight );
		iPainter->drawText(pvRect.x()+4, pvRect.y()+12, text);

		//exit
		iPainter->setPen(Qt::NoPen);
	}
}


bool RRMidPoints::eventFilter(QObject *object, QEvent *event)
{
    if (object == m_rregion->getHostWidget() && m_rregion->isActive() && !m_rregion->isBlocked())
    {
        switch (event->type())
        {

#ifndef GFXVIEW
        case QEvent::MouseButtonPress:
#else
        case QEvent::GraphicsSceneMousePress:
#endif
        {
#ifndef GFXVIEW
			QMouseEvent *me = (QMouseEvent *) event;
			int x = me->pos().x();
			int y = me->pos().y();
#else
			QGraphicsSceneMouseEvent *me = (QGraphicsSceneMouseEvent *) event;
			int x = me->scenePos().x();
			int y = me->scenePos().y();
#endif

        	int m_pointSize = 8;
            qreal distance = -1;

        	if(me->button() == Qt::LeftButton)
        	{
				for (int i=0; i<m_points.size(); ++i)
				{
					qreal d = QLineF(me->pos(), m_points.at(i)).length();
					if ((distance < 0 && d < m_pointSize) || d < distance)
					{
						distance = d;
						m_activePoint = i;
					}
				}
				if (m_activePoint != -1)
				{
					m_rregion->setIsMovingPoints( true );

				}else
				{
					m_rregion->setIsMovingPoints( false );
				}
        	}
        } break;

#ifndef GFXVIEW
        case QEvent::MouseMove:
#else
        case QEvent::GraphicsSceneMouseMove:
#endif
        {
        	if(m_activePoint != -1)
        	{
        		//force flag or draggin pts outside the rect
        		//would engage free rregion drawing ........
				m_rregion->setIsMovingPoints( true );
				m_isMovingPoints = true;

#ifndef GFXVIEW
				QMouseEvent *me = (QMouseEvent *) event;
				int x = me->pos().x();
				int y = me->pos().y();
#else
				QGraphicsSceneMouseEvent *me = (QGraphicsSceneMouseEvent *) event;
				int x = me->scenePos().x();
				int y = me->scenePos().y();
#endif

				//check view/widget boundaries
				int left = m_rregion->getViewOffsetLR().width();
				int right = m_rregion->getViewOffsetLR().height();
				int top = m_rregion->getViewOffsetTB().width();
				int bottom = m_rregion->getViewOffsetTB().height();

				if(x<left) x=left;
				if(x>right) x=right -1;

				if(y<top) y=top+1;
				if(y>bottom) y=bottom -2;


				//limit move
				switch(m_activePoint)
				{
			    // ---0---
			    // |     |
			    // 3  c  1
			    // |     |
			    // ---2---
				//TODO: here we're clockwise while building
				//both initially the pts list and rebuilding
				//the rregion we are counter-clockwise ??!
				//the point is that it work in both cases.
				case 0:
					if(y>m_rregion->getBBoxRect().bottom()-32) y = m_rregion->getBBoxRect().bottom()-32;
					x = m_rregion->getBBoxRect().center().x();	//we don't use it later for rebuilding the
																//rect, fix it just in case
					break;
				case 1:
					if(x<m_rregion->getBBoxRect().right()+32)x=m_rregion->getBBoxRect().right()+32;
					y = m_rregion->getBBoxRect().center().y();	//idem ...................................
					break;
				case 2:
					if(y<m_rregion->getBBoxRect().top()+32)y=m_rregion->getBBoxRect().top()+32;
					x = m_rregion->getBBoxRect().center().x();
					break;
				case 3:
					if(x>m_rregion->getBBoxRect().left()-32)x=m_rregion->getBBoxRect().left()-32;
					y = m_rregion->getBBoxRect().center().y();
					break;
				}


				//force square when moving pts with shift pressed
				if(me->modifiers()==(Qt::ShiftModifier))
				{
					m_rregion->rebuildBBox (QPointF(x,y), m_activePoint, RenderRegion::FROMMIDPOINT, true);
				}else
				{
					//rebuil rregion from the new mid point
					//will take care to rebuild cornerpoijnts too
					m_rregion->rebuildBBox (QPointF(x,y), m_activePoint, RenderRegion::FROMMIDPOINT);
				}
				//we used QRectF directly to get back a valid rect
				//and here we get a new updated point list from that
				this->setPoints(m_rregion->getBBoxRect());
        	}
        } break;

#ifndef GFXVIEW
        case QEvent::MouseButtonRelease:
#else
        case QEvent::GraphicsSceneMouseRelease:
#endif
        {
        	if (m_isMovingPoints){
    			//we've resized out rregion, set now our slots
        		//as they are not visible while resiing rregion
				m_rregion->setSlots();

				m_activePoint = -1;
				m_rregion->setIsMovingPoints( false );
				m_isMovingPoints = false;
        	}

        } break;
        default: break;
        }
    }
    return false;
}


//***********************************************************************************************************//
// RenderRegion	*********************************************************************************************//
//***********************************************************************************************************//
#ifdef GFXVIEW
RenderRegion::RenderRegion(QGraphicsScene *widget)
#else
RenderRegion::RenderRegion(GLViewer *widget)
#endif
: g_renderregion (false)
, g_rr_isInit (false)
, g_rr_isBlocked (false)
, g_rr_isOverBBox (false)

, g_rr_paint (false)
, g_rr_isDrawing (false)
, g_rr_isDragging (false)
, g_rr_isMovingPoints (false)
, g_rr_isTweakingSlots (false)
, g_rr_isMovingDivider (false)

, g_rr_invalidadedContext (false)
, g_rr_lockslots (false)
, g_rr_paintInfo (false)
, g_rr_doNotPaintQuads (false)

, g_host_panned (false)

, g_rr_screen_rect (0,0,512,512)
, g_rr_fb_start (0,0)

, g_width (512)
, g_height (512)

, m_offs_top (0)
, m_offs_bottom (0)
, m_offs_left (0)
, m_offs_right (0)

,m_fbscene_width (512)
,m_fbscene_height (512)

, m_border_top (0)
, m_border_bottom (0)
, m_border_left (0)
, m_border_right (0)

, m_widget (widget)
{
	//host widget
	registerDevice ();


    //connect signals to slots
    connect( this, 		SIGNAL (rRegionHasChanged (int, int, int, int)),
    		 m_widget, 	SLOT   (setRenderRegion   (int, int, int, int)) );

    connect( this, 		SIGNAL (rrSlotHasChanged 	(int, int)),
    		 m_widget, 	SLOT   (setRenderRegionSlot (int, int)) );

    //!< Points for the rregion tweaking
    m_slotpoints 	= new RRSlotPoints (this);
    m_cornerpoints 	= new RRCornerPoints (this);
    m_midpoints 	= new RRMidPoints (this);
    m_divider		= new RRDivider (this);
}

void RenderRegion::registerDevice ()
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

void RenderRegion::setBorders ()
{
	//if fb render contained into widget
	if(m_fbscene_height<=g_height)
	{
		//set boundaries as fb render
		m_border_top = m_offs_top;
		m_border_bottom = m_offs_bottom;
	}else
	{
		//if not, set them as widget size
		m_border_top = 0;
		m_border_bottom = g_height;
	}

	if(m_fbscene_width<=g_width)
	{
		m_border_left = m_offs_left;
		m_border_right = m_offs_right;
	}else
	{
		m_border_left = 0;
		m_border_right = g_width;
	}
}

void RenderRegion::viewerInit ()
{}

//void RenderRegion::viewerResized (int iwidth, int iheight)
void RenderRegion::viewerResized ()
{
	this->hostResized ();
}

bool RenderRegion::hasInvalidatedContext()
{
	return g_rr_invalidadedContext;
}

void RenderRegion::setHasInvalidatedContext (bool ientry)
{
	g_rr_invalidadedContext = ientry;
}

template<class QRECTx, class QPOINTx, typename TYPE>
QRECTx RenderRegion::remapRegion (const QRECTx& oRect, const QRECTx& iRect, const QRECTx& iRegion,
		bool noWremap, bool noHremap)
{
	TYPE value = iRegion.topLeft().x();

	TYPE iValues [4];
	iValues[0] = iRegion.topLeft().x();
	iValues[1] = iRegion.topLeft().y();
	iValues[2] = iRegion.bottomRight().x();
	iValues[3] = iRegion.bottomRight().y();

	QPOINTx oEntries [4];
	oEntries[0] = QPOINTx (oRect.topLeft().x(), oRect.topRight().x());
	oEntries[1] = QPOINTx (oRect.topLeft().y(), oRect.bottomLeft().y());
	oEntries[2] = QPOINTx (oRect.topLeft().x(), oRect.topRight().x()); //todo:optimize, w and h remap are the same
	oEntries[3] = QPOINTx (oRect.topLeft().y(), oRect.bottomLeft().y());

	QPOINTx iEntries [4];
	iEntries[0] = QPOINTx (iRect.topLeft().x(), iRect.topRight().x());
	iEntries[1] = QPOINTx (iRect.topLeft().y(), iRect.bottomLeft().y());
	iEntries[2] = QPOINTx (iRect.topLeft().x(), iRect.topRight().x());
	iEntries[3] = QPOINTx (iRect.topLeft().y(), iRect.bottomLeft().y());

	TYPE oValues [4];
	for(int i=0; i<4; i++)
	{
		if( (noWremap && (i==0 || i==2)) ||
			(noHremap && (i==1 || i==3)))
		{
			oValues[i] = iValues[i];
			continue;
		}

		TYPE vMin = oEntries[i].x();
		TYPE vMax = oEntries[i].y();
		TYPE vDiff  = vMax - vMin;

		TYPE pMin = iEntries[i].x();
		TYPE pMax = iEntries[i].y();
		TYPE pDiff  = pMax - pMin;

		oValues[i] = pMin + ((iValues[i] - vMin) * pDiff) / vDiff;
	}

	return QRECTx (QPOINTx(oValues[0],oValues[1]), QPOINTx(oValues[2],oValues[3]));
}

void RenderRegion::hostPanned ()
{
	//while we're panning render fb over the widget
	//the rregion needs to stay stick to the fb render coords
	//while referencing the new widget coords ...

/*
	//get center rregion in widget screen coords
	QPointF wdg_c_rregion (g_rr_screen_rect.center());

	//get center in fb render coords
	QPointF rnd_c_rregion (	wdg_c_rregion.x() +abs(m_offs_left),
							wdg_c_rregion.y() +abs(m_offs_top));

	int ioffx = abs(m_widget->getOffsetWidth());
	int ioffy = abs(m_widget->getOffsetHeight());

	QPointF new_c_rregion (	 (rnd_c_rregion.x() - ioffx), //invert x
							  rnd_c_rregion.y() - ioffy);

	//std::cout << "current rregion " << wdg_c_rregion.x() << ", " << wdg_c_rregion.y() << std::endl;
	//std::cout << "render fb rregion " << rnd_c_rregion.x() << ", " << rnd_c_rregion.y() << std::endl;
	//std::cout << "new rregion " << new_c_rregion.x() << ", " << new_c_rregion.y() << std::endl;
	 *
	g_rr_screen_rect.moveCenter (new_c_rregion);
*/

	//update rregion with delta offset
	int dx = -(abs(m_offs_left) - abs(m_widget->getOffsetWidth())); //invert x ,TODO:CHECK FOR RIGHTHAND COORDS ???!!!!!!!!!!!!!!!!!!!!!!!
	int dy =  (abs(m_offs_top) - abs(m_widget->getOffsetHeight()));

	g_rr_screen_rect.moveCenter (QPointF(	g_rr_screen_rect.center().x() +dx,
											g_rr_screen_rect.center().y() +dy ));


	//init remain stuff
	if(g_rr_isInit)
	{
		//re-set stuff
		setPts ();
		setSlots (false, QPointF (-666,-666));

		g_rr_fb_start.setX (m_widget->getWidgetWidth() - g_rr_screen_rect.bottomRight().x());
		g_rr_fb_start.setY (g_rr_screen_rect.topLeft().y());
	}

	m_offs_top = m_widget->getOffsetHeight();
	m_offs_bottom = m_offs_top + m_widget->getFbHeight();
	m_offs_left = m_widget->getOffsetWidth();
	m_offs_right = m_offs_left + m_widget->getFbWidth();

	setBorders ();

	g_host_panned = true;

}

void RenderRegion::hostResized ()
{
	if(!g_renderregion &&
			!g_rr_isInit) //track it anyway if rregion is initialized but hidden
		return;


	//get previous fb size
	qreal ofbwidth = m_fbscene_width;
	qreal ofbheight = m_fbscene_height;
	qreal ifbwidth = m_widget->getFbSceneWidth();
	qreal ifbheight = m_widget->getFbSceneHeight();

	//get rregion normalized center (widget) coords
	QPointF ncenter (g_rr_screen_rect.center());

	if(g_host_panned)
	{
		//if we panned, first
		//get rr coords based on the centered widget
		//(on resize in fact we center the widget over the fb, or viceversa)
		float diffoff = ofbwidth - g_width;
		float c_width_off = (diffoff/2);
		diffoff = ofbheight - g_height;
		float c_height_off = (diffoff/2);

		int dx = -(abs(m_offs_left) - abs(c_width_off));
		int dy =  (abs(m_offs_top) - abs(c_height_off));

		ncenter = QPointF(	g_rr_screen_rect.center().x() +dx,
							g_rr_screen_rect.center().y() +dy );
		g_rr_screen_rect.moveCenter(ncenter);

		//update old offset like if the widget was centered
		m_offs_left = -c_width_off;
		m_offs_top = -c_height_off;

		g_host_panned = false;
	}

	//just remove the offset, to get them in fb coords
	ncenter.setX (ncenter.x() -m_offs_left);
	ncenter.setY (ncenter.y() -m_offs_top);

	//then get new rregion center based on new fbsize
	//ofbwidth : ifbwidth = ncenter.x() : ix
	qreal ix = (ifbwidth * ncenter.x()) /ofbwidth;
	qreal iy = (ifbheight * ncenter.y()) /ofbheight;

	//add offset to get rr center in widget coords
	QPointF icenter (ix + m_widget->getOffsetWidth(), iy + m_widget->getOffsetHeight());

	//remap rregion in fb coords
	QRectF oRect (QPointF (0,0), QPointF (ofbwidth, ofbheight));
	QRectF iRect (QPointF (0,0), QPointF (ifbwidth, ifbheight));

	QRectF resizedRR = remapRegion(oRect, iRect, g_rr_screen_rect);
	g_rr_screen_rect = resizedRR;

	//move new rregion to widget coords (fb+offs)
	g_rr_screen_rect.moveCenter(icenter);


	//init remain stuff
	if(g_rr_isInit)
	{
		//clear rregion fb slots if coming from
		//widget to framed fb
		m_slotpoints->clearSlots();

		//re-set stuff
		setPts ();
		setSlots (false, QPointF (-666,-666));

		g_rr_fb_start.setX (m_widget->getWidgetWidth() - g_rr_screen_rect.bottomRight().x());
		g_rr_fb_start.setY (g_rr_screen_rect.topLeft().y());
	}


	//init global rregion stuff
	g_width = m_widget->getWidgetWidth();
	g_height = m_widget->getWidgetHeight();

	m_fbscene_width = m_widget->getFbSceneWidth ();
	m_fbscene_height = m_widget->getFbSceneHeight ();

	m_offs_top = m_widget->getOffsetHeight();
	m_offs_bottom = m_offs_top + m_widget->getFbHeight();
	m_offs_left = m_widget->getOffsetWidth();
	m_offs_right = m_offs_left + m_widget->getFbWidth();

	setBorders ();

	if(g_renderregion)
	{
		int startX = g_rr_screen_rect.topLeft().x();
		int startY = g_rr_screen_rect.topLeft().y();
		int stopX = g_rr_screen_rect.bottomRight().x();
		int stopY = g_rr_screen_rect.bottomRight().y();

		#ifdef RIGHTHANDLEDCOORDSYS
		//last_rect is in screen coords, ie. x might be inverted, reverse that
		startX = g_width - startX; stopX = g_width - stopX;
		#endif

		//reset rregion borders
		resetBorders (startX,startY,stopX,stopY);

		//update render device framebuffer
		emit rRegionHasChanged ( startX, startY, stopX, stopY );
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool RenderRegion::init( bool iEnabled )
{
	g_renderregion = iEnabled;

	g_width = m_widget->getWidgetWidth();
	g_height = m_widget->getWidgetHeight();

	m_fbscene_width = m_widget->getFbSceneWidth ();
	m_fbscene_height = m_widget->getFbSceneHeight ();

	m_offs_top = m_widget->getOffsetHeight();
	m_offs_bottom = m_offs_top + m_widget->getFbHeight();
	m_offs_left = m_widget->getOffsetWidth();
	m_offs_right = m_offs_left + m_widget->getFbWidth();

	setBorders (); //set borders for boundaries check


	if(g_renderregion)	//!< reset renderregion
	{
		//when true, getting back from rr will restart the renderer
		g_rr_invalidadedContext = false;


		//initialize a full fb rregion
		if(!g_rr_isInit)
		{
			int off_left = m_offs_left;
			int off_top = m_offs_top;
			if(m_widget->getOffsetWidth()<0)
			{
				//reset it to widget size if
				//fb render bigger than widget
				off_left = 0;
				off_top = 0;
			}

			int off_right = m_offs_right;
			int off_bottom = m_offs_bottom;
			if(m_widget->getOffsetWidth()<0)
			{
				off_right = g_width;
				off_bottom = g_height;
			}

			//rregion bbox
			g_rr_screen_rect.setTopRight(QPointF (off_left, off_top +1));
			g_rr_screen_rect.setBottomLeft(QPointF (off_right -1, off_bottom-2));

			//enable pts if not full widget
			if(off_top!=0 && off_left!=0)
			{
				setPts ();
				setSlots ();

				g_rr_fb_start.setX (m_widget->getWidgetWidth() - g_rr_screen_rect.bottomRight().x());
				g_rr_fb_start.setY (g_rr_screen_rect.topLeft().y());

				g_rr_isInit = true;
			}

			emit rRegionHasChanged ( off_left, off_top, off_right, off_bottom );

		}else
		{
			//we may come back after a resize
			//set back previous renderregion
			QPointF qpTL = g_rr_screen_rect.topLeft();
			QPointF qpBR = g_rr_screen_rect.bottomRight();

			int startX = qpTL.x();
			int startY = qpTL.y();
			int stopX = qpBR.x();
			int stopY = qpBR.y();

			int border = 1;
			if(startX<m_border_left) 		startX = m_border_left;//0
			if(startY<m_border_top) 		startY = m_border_top;//0
			if(stopX>m_border_right) 		stopX = m_border_right;//g_width
			if(stopY>m_border_bottom) 	stopY = m_border_bottom-border;//g_height

			g_rr_screen_rect.setTopLeft(QPointF(startX, startY));
			g_rr_screen_rect.setBottomRight(QPointF(stopX, stopY));

			#ifdef RIGHTHANDLEDCOORDSYS
			//last_rect is in screen coords, ie. x might be inverted, reverse that
			startX = g_width - startX; stopX = g_width - stopX;
			#endif

			//if(startX==m_offs_left+1) stopY=m_offs_left;				//get back the offset
			if(stopY==m_border_bottom-border) stopY=m_border_bottom;	//get back the offset

			emit rRegionHasChanged ( startX, startY, stopX, stopY );
		}
	}
	else
	{
		//go full framebuffer ...
		//force routine to get fb size automatically
		emit rRegionHasChanged (-6666666, -6666666, 6666666, 6666666 );
	}

	return iEnabled;	//chain rregion status
}


void RenderRegion::paint(QPainter* iPainter)
{
	QColor penColor(255, 98, 0, 220);
	if(g_renderregion && !g_rr_paint)
	{
		//draw quadrant lines
		if(g_rr_isInit && !g_rr_doNotPaintQuads &&
				(g_rr_isDragging || g_rr_isDrawing || g_rr_isMovingPoints))
		{
			QPen pen;
			pen.setColor( penColor );
			pen.setWidth(1);
			iPainter->setPen( pen );

			int nbSeg = 3;
			int wSpace = g_rr_screen_rect.width() / nbSeg;
			int hSpace = g_rr_screen_rect.height() / nbSeg;
			QPointF tlPivot (g_rr_screen_rect.left(), g_rr_screen_rect.top());

			for(int i=1; i<nbSeg; i++)
			{
				int x = tlPivot.x()+wSpace*i;
				iPainter->drawLine (x, tlPivot.y(), x, g_rr_screen_rect.bottom());

				int y = tlPivot.y()+hSpace*i;
				iPainter->drawLine (tlPivot.x(), y, g_rr_screen_rect.right() +1, y);
			}

			//draw bbox rect
			pen.setWidth(2);
			iPainter->setPen( pen );
			iPainter->drawRect(g_rr_screen_rect);

			iPainter->setPen( penColor ); //reset qpen
			//iPainter->setPen(Qt::NoPen);
		}else
		{
			QPen pen;
			pen.setColor( penColor );
			pen.setWidth(2);
			iPainter->setPen( pen );
			//iPainter->setPen( penColor );
			iPainter->drawRect(g_rr_screen_rect);
		}

		//Paint drag rects
		this->paintPts (iPainter);

		//Draw width/height info
		int baseWH_Width=84;
		if(g_rr_paintInfo || g_rr_isDragging)
		{
			int xOffs = 8;
			int yOffs = 8;
			int bOffs = 24;

			//XY bottom left(right) point
			int x = g_rr_screen_rect.right();
			int y = g_rr_screen_rect.bottom();
			QPointF pvXY (x,y);

			int rrWidth = abs(g_rr_screen_rect.width());
			int rrHeight = abs(g_rr_screen_rect.height());

			int baseHeight = 16;
			//baseWH_Width = 84;	//hugly inc rect size to accomodate coords
			if(g_rr_screen_rect.center().x()>10 && g_rr_screen_rect.center().x()<100) 			baseWH_Width += 6;
			else if(g_rr_screen_rect.center().x()>100 && g_rr_screen_rect.center().x()<1000) 	baseWH_Width += 12;
			else if(g_rr_screen_rect.center().x()>1000) 										baseWH_Width += 18;
			if(g_rr_screen_rect.center().y()>10 && g_rr_screen_rect.center().y()<100) 			baseWH_Width += 6;
			else if(g_rr_screen_rect.center().y()>100 && g_rr_screen_rect.center().y()<1000) 	baseWH_Width += 12;
			else if(g_rr_screen_rect.center().y()>1000) 										baseWH_Width += 18;

			QRectF pvRect (	pvXY.x()+xOffs,
							//pvXY.y()>bOffs ? pvXY.y()-(yOffs*3-2) : pvXY.y()+yOffs, //swap side if near the edge
							pvXY.y()<getFullViewSize().height()-bOffs ? pvXY.y()+yOffs : pvXY.y()-(yOffs*3-2),
							baseWH_Width, baseHeight);

			//set font
			QFont font = QApplication::font();
			//QFont font("Arial");
			font.setPixelSize( 11 );
			iPainter->setFont( font );

			//drawing
			iPainter->setRenderHint(QPainter::Antialiasing);

/*
			QRectF shadow (pvRect);
			shadow.setHeight(pvRect.height()+2);
			shadow.setWidth(pvRect.width()+2);
			shadow.setTopLeft( QPointF(pvRect.topLeft().x()+2, pvRect.topLeft().y()+2) );

			//QLinearGradient qGradientLinear( QPointF(pvRect.center().x(), pvRect.top()-16), QPointF(pvRect.center().x(), pvRect.bottom()) );
			QLinearGradient qGradientLinear( pvRect.topLeft(), pvRect.bottomRight() );
			qGradientLinear.setColorAt(0.0,QColor(50,50,50,127));
			qGradientLinear.setColorAt(1.0,QColor(0,0,0,0));

			iPainter->setBrush(QColor(40, 40, 40, 250));
			//iPainter->setBrush( qGradientRadial );
			iPainter->setPen(Qt::NoPen);
			iPainter->drawRect(shadow);
*/
			iPainter->setBrush(QColor(100, 100, 100, 228));
			iPainter->setPen(Qt::gray);
			iPainter->drawRect(pvRect);
			//iPainter->drawRoundedRect(pvRect, 8, 8, Qt::RelativeSize);
			iPainter->setBrush(Qt::NoBrush);

			//text
			//iPainter->setPen(Qt::white);
			iPainter->setPen(QColor(16,16,16));
			QString text = tr("center x: ") + QString::number( (int)g_rr_screen_rect.center().x() )
						 + tr(", y: ") + QString::number( (int)g_rr_screen_rect.center().y() );
			iPainter->drawText(pvRect.x()+4, pvRect.y()+12, text);

			//exit
			iPainter->setPen(Qt::NoPen);
		}

		//paint extra info (ie. width/height)
		if(g_rr_paintInfo)
		{
			int xOffs = 8;
			int yOffs = 8;
			int bOffs = 24;

			//XY Pivot Point
			//XY bottom left(right) point
			int x = g_rr_screen_rect.right();
			int y = g_rr_screen_rect.bottom();
			QPointF pvXY (x,y);	//start from prev xy box
			QPointF pvWHXY( pvXY.x() + baseWH_Width + xOffs , pvXY.y());

			int baseHeight = 16;
			int baseWidth = 54;		// inc rect size to accomodate coords

			int rrWidth = abs(g_rr_screen_rect.width());
			int rrHeight = abs(g_rr_screen_rect.height());

			if(rrWidth>10 && rrWidth<100) 			baseWidth += 6;
			else if(rrWidth>100 && rrWidth<1000) 	baseWidth += 12;
			else if(rrWidth>1000) 					baseWidth += 18;
			if(rrHeight>10 && rrHeight<100) 		baseWidth += 6;
			else if(rrHeight>100 && rrHeight<1000) 	baseWidth += 12;
			else if(rrHeight>1000) 					baseWidth += 18;


			//set font
			QFont font = QApplication::font();
			//QFont font("Arial");
			font.setPixelSize( 11 );
			iPainter->setFont( font );

			iPainter->setRenderHint(QPainter::TextAntialiasing);

			//Width/Height
			QRectF pvHWRect (pvWHXY.x()+xOffs,
							pvWHXY.y()<getFullViewSize().height()-bOffs ? pvWHXY.y()+yOffs : pvWHXY.y()-(yOffs*3-2),
							baseWidth, baseHeight);

			//drawing
			iPainter->setRenderHint(QPainter::Antialiasing);
			iPainter->setBrush(QColor(100, 100, 100, 228));
			iPainter->setPen(Qt::gray);
			iPainter->drawRect(pvHWRect/*, 8, 8, Qt::RelativeSize*/);
			iPainter->setBrush(Qt::NoBrush);

			//text
			//iPainter->setPen(Qt::white);
			iPainter->setPen(QColor(16,16,16));
			QString text = tr("w: ") + QString::number( rrWidth )
						 + tr(", h: ") + QString::number( rrHeight );
			iPainter->drawText(pvHWRect.x()+4, pvHWRect.y()+12, text/*,Qt::AlignHCenter | Qt::TextSingleLine, text*/);

			//exit
			iPainter->setPen(Qt::NoPen);

		}
	}
}


void RenderRegion::rebuildBBox(QPointF ipt, int iact, int mode, bool forceScale)
{
	QPointF tp;
	tp.setX((int)ipt.x());
	tp.setY((int)ipt.y());

	if(mode==FROMCORNERPOINT)
	{
		// 1----0
		// |    |
		// |    |
		// 2----3
		switch( iact )
		{
		case 0:
			g_rr_screen_rect.setTopRight (tp);
			break;
		case 1:
			g_rr_screen_rect.setTopLeft (tp);
			break;
		case 2:
			g_rr_screen_rect.setBottomLeft (tp);
			break;
		case 3:
			g_rr_screen_rect.setBottomRight (tp);
			break;
		}

		//QRectF has been automagically updated
		//after the corner has been set ......

		//!< rebuild midpoint list
		m_midpoints->setPoints(g_rr_screen_rect);
	}
	else if(mode==FROMMIDPOINT)
	{
	    // ---0---
	    // |     |
	    // 1  c  3
	    // |     |
	    // ---2---


		//when shift pressed .. iev0-c c-2 are constant
		//kinda simmetrical scale .....................
		QPointF oCenter;
		if(forceScale)
			//get former center before moving sides
			oCenter = g_rr_screen_rect.center();

		switch( iact )
		{
		case 0:
		{
			//update rect
			g_rr_screen_rect.setTop (tp.y());

			if(forceScale)
			{
				//and accordling the opposite one
				int bott = g_rr_screen_rect.top() - oCenter.y();
				int gPos = oCenter.y() -bott;
				if(gPos>=m_border_bottom) gPos = m_border_bottom-1;
				g_rr_screen_rect.setBottom (gPos);
			}
		}
			break;
		case 1:
			g_rr_screen_rect.setLeft (tp.x());

			if(forceScale)
			{
				int bott = g_rr_screen_rect.left() - oCenter.x();
				int gPos = oCenter.x() -bott;
				if(gPos<m_border_left) gPos = m_border_left;
				g_rr_screen_rect.setRight (gPos);
			}
			break;
		case 2:
			g_rr_screen_rect.setBottom (tp.y());

			if(forceScale)
			{
				int bott = g_rr_screen_rect.bottom() - oCenter.y();
				int gPos = oCenter.y() -bott;
				if(gPos<m_border_top) gPos = m_border_top;
				g_rr_screen_rect.setTop (gPos);
			}
			break;
		case 3:
			g_rr_screen_rect.setRight (tp.x());

			if(forceScale)
			{
				int bott = g_rr_screen_rect.right() - oCenter.x();
				int gPos = oCenter.x() -bott;
				if(gPos>m_border_right) gPos = m_border_right;
				g_rr_screen_rect.setLeft (gPos);
			}
			break;
		}

		//QRectF has been automagically updated
		//after the corner has been set ......

		//!< rebuild midpoint list from the new rect
		m_cornerpoints->setPoints(g_rr_screen_rect);
	}

	//get rect corners
	QPointF qpTL = g_rr_screen_rect.topLeft();
	QPointF qpBR = g_rr_screen_rect.bottomRight();

	//go to setup render view
	int startX = qpTL.x();
	int startY = qpTL.y();
	int stopX = qpBR.x();
	int stopY = qpBR.y();

	#ifdef RIGHTHANDLEDCOORDSYS
	startX = g_width - startX; stopX = g_width - stopX;
	#endif

	//reset rregion borders for render device
	resetBorders (startX, startY, stopX, stopY);

	//set render region view
	emit rRegionHasChanged (  startX, startY, stopX, stopY );
}

void RenderRegion::resetBorders (int& startX, int& startY, int& stopX, int& stopY, int lOffset)
{
	if(startX==m_border_left +lOffset || startX==m_border_left -lOffset ||
	   startX==m_border_left +(lOffset*2) || startX==m_border_left -(lOffset*2) ) 	startX = m_border_left;
	if(startY==m_border_top +lOffset || startY==m_border_top -lOffset ||
	   startY==m_border_top +(lOffset*2) || startY==m_border_top -(lOffset*2)) 		startY = m_border_top;
	if(stopX==m_border_right -lOffset || stopX==m_border_right +lOffset ||
	   stopX==m_border_right -(lOffset*2) || stopX==m_border_right +(lOffset*2) ) 	stopX = m_border_right;
	if(stopY==m_border_bottom -lOffset || stopY==m_border_bottom +lOffset ||
	   stopY==m_border_bottom -(lOffset*2) || stopY==m_border_bottom +(lOffset*2) ) stopY = m_border_bottom;
}


bool RenderRegion::eventFilter(QObject *object, QEvent *event)
{
    if (object == m_widget && isActive()  /*&& !isBlocked()*/ &&
    		!g_rr_isMovingPoints && !g_rr_isTweakingSlots && !g_rr_isMovingDivider)
    {
        switch (event->type())
        {

        case QEvent::KeyPress:
        {
			QKeyEvent *me = (QKeyEvent *) event;
			if(me->key()==Qt::Key_L && me->modifiers()==Qt::ShiftModifier)
			{
				//camera dof has benn modified ........
				//invalidate the render context so that back from rregion
				//we'll reset accumulation of the whole framebuffer
				g_rr_invalidadedContext = true;
			}
			else if(me->key()==Qt::Key_L)
			{
				g_rr_invalidadedContext = true;
			}
        } break;

#ifndef GFXVIEW
        case QEvent::MouseButtonPress:
        {
        	QMouseEvent *me = (QMouseEvent *) event;
        	int x = me->pos().x();
        	int y = me->pos().y();
#else
        case QEvent::GraphicsSceneMousePress:
        {
        	QGraphicsSceneMouseEvent *me = (QGraphicsSceneMouseEvent *) event;
        	int x = me->scenePos().x();
        	int y = me->scenePos().y();
#endif

        	//!< RenderRegion mouse mode ////////////////////////////////////////////
			#ifdef RIGHTHANDLEDCOORDSYS
			x = g_width - x;
			#endif

			//!< Paint mode enabled (even if rregion is blocked)
			if(me->button() == Qt::LeftButton && me->modifiers()==(Qt::ControlModifier))
			{
				g_rr_paint = true;
				QApplication::setOverrideCursor(QCursor(Qt::PointingHandCursor));
				return false;
			}

        	if( this->isBlocked() ) return false;	//!< Locked !!

			//!< Render region draw box
			//else
			if(me->button() == Qt::LeftButton)
			{
				g_rr_isDrawing = true;

				//pivot, rect topLeft
				g_rr_fb_start.setX(x);
				g_rr_fb_start.setY(y);
			}
			//!< Render region pivot drawing
			else if(me->button() == Qt::RightButton)
			{
				g_rr_isDrawing = true;
				//we enable drawing without setting a new topleft
				//...... so the last one is effectively the pivot
			}
			//!< Render region drag box
			else if(me->button() == Qt::MidButton)
			{
				if(g_rr_isInit && g_rr_isOverBBox) //only if we're above a valid rregion
				{

					//start draggin
					g_rr_start_drag = g_rr_screen_rect.center();

					g_rr_isDragging = true;
					QApplication::setOverrideCursor(QCursor(Qt::ClosedHandCursor));
				}
			}
        } break;

#ifndef GFXVIEW
        case QEvent::MouseMove:
#else
        case QEvent::GraphicsSceneMouseMove:
#endif
        {

        	//check if we're above rregion
			if(g_rr_isInit /*&& !this->isBlocked()*/) //only if we've a valid rregion
			{
#ifndef GFXVIEW
				QMouseEvent *me = (QMouseEvent *) event;
				int x = me->pos().x();
				int y = me->pos().y();
#else
				QGraphicsSceneMouseEvent *me = (QGraphicsSceneMouseEvent *) event;
				int x = me->scenePos().x();
				int y = me->scenePos().y();
#endif

				//check if we are above rregion to start draggin it
				int offset = -4; 						//set an offset to not collide with pts events
				if( this->isBlocked() ) offset = 8; 	//but not if rregion is blocked

				QRectF tt(g_rr_screen_rect.topLeft().x()+offset, g_rr_screen_rect.topLeft().y()-offset, g_rr_screen_rect.width()-(offset*2), g_rr_screen_rect.height()-(offset*2));

				//engage drag only if mouse over rregion
				#ifdef RIGHTHANDLEDCOORDSYS
				//if(x < qpTL.x() && x > qpBR.x() &&
				//		y > qpTL.y() &&  y < qpBR.y() )
				if (tt.intersects (QRectF(x, y, 1, 1)))
				#else
				//TODO:checkthis to use intersects !!
				QPointF qpTL (g_rr_screen_rect.topLeft().x()-offset, g_rr_screen_rect.topLeft().y()+offset);
				QPointF qpBR (g_rr_screen_rect.bottomRight().x()+offset, g_rr_screen_rect.bottomRight().y()-offset );
				if(x > qpTL.x() && x < qpBR.x() &&
					y > qpTL.y() &&  y < qpBR.y() )
				#endif
				{
					g_rr_isOverBBox = true;
				}else
				{
					g_rr_isOverBBox = false;
				}
			}

			//!< Render paint with mouse as a brush /////////////////////////////
			if(g_rr_paint)
			{
#ifndef GFXVIEW
				QMouseEvent *me = (QMouseEvent *) event;
				int x = me->pos().x();
				int y = me->pos().y();
#else
				QGraphicsSceneMouseEvent *me = (QGraphicsSceneMouseEvent *) event;
				int x = me->scenePos().x();
				int y = me->scenePos().y();
#endif

				#ifdef RIGHTHANDLEDCOORDSYS
				x = g_width - x;
				#endif

				int xytile = 32;

				int rrStart_X = x-xytile; int rrStart_Y = y-xytile;
				int rrStop_X = x+xytile; int rrStop_Y = y+xytile;

				//manage out-of-view move,
				//ie. bound renderregion box to fb/widget size
				int border_offset = 1;
				int halftile = xytile/2;
				if(rrStart_X<m_border_left+halftile)		rrStart_X = m_border_left;
				if(rrStart_Y<m_border_top+halftile)		rrStart_Y = m_border_top;
				if(rrStop_X>m_border_right-halftile)		rrStop_X = m_border_right;
				if(rrStop_Y>m_border_bottom-halftile)		rrStop_Y = m_border_bottom;

				//enforce tile is reliable
				if((rrStop_X-rrStart_X)<16 || (rrStop_Y-rrStart_Y)<16)
				return false; //TODO:make it better and anywhere ?!!!!

				//Set single tile for rendering
				emit rRegionHasChanged ( rrStart_X, rrStart_Y, rrStop_X, rrStop_Y );
			}

			//!< Render region box drawing //////////////////////////////////////
			else if(g_rr_isDrawing && !this->isBlocked())
			{
#ifndef GFXVIEW
				QMouseEvent *me = (QMouseEvent *) event;
				int x = me->pos().x();
				int y = me->pos().y();
#else
				QGraphicsSceneMouseEvent *me = (QGraphicsSceneMouseEvent *) event;
				int x = me->scenePos().x();
				int y = me->scenePos().y();
#endif

				#ifdef RIGHTHANDLEDCOORDSYS
				int xx = x;
				x = g_width - x;
				#endif

				int x0 = g_rr_fb_start.x();
				int y0 = g_rr_fb_start.y();

				int rrStart_X = 0; int rrStart_Y = 0;
				int rrStop_X = 0; int rrStop_Y = 0;

				int rrWidth = x - x0;
				int rrHeight = y -y0;

				//force some drawing to get a new rregion
				if(abs(rrWidth)<16 || abs(rrHeight)<16)
				{
					if(g_rr_isOverBBox){
						g_rr_paintInfo = true;	//so we just click on rregion to get displayed info stuff
					}else g_rr_doNotPaintQuads = true;

					g_rr_lockslots = true;
					return false;
				}else g_rr_lockslots = false;
				g_rr_doNotPaintQuads = false;

				//force square when drawing with shift pressed
				if(me->modifiers()==(Qt::ShiftModifier))
				{
					bool doCheck=true;
					if( (rrHeight<0 && rrWidth<0) ||
						(rrHeight>0 && rrWidth>0) )
					{ doCheck=false; }//if different signs ...
					int inv = 1; 	  //preserve original sign

					//square it over the major side
					if(abs(rrWidth) >= abs(rrHeight))
					{
						if(doCheck && (rrHeight<0 || rrWidth<0)) inv=-1;
						rrHeight = rrWidth *inv;
						y = rrHeight + y0;
					}
					else
					{
						if(doCheck && (rrWidth<0 || rrHeight<0)) inv=-1;
						rrWidth = rrHeight *inv;
						x = rrWidth + x0;
					}
				}

				//swap topLeft/bottomRight if necessary
				if(rrWidth>0){ rrStart_X = x0; rrStop_X = x; }
				else{ rrStart_X = x; rrStop_X = x0;	}

				if(rrHeight>0){ rrStart_Y = y0; rrStop_Y = y; }
				else{ rrStart_Y = y; rrStop_Y = y0;	}

				//TODO: size limit
				//if(rrStop_X<=rrStart_X+32) rrStop_X=rrStart_X+32;
				//if(rrStop_Y<=rrStart_Y+32) rrStop_Y=rrStart_Y+32;

				//manage out-of-view move, ie. bound renderregion to fb size
				int border_offset = 1;

				if(rrStart_X<m_border_left)	rrStart_X = m_border_left +border_offset;
				if(rrStart_Y<m_border_top)	rrStart_Y = m_border_top +border_offset;
				if(rrStop_X>m_border_right)	rrStop_X =  m_border_right; //-border_offset
				if(rrStop_Y>m_border_bottom)	rrStop_Y =  m_border_bottom -(border_offset*2);

				//painter rect needs screen pos
				int rrStart_screenX = rrStart_X; int rrStop_screenX = rrStop_X;
				#ifdef RIGHTHANDLEDCOORDSYS
				rrStart_screenX = g_width-rrStart_X; rrStop_screenX = g_width-rrStop_X;
				#endif
				g_rr_screen_rect.setTopLeft(QPointF(rrStart_screenX,rrStart_Y));
				g_rr_screen_rect.setBottomRight(QPointF(rrStop_screenX,rrStop_Y));

				//!< install draggable points
				setPts ();

				//as we applied an offset for boundary rregions, remove it from fb render
				resetBorders (rrStart_X, rrStart_Y, rrStop_X, rrStop_Y);

				//set renderer region (with fb coords)
				emit rRegionHasChanged ( rrStart_X, rrStart_Y, rrStop_X, rrStop_Y );

				if(!g_rr_isInit) g_rr_isInit = true; //!< a first initial rregion has been drawn
			}

			//!< Render region box dragging /////////////////////////////////////
			else if(g_rr_isDragging && !this->isBlocked())
			{
#ifndef GFXVIEW
				QMouseEvent *me = (QMouseEvent *) event;
				int x = me->pos().x();
				int y = me->pos().y();
#else
				QGraphicsSceneMouseEvent *me = (QGraphicsSceneMouseEvent *) event;
				int x = me->scenePos().x();
				int y = me->scenePos().y();
#endif

				#ifdef RIGHTHANDLEDCOORDSYS
				x = g_width - x;
				#endif

				//use current mouse pos as rect center
				#ifdef RIGHTHANDLEDCOORDSYS
				int dX = g_width-x;
				#else
				int dX = x;
				#endif
				int dY = y;

				//get width/height of the rect
				int wRect = g_rr_screen_rect.width();
				int hRect = g_rr_screen_rect.height();

				//force X or Y direction when shift pressed
				if(me->modifiers()==(Qt::ShiftModifier))
				{
					int xx = abs( dX - g_rr_start_drag.x() );
					int yy = abs( dY - g_rr_start_drag.y() );

					if(xx+8 > yy)
					{
						dY = g_rr_start_drag.y();
					}else
					{
						dX = g_rr_start_drag.x();
					}
				}

				//update the corner
				int rrStart_X = dX - (wRect/2);
				int rrStart_Y = dY - (hRect/2);

				//check widget boundaries, 1 is an offset to
				//better display rregion box at widget borders
				//TODO: make it work for LEFTHAND coords !!!!!
				int lOffset = 1;

				if(rrStart_X < m_border_left-wRect) 		rrStart_X = m_border_left-wRect +lOffset;
				if(rrStart_X > m_border_right) 			rrStart_X = m_border_right -lOffset; //
				if(rrStart_Y < m_border_top+lOffset) 		rrStart_Y = m_border_top +lOffset;
				if((rrStart_Y+hRect) > m_border_bottom) 	rrStart_Y = m_border_bottom-hRect -lOffset; //

				//re-establish rect based on original size and new corner
				g_rr_screen_rect.setTopLeft(QPointF(rrStart_X,rrStart_Y));
				g_rr_screen_rect.setWidth(wRect);
				g_rr_screen_rect.setHeight(hRect);

				//!< install draggable points
				setPts ();

				//set renderregion
				QPointF qpTL = g_rr_screen_rect.topLeft();
				QPointF qpBR = g_rr_screen_rect.bottomRight();
				int xFBStart = qpTL.x(); int xFBStop = qpBR.x();
				#ifdef RIGHTHANDLEDCOORDSYS
				//last_rect is in screen coords, x might be inverted, reverse it
				xFBStart = g_width - qpTL.x(); xFBStop = g_width - qpBR.x();
				#endif

				//for rregions that end at 1 g_height-1, ie they may have
				//been moved by the above border check,or whatever, stich it to 0
				int startX = xFBStart;
				int startY = qpTL.y();
				int stopX = xFBStop;
				int stopY = qpBR.y();

				resetBorders (startX, startY, stopX, stopY);

				//alert render device for framebuffer changes
				emit rRegionHasChanged ( startX, startY, stopX, stopY );

				//update rr_start for pivot draggin
				//TODO: once dragged, the initial quadrant is lost
				//hence we'll use the wrong pivot point...........
				g_rr_fb_start.setX (startX);
				g_rr_fb_start.setY (qpTL.y());
			}
        } break;

#ifndef GFXVIEW
        case QEvent::MouseButtonRelease:
#else
        case QEvent::GraphicsSceneMouseRelease:
#endif
        {
			//!< Render paint released mouse ////////////////////////////////////////
			if(g_rr_paint)
			{
				g_rr_paint=false;
				QApplication::restoreOverrideCursor();

				//we may have started painting .........
				//before having drawn an initial rregion
				//so will keep last tile as rregion ....
				if(!g_rr_isInit)return false;

				//set back previous renderregion
				QPointF qpTL = g_rr_screen_rect.topLeft();
				QPointF qpBR = g_rr_screen_rect.bottomRight();
				int xFBStart = qpTL.x(); int xFBStop = qpBR.x();
				#ifdef RIGHTHANDLEDCOORDSYS
				//last_rect is in screen coords, x might be inverted, reverse it
				xFBStart = g_width - qpTL.x(); xFBStop = g_width - qpBR.x();
				#endif

				emit rRegionHasChanged ( xFBStart, qpTL.y(), xFBStop, qpBR.y() );
				return false;
			}

        	if( this->isBlocked() ) return false; //enforce lock

			//!< Render region finished drawing box /////////////////////////////////
			if(g_rr_isDrawing)
			{
				g_rr_isDrawing = false;

				//we finished drawing rregion, set slots
				if(!g_rr_lockslots)
				//m_slotpoints->setPoints (g_rr_screen_rect);
				setSlots();

				g_rr_lockslots = false;
				g_rr_paintInfo = false;
				g_rr_doNotPaintQuads = false;
			}

			//!< Render region finished dragging box /////////////////////////////////
			else if(g_rr_isDragging)
			{
				g_rr_isDragging = false;
				QApplication::restoreOverrideCursor();

				//we finished dragging rregion, set slots
				//TODO:still need to implement the lock in the draggin
				//event routine, just check for a topleft area .......
				if(!g_rr_lockslots)
				//m_slotpoints->setPoints (g_rr_screen_rect);
				setSlots();

				g_rr_lockslots = false;
			}

        } break;
        default: break;
        }
    }
    return false;
}
