//This file is part of the Enume project which provides libraries for
//extending Qt functionality.
//
//Copyright (C) 2010  Marko Mattila, marko.a.mattila@gmail.com
//
//This library is free software; you can redistribute it and/or
//modify it under the terms of the GNU Lesser General Public
//License as published by the Free Software Foundation; either
//version 2.1 of the License, or (at your option) any later version.
//
//This library is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//Lesser General Public License for more details.
//
//You should have received a copy of the GNU Lesser General Public
//License along with this library; if not, write to the Free Software
//Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

#include "pannableview.h"
#include "pannableview_p.h"


#include <QGraphicsScene>
#include <QGraphicsWidget>
#include <QTime>
#include <QPointF>
#include <QTimeLine>
#include <QtDebug>
#include <math.h>
#include <QPaintEvent>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsLayout>
#include <QGraphicsLinearLayout>
#include <QRectF>
#include <QCoreApplication>
#include <QPropertyAnimation>
#include <QCoreApplication>
#include <QGraphicsView>
#include <QList>
#include <QPropertyAnimation>


#define PANNABLE_FRICTION_COEFFICIENT 0.15 * 9.81  // F = y*N, 0.1 = friction coefficient for ice on ice.
#define TAP_GESTURE_THRESHOLD 20
#define VELOCITY_THRESHOLD 0.5
#define MSECS 1000
#define SCROLL_MIN_THRESHOLD 0
#define SCROLL_MAX_THRESHOLD 0.35
#define SLOW_PERCENT -0.4
#define SWIPE_GESTURE_THRESHOLD 150


PannableViewPrivate::PannableViewPrivate( PannableView * parent ):
        q_ptr              (parent ),
        m_deltaX           (   0   ),
        m_deltaY           (   0   ),
        m_vX               (   0   ),
        m_vY               (   0   ),
        m_minX             (   0   ),
        m_minY             (   0   ),
        m_maxX             (   0   ),
        m_maxY             (   0   ),
        m_time             (       ),
        m_scrollingTimeLine(   0   ),
        m_orientation      (   0   ),
        m_skipTapGesture   ( false ),        
        m_pannableWidget   (   0   ),
        m_enableIndicator  ( true  ),        
        m_enableContinuousScrolling( false )
{
    m_scrollingTimeLine = new QTimeLine( MSECS, this );
    m_scrollingTimeLine->setCurveShape( QTimeLine::LinearCurve );
    m_scrollingTimeLine->setUpdateInterval(30);
    connect( m_scrollingTimeLine, SIGNAL(valueChanged(qreal)), this, SLOT(scroll(qreal)));
    connect( m_scrollingTimeLine, SIGNAL(finished()), this, SLOT(emitPositionChanged()));    
}

PannableViewPrivate::~PannableViewPrivate()
{

}

void PannableViewPrivate::startScrolling( int duration )
{    
    m_scrollingTimeLine->setCurrentTime(0);
    m_scrollingTimeLine->setDuration( duration );
    m_scrollingTimeLine->start();

    Q_Q(PannableView);
    q->emit scrolling(1);
}

void PannableViewPrivate::stopScrolling( int clear )
{
    m_scrollingTimeLine->stop();

    if ( clear ){
        m_scrollingTimeLine->setCurrentTime(0);
        m_vX = 0;
        m_vY = 0;
    }

    emitPositionChanged();

    Q_Q(PannableView);
    q->emit scrolling(0);
}




bool PannableViewPrivate::mousePressEvent( QGraphicsSceneMouseEvent * event ){
    Q_UNUSED(event);

    emit stopAnimation();

    if ( event->button() != Qt::LeftButton ){
        event->ignore();
        return false;
    }

    if (m_scrollingTimeLine->state() == QTimeLine::Running ){
        stopScrolling(false);
        m_skipTapGesture = true; // We don't want to stop and tap. We want stop or tap
    }

    qDebug() << "PannableViewPrivate->mousePressEvent";
    m_time.start();    
    return true;
}

