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


#ifndef PANNABLEVIEW_P_H
#define PANNABLEVIEW_P_H
#include <QObject>
#include <QTime>
#include <QPointF>
#include <QList>
#include <QGraphicsWidget>
#include <QGraphicsLinearLayout>

class QTimeLine;
class QGraphicsSceneMouseEvent;
class PannableView;
class PannableWidget;


class PannableViewPrivate:public QObject{

    Q_OBJECT
public:
    //! Constructor.
    //! \param parent Parent widget
    PannableViewPrivate( PannableView * parent );

    //! Dtor.
    virtual ~ PannableViewPrivate();

    //! Start scrolling
    //! \scroll the duration time. This is in milliseconds
    void startScrolling( int duration = 1000000 );

    //! Stop scrolling.
    //! \clear if this is set to true, the current position is cleaned.
    void stopScrolling( int clear=true);

    //! Handle mouse press event.
    //! \param event The event.
    //! \return true if event is handled, otherwise return false.
    bool mousePressEvent( QGraphicsSceneMouseEvent * event );

    //! Handle mouse move event.
    //! \param event The event.
    //! \return true if event is handled, otherwise return false.
    bool mouseMoveEvent( QGraphicsSceneMouseEvent * event );

    //! Handle mouse release event.
    //! \param event The event.
    //! \return true if event is handled, otherwise return false.
    bool mouseReleaseEvent( QGraphicsSceneMouseEvent *event );

    //! Handle tapping.
    //! \paran event The event.
    //! \return true if success, false otherwise
    bool tap(  QGraphicsSceneMouseEvent * event );

    //! Handle swipe gesture.
    //! \param startPoint The point where mouse was pressed
    //! \param endPoint The point where mouse was released
    //! \return true on success, false on failure
    bool swipe( QPointF const & startPoint, QPointF const &endPoint );

    //! Method for animating the collision with one of the four ends.
    //! \param points A list point which are used for animation
    //! \param duration A duration for animation. Default is 800 ms.
    void animateEnd(QList<QPointF> const & points, int duration=800 );

    void storeSwipeStyle( QPointF const &first, QPointF const &second );

    //! Check if end has reached.    
    //! \return true if end has been reached, return false otherwise.
    bool endReached();

    //! Move current position to the begining.
    void moveToBegin();

    //! Move current position to the end.
    void moveToEnd();

    //! \return true if scrolling is active, false otherwise.
    bool isScrolling() const;

    //! Move the current position to \pos.
    //! \pos New position
    void moveTo(QPointF const & pos );

    //! Calculate the pannable widget geometry and set a new min max
    //! values so that if e.g. widget size changes the panning should
    //! still work.
    void recalculateGeometry();

Q_SIGNALS:
    // Signal for stopping the animation
    void stopAnimation();

private Q_SLOTS:
    //! A slot for scrolling.
    //! \param value not used for anything
    void scroll( qreal value );

    //! A slot for emitting posChanged() signal of parent
    void emitPositionChanged();


public:
    enum SwipeStyle{
        Horizontal = 0,
        Vertical,
        Diagonal
    };

    // Members
    PannableView          *q_ptr;
    qreal                  m_deltaX;
    qreal                  m_deltaY;
    qreal                  m_vX;
    qreal                  m_vY;
    int                    m_minX;
    int                    m_minY;
    int                    m_maxX;
    int                    m_maxY;
    QTime                  m_time;
    QTimeLine            * m_scrollingTimeLine;
    Qt::Orientations       m_orientation;
    bool                   m_skipTapGesture;    
    PannableWidget       * m_pannableWidget;
    bool                   m_enableIndicator;    
    bool                   m_enableContinuousScrolling;    
    SwipeStyle             m_swipeStyle;

    Q_DECLARE_PUBLIC(PannableView)

/*#
define Q_DECLARE_PUBLIC(Class)                                    \
    inline Class* q_func() { return static_cast<Class *>(q_ptr); } \
    inline const Class* q_func() const { return static_cast<const Class *>(q_ptr); } \
    friend class Class;
*/
//#define Q_Q(Class) Class * const q = q_func()
};



//! \class PannableWidget
//! \brief PannableWidget is just a simple wrapper which provides shape information used by clipping.
//! This class is used internally only by PannableViewport.
//! TODO: Figure out what to do with this widget.
class PannableWidget: public QGraphicsWidget{    
public:

    //! PannableWidget constructor
    //
    //! \param parent optional parent
    PannableWidget( QGraphicsObject * parent = 0 );

    //! Destructor
    virtual ~PannableWidget();

    //! Shape method in order to make children to clip to shape
    //! \return instance shape in QPainterPath object.
    virtual QPainterPath shape () const;

    //! Set a widget for this widget
    virtual void setWidget( QGraphicsWidget * widget );

private:
    QGraphicsLinearLayout * m_layout;
};


#endif // PANNABLEVIEW_P_H



