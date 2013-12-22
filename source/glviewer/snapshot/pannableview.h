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


#ifndef PANNABLEVIEW_HH
#define PANNABLEVIEW_HH


#include <QGraphicsWidget>
#include <QPainterPath>
#include <QTimeLine>

class PannableViewPrivate;
class PannableWidgetPrivate;


//! \class PannableView
//! Class PannableView is a simple view for the PannableWidget which can contain any kind
//! of QGraphicsWidget based items. PannableView provide either vertical or horizontal
//! scrolling. It is possible to set both directions on and then PannableView scrolls to
//! the both directions. This view can be used for clipping part of the pannable area away
//! for example if panning in vertical it clips all the items that are outside of this
//! view's visible area.
//!
//! Usage:
//! // Create a layout or a QGraphicsWidget
//! QGraphicsLinearLayout * layout = new QGraphicsLinearLayout(Qt::Vertical);
//! for(int i=0; i< 1500; i++){
//!     layout->addItem( new MyCustomItem );
//! }
//!
//! QGraphicsWidget * widget = new QGraphicsWidget;
//! widget->setLayout( layout );
//!
//! PannableView * view = new PannableView(Qt::Vertical, 865,480 );
//! view->setWidget( widget );
//! QGraphicsScene scene(0,0,856,480);
//! scene.addItem( view );
//!
//! ...
//!
//!
//! TODO:
//! - provide a signal for notifying scrolling state: scrolling started, scrolling, scrolling ended
//! - Add a scroll indicator to indicate current scrolling position
//! - Make this class to use Qt's property system
//! - Let a user define if she wants some of the hardcoded animations to be executed or not
class PannableView : public QGraphicsWidget
{
    Q_OBJECT
public:


    //! Constructor. Create PannableView object, with specified properties.
    //! \param orientation Qt::Horizontal or Qt::Vertical. These enums are possible to be ORed also.
    //! \param width The width of the view
    //! \param height The height of the view
    //! \param parent optional parent
    PannableView( Qt::Orientations orientation, qreal width, qreal height, QGraphicsItem * parent = 0 );

    //! Constructor.
    //! \param orientation Qt::Horizontal or Qt::Vertical. These are possible to "or" also.
    //! \param parent An optional parent
    PannableView( Qt::Orientations orientation, QGraphicsItem * parent = 0 );

    //! Destructor
    virtual ~PannableView();

    //! Set a pannable widget to this view.
    //! NOTE: Make sure that this view is in the scene before adding the widget.
    //! \param widget A widget that with some content.
    void setWidget( QGraphicsWidget * widget );

    //! \return true if view is scrolling, false is returned otherwise
    bool isScrolling() const;

    //! Move current position to the begining
    void moveToBegin();

    //! Move current position to the end.
    void moveToEnd();

    //! Enable continous scrolling. This  means that when one of the ends of the pannable
    //! widget has been reached the widget moves to the opposite end.
    //! \param enable True if enabling continuous scrolling, false disables it.
    void enableContinuousScrolling( bool enable );

    //! Enable scroll indicator. Scroll indicator indicates current scrolling position.
    //! \param enable True if enabling scroll indicator, false disables it.
    void enableScrollIndicator( bool enable );

    //! \return The panning orientation.
    Qt::Orientations orientation() const;

public Q_SLOTS:
    //! Move pannable viewport to the \p item.
    //! \param item This item is centered to the view.
    void moveToItem( QGraphicsItem * item);

    //! Set orienation for PannableView. The \p orientation can be either vertical or
    //! horizontal or both.
    //! \param orientation Qt::Vertical or Qt::Horizontal or OR combination of these.
    void setOrientation( Qt::Orientations orientation );

protected:
    //! reimp
    virtual bool sceneEvent ( QEvent * event );

Q_SIGNALS:
    //! This signal is emitted when position of the pannable item has been changed. The density of how often this
    //!  signal is emitted can be changed with \see setPosChangedInterval(int interval)
    //! \param viewRect The rectangle of the viewport.
    void posChanged(QRectF const & viewRect );

    //! This signal is emitted when the view started(1) and stopped(0)
    void scrolling (int state);

    //! A signal for indicating tapping.
    //! \param point tapped position.
    void tapped(QPointF const & point );

private:
    // Private members
    PannableViewPrivate * d_ptr;
    Q_DECLARE_PRIVATE(PannableView);
    Q_DISABLE_COPY(PannableView);    
};


#endif