bool PannableViewPrivate::mouseMoveEvent( QGraphicsSceneMouseEvent * event ){

    qreal deltaX = 0;
    qreal deltaY = 0;

    QPointF delta = event->pos()  - event->lastPos();
    if ( m_orientation & Qt::Horizontal ){
        deltaX = delta.x();
    }

    if ( m_orientation & Qt::Vertical ){
        deltaY = delta.y();
    }

    // translate the view by delta    
    m_pannableWidget->moveBy(deltaX, deltaY);

    // emit scrolling signals
    Q_Q(PannableView);
    q->emit scrolling(1);

    return true;
}

bool PannableViewPrivate::mouseReleaseEvent( QGraphicsSceneMouseEvent *event ){


    if ( event->button() != Qt::LeftButton ){
        event->ignore();        
        return false;
    }


    // For some weird reason the mouseGrapper thingy makes the buttonDownPos() to return
    // exactly the same as scenePos() therefore we need to store mouse press position and
    // use it here.
    QPointF pressPos   = event->buttonDownPos(Qt::LeftButton);
    QPointF releasePos = event->pos();

    QPointF delta = pressPos - releasePos;
    qreal  distance = sqrt( pow(delta.x(),2) + pow(delta.y(),2));


    if ( distance < TAP_GESTURE_THRESHOLD && !m_skipTapGesture ){
        // This is a tap gesture
        return tap( event );
    }else{
        // This is a swipe gesture
        return swipe( pressPos, releasePos );
    }

}

bool PannableViewPrivate::tap(  QGraphicsSceneMouseEvent * event )
{    
    endReached();

    Q_Q(PannableView);
    if ( !(event->type() == QEvent::GraphicsSceneMousePress ||
           event->type() == QEvent::GraphicsSceneMouseRelease )){
        return false;
    }


    if ( event->button() != Qt::LeftButton ){
        return false;
    }

    QGraphicsSceneMouseEvent press(QEvent::GraphicsSceneMousePress);
    QGraphicsSceneMouseEvent release(QEvent::GraphicsSceneMouseRelease);

    press.setScenePos( event->scenePos());
    press.setScreenPos(event->screenPos() );
    press.setButton( event->button());
    press.setPos(event->pos());

    release.setScenePos( event->scenePos());
    release.setScreenPos(event->screenPos() );
    release.setButton( event->button());
    release.setPos(event->pos());

    QGraphicsScene * scene = q->scene();
    QList<QGraphicsItem*> items = scene->items(event->scenePos());

    Q_FOREACH(QGraphicsItem * item, items ){
        QGraphicsWidget * widget = static_cast<QGraphicsWidget*>(item);

        // Filter out this and parent items
        if ( 0 == widget || q == widget || q->parentItem() == widget  ){
            continue;
        }


        // Send event to the receivers
        scene->sendEvent(widget, &press);
        scene->sendEvent(widget, &release);

    }

    q->emit tapped(event->pos());    
    return true;
}


bool PannableViewPrivate::swipe( QPointF const & startPoint, QPointF const &endPoint )
{    
    m_skipTapGesture = false;
    QPointF delta = endPoint-startPoint;
    qint32 tD     =  m_time.elapsed();  // Delta Time
    qreal  dxD    = delta.x();          // Delta X distance
    qreal  dyD    = delta.y();          // Delta Y distance


    qreal vx = (dxD / tD ) * MSECS;          // Velocity in x dimension (pixels / second).
    qreal vy = (dyD / tD ) * MSECS;          // Velocity in y dimension (pixels / second).

    // Store swipe style for later use
    storeSwipeStyle(startPoint, endPoint);

    if ( fabs( vx ) > VELOCITY_THRESHOLD || fabs(vy) > VELOCITY_THRESHOLD ){        
        qreal velocity = 0;  // Current velocity in x or y dimension.

        m_deltaX = dxD;
        m_deltaY = dyD;

        m_vX = vx;
        m_vY = vy;

        if ( m_orientation & Qt::Horizontal ){
            velocity = fabs( vx );
        }else{
            velocity = fabs( vy );
        }

        if ( velocity > 0 && !endReached() ){
            // Actually this duration doesn't effect on anything at the moment.
            startScrolling( static_cast<int>( velocity/PANNABLE_FRICTION_COEFFICIENT ) );
        }


        return true;
    }

    return true;
}

