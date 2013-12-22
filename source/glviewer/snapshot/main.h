#ifndef MAIN_H
#define MAIN_H

#include <QGraphicsWidget>
#include <QPixmap>


class PannableTest : public QGraphicsWidget
{
    Q_OBJECT
public:
    PannableTest( QGraphicsItem * parent = 0 );

    virtual ~PannableTest();

    virtual void paint ( QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget = 0 );

private:
    QPixmap m_pixmap;
};


#endif // MAIN_H
