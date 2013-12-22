/*
 * snapshotsviewer.cpp
 *
 *  Created on: Dec 18, 2013
 *      Author: max
 */


#include "snapshotsviewer.h"


SnapshotItem::SnapshotItem(	QString const & pixmap, QString const & name, int iHeight, QGraphicsItem * parent )
: QGraphicsWidget (parent)
, m_size (QSize((iHeight-8)*1.5,iHeight-8)), m_selected (false)
, m_pxm (new QPixmap(pixmap))
{
	//qDebug()  << "SnapshotItem->iHeight: " << iHeight;
	setObjectName (name);
	setPreferredSize (m_size);
	//setPreferredSize (m_pxm->size());
	//setMinimumSize (m_pxm->size());
	//setMaximumSize (m_pxm->size());
}

SnapshotItem::~SnapshotItem() { /*qDebug()<<"Deleting Snapshots Item!!";*/ delete m_pxm; }


void SnapshotItem::paint ( QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget )
{
	Q_UNUSED( option );
	Q_UNUSED( widget );

	if ( m_selected )
	{
		//painter->setBrush(Qt::yellow);
		painter->setPen(QPen(Qt::lightGray, 2 ));
		painter->drawRect (-1,-1, m_size.width()+2,m_size.height()+2);
	}else
	{
		painter->setOpacity(0.5);
	}

	QRect pxRect (0,0, m_size.width(), m_size.height());
	painter->drawPixmap (pxRect, *m_pxm);
	//painter->drawText (QRectF(QPointF(0,0), preferredSize()), objectName(), QTextOption(Qt::AlignHCenter|Qt::AlignCenter));
	painter->setOpacity(1.0);
}

void SnapshotItem::mousePressEvent ( QGraphicsSceneMouseEvent * mouseEvent )
{
    Q_UNUSED(mouseEvent);
    mouseEvent->accept();
    qDebug() << "item mouse press event" << objectName();
    m_selected = !m_selected;
    update();
}

void SnapshotItem::mouseMoveEvent ( QGraphicsSceneMouseEvent * mouseEvent )
{
    Q_UNUSED(mouseEvent);
    qDebug() << "item mouse move event" << objectName();
    //m_selected = false;
    update();
}

void SnapshotItem::mouseReleaseEvent ( QGraphicsSceneMouseEvent * mouseEvent )
{
    Q_UNUSED(mouseEvent);
    qDebug() << "item mouse release event" << objectName();
    //m_selected = false;
    update();
}