void PannableViewPrivate::scroll( qreal value ){
    Q_UNUSED(value);

    // Check first if we have reached one of the ends    


    if ( endReached() ){
        return;
    }

    qreal sX=0, sY=0;

    Qt::Orientations orientation = m_orientation;
    if ( m_orientation & Qt::Vertical && m_orientation & Qt::Horizontal){
        switch ( m_swipeStyle ){
        case Vertical:
            orientation = Qt::Vertical;
            break;
        case Horizontal:
            orientation = Qt::Horizontal;
            break;
        default:
            break;
        }
    }

    if ( orientation & Qt::Horizontal ){
        // s = 1/2 * (u + v)t,
        // u=0,
        // t=update interval (in ms) of the QTimeLine.
        sX = m_vX * (m_scrollingTimeLine->updateInterval()/ 2) / MSECS ;

        // v = u + at,
        // u = d_ptr->m_vX (current speed),
        // a = yN/m, m = 1kg (just to have something), yN = friction
        qreal v = ( fabs(m_vX) - (PANNABLE_FRICTION_COEFFICIENT * m_scrollingTimeLine->updateInterval()) );
        m_vX = (m_vX < 0 ? -v : v);


        if ( fabs(sX) > SCROLL_MIN_THRESHOLD && fabs(sX) < SCROLL_MAX_THRESHOLD ){
            stopScrolling();
        }
    }

    if ( orientation & Qt::Vertical ){
        // s = 1/2 * (u + v)t,
        // u=0,
        // t=update interval (in ms) of the QTimeLine.
        sY = m_vY * (m_scrollingTimeLine->updateInterval()/ 2) / MSECS;

        // v = u + at,
        // u = d_ptr->m_vY (current speed),
        // a = yN/m, m = 1kg (just to have something), yN = friction
        qreal v = ( fabs(m_vY) - (PANNABLE_FRICTION_COEFFICIENT * m_scrollingTimeLine->updateInterval()) );
        m_vY = (m_vY < 0 ? -v : v);

        if ( fabs(sY) > SCROLL_MIN_THRESHOLD && fabs(sY) < SCROLL_MAX_THRESHOLD ){            
            stopScrolling();
        }
    }

    m_pannableWidget->moveBy(sX*2, sY*2);
    emitPositionChanged();
}


void PannableViewPrivate::animateEnd(QList<QPointF> const & points, int duration )
{

    if ( points.isEmpty()){
        return;
    }

    QPropertyAnimation * animation = new QPropertyAnimation(m_pannableWidget,"pos");
    qreal step  = 1.0 / (points.size() - 1);
    qreal value = 0;

    Q_FOREACH( QPointF point, points){        
        animation->setKeyValueAt( value, point);
        value += step;
    }

    animation->setEasingCurve(QEasingCurve::OutQuart);
    animation->setDuration( duration );
    connect( animation, SIGNAL(finished()), this, SLOT(emitPositionChanged()));    
    connect( this, SIGNAL(stopAnimation()), animation,SLOT(stop()));
    animation->start(QAbstractAnimation::DeleteWhenStopped);
}


void PannableViewPrivate::storeSwipeStyle( QPointF const &first, QPointF const &second )
{
    qreal yD = fabs(second.y() - first.y());
    qreal xD = fabs(second.x() - first.x());


    if ( yD <= SWIPE_GESTURE_THRESHOLD ){
        m_swipeStyle = Horizontal;        
        return;
    }

    if ( xD <= SWIPE_GESTURE_THRESHOLD ){
        m_swipeStyle = Vertical;       
        return;
    }

    if ( yD > SWIPE_GESTURE_THRESHOLD &&
         xD > SWIPE_GESTURE_THRESHOLD){
        m_swipeStyle = Diagonal;
    }
}

bool PannableViewPrivate::endReached()
{    
    QPointF pos   = m_pannableWidget->pos();
    qreal widgetX = pos.x();
    qreal widgetY = pos.y();
    qreal margin  = 0;    

    Q_Q(PannableView);
    QRectF limitRect( QPointF(widgetX, widgetY), m_pannableWidget->size() );
    QRectF viewRect = q->boundingRect();
    int viewHeight  = viewRect.height();
    int viewWidth   = viewRect.width();

    // Flags to indicate if e.g. the limit rect height or width is less than viewRect's
    // Then we can't use that value as a comparison because it will be true always.
    bool checkWidth  = limitRect.width() >= viewWidth;
    bool checkHeight = limitRect.height() >= viewHeight;

    if ( !limitRect.contains( viewRect )){

        QRectF intersection = limitRect.intersected( viewRect );
        QList<QPointF> points;
        points << pos;
        int width = intersection.width();
        int height= intersection.height();


        // Handle the situation where we have intersection in corners
        if (width >= margin && width < viewWidth &&
            height >= margin && height < viewHeight ){
            stopScrolling();
            int x = widgetX, y = widgetY;
            if ( checkWidth && m_deltaX < 0 ){
                x = -m_maxX;
            }else{
                x = m_minX;
            }

            if ( checkHeight && m_deltaY < 0 ){
                y = -m_maxY;
            }else{
                y = m_minY;
            }
            points << QPointF(x,y);
            animateEnd(points);
            return true;
        }

        // Handle the situation where only left, top, right or bottom
        // has been intersected.
        if (checkWidth && width >= margin && width < viewWidth){            
            stopScrolling();
            if ( m_deltaX < 0 ){
                //Panning to the right ->
                points << QPointF(-m_maxX, widgetY);
            }else{
                //Panning to the left <-
                points << QPointF(m_minX, widgetY);
            }            
            animateEnd(points);
            return true;
        }

        if ( checkHeight && height >= margin && height < viewHeight ){
            stopScrolling();
            if ( m_deltaY < 0 ){
                //Panning to the down
                points <<  QPointF( widgetX, -m_maxY);
            }else{
                //Panning to the top
                points << QPointF( widgetX, m_minY);
            }
            animateEnd(points);
            return true;
        }

    }

    return false;
}

void PannableViewPrivate::emitPositionChanged()
{
    Q_Q(PannableView);
    QPointF point = m_pannableWidget->pos() * -1.0;
    q->emit posChanged(QRectF(point, q->boundingRect().size()));

    // TODO: Add here indicator updating
}

void PannableViewPrivate::moveToBegin()
{    
    m_pannableWidget->setPos(0, 0);
}

void PannableViewPrivate::moveToEnd()
{    
    m_pannableWidget->setPos(m_maxX, m_maxY);
}

bool PannableViewPrivate::isScrolling() const
{    
    return (m_scrollingTimeLine->state() == QTimeLine::Running);
}

void PannableViewPrivate::moveTo(QPointF const & pos )
{
    QPointF newPos;
    if (m_orientation & Qt::Vertical){
        newPos.setY( -pos.y());
    }
    if (m_orientation & Qt::Horizontal){
        newPos.setX( -pos.x());
    }

    if ( isScrolling() ){
        stopScrolling();
    }

    QList<QPointF> points;
    points << m_pannableWidget->pos() << newPos;
    animateEnd(points);
}


void PannableViewPrivate::recalculateGeometry()
{
    Q_Q(PannableView);    
    if(!(m_pannableWidget->layout())) qDebug() << "no layout!!";
    QRectF geom = m_pannableWidget->layout()->geometry();
    m_minX = geom.x();
    m_minY = geom.y();
    m_maxX = geom.width() - q->boundingRect().width();
    m_maxY = geom.height()- q->boundingRect().height();
}



//// Pannable Widget Implementation ////
PannableWidget::PannableWidget( QGraphicsObject * parent ): QGraphicsWidget( parent ), m_layout(NULL)
{
    setFlags( QGraphicsItem::ItemClipsChildrenToShape);
    setObjectName("PannableWidget");
}

PannableWidget::~PannableWidget(){
}


QPainterPath PannableWidget::shape () const{
    QPainterPath path;
    QSizeF size = QGraphicsWidget::size();
    path.addRect( QRectF( 0,0, size.width(), size.height() ) );
    return path;
}


void PannableWidget::setWidget( QGraphicsWidget * widget ){
    if ( 0 == widget ){
        setLayout(0);
        return;
    }

    setFlag(QGraphicsItem::ItemStacksBehindParent);
    m_layout = new QGraphicsLinearLayout( Qt::Vertical );
    m_layout->setSpacing(0);
    m_layout->setContentsMargins(0,0,0,0);
    m_layout->addItem( widget );
    setLayout( m_layout );
}




//// Pannable View Implementation ////
PannableView::PannableView( Qt::Orientations orientation, qreal width, qreal height, QGraphicsItem * parent ):
        QGraphicsWidget(parent),
        d_ptr( new PannableViewPrivate(this) )
{
    setMinimumSize  ( width, height );
    setPreferredSize( width, height );
    setMaximumSize  ( width, height );
    setFlags( QGraphicsItem::ItemClipsChildrenToShape );
    Q_D(PannableView);
    d->m_orientation    = orientation;    
    d->m_pannableWidget = new PannableWidget( this );

    // TODO: Add here grabGesture() calls and make this widget to use QGestures
}

PannableView::PannableView( Qt::Orientations orientation, QGraphicsItem * parent ):
        QGraphicsWidget(parent),
        d_ptr( new PannableViewPrivate(this) )
{
    setFlags( QGraphicsItem::ItemClipsChildrenToShape );
    Q_D(PannableView);
    d->m_orientation    = orientation;
    d->m_pannableWidget = new PannableWidget( this );

    // TODO: Add here grabGesture() calls and make this widget to use QGestures
}


PannableView::~PannableView()
{
    delete d_ptr;
    d_ptr = 0;
}


void PannableView::setWidget( QGraphicsWidget * widget )
{
    Q_D(PannableView);    
    d->m_pannableWidget->setWidget( widget );    
    d->recalculateGeometry();
}

bool PannableView::isScrolling() const
{
    Q_D(const PannableView);
    return d->isScrolling();
}

void PannableView::moveToBegin()
{
    Q_D(PannableView);
    return d->moveToBegin();
}

void PannableView::moveToEnd()
{
    Q_D(PannableView);
    return d->moveToEnd();
}


bool PannableView::sceneEvent ( QEvent * event )
{
    Q_D(PannableView);
    switch( event->type()){

    case QEvent::GraphicsSceneMousePress:                
        d->recalculateGeometry(); // In a case if pannable widget size has changed
        return d->mousePressEvent(static_cast<QGraphicsSceneMouseEvent*>(event));

    case QEvent::GraphicsSceneMouseMove:
        return d->mouseMoveEvent(static_cast<QGraphicsSceneMouseEvent*>(event));

    case QEvent::GraphicsSceneMouseRelease:
        return d->mouseReleaseEvent(static_cast<QGraphicsSceneMouseEvent*>(event));

    default:        
        return false;
    }
}


void PannableView::enableContinuousScrolling( bool enable )
{    
    Q_D(PannableView);
    d->m_enableContinuousScrolling = enable;
}

Qt::Orientations PannableView::orientation() const
{
    Q_D(const PannableView);
    return d->m_orientation;
}

void PannableView::enableScrollIndicator( bool enable )
{
    Q_D(PannableView);
    d->m_enableIndicator = enable;
}

void PannableView::moveToItem( QGraphicsItem * item)
{
    QList<QGraphicsItem *> items = scene()->items();
    if ( !items.contains( item )){
        return;
    }

    // Move view to the center of the item
    Q_D(PannableView);    
    QSizeF viewSize = geometry().size();
    QSizeF itemSize = item->boundingRect().size();
    QPointF offset( (viewSize.width() - itemSize.width()) / 2, (viewSize.height() - itemSize.height()) / 2 );
    d->moveTo( item->pos()  - offset );
}

void PannableView::setOrientation(Qt::Orientations orientation )
{
    Q_D(PannableView);
    d->m_orientation = orientation;    
    d->recalculateGeometry();
}
