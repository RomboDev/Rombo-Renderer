/*
 * _test_pannable.cpp
 *
 *  Created on: Dec 14, 2013
 *      Author: max
 */


#ifndef SNAPSHOTSVIEWER_H_
#define SNAPSHOTSVIEWER_H_


#include <QApplication>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsWidget>
#include <QGraphicsGridLayout>
#include <QGraphicsLinearLayout>
#include <QGraphicsSceneMouseEvent>

#include <./source/glviewer/snapshot/pannableview.h>
#include <./source/glviewer/IGLViewerDevice.h>
#include <QPen>
#include <QPainter>
#include <QtDebug>
#include <QDir>
#include <QPixmap>
#include <QSplitter>
#include <QSize>
#include <QVector>


// ////////////////////////////////////////////////////////////////////////////////////////////////////////////
class SnapshotItem: public QGraphicsWidget
{
public:
	SnapshotItem (QString const & pixmap, QString const & name, int iHeight, QGraphicsItem * parent=0);
	virtual ~SnapshotItem();

    virtual void paint ( QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget = 0 );

protected:
    virtual void mousePressEvent ( QGraphicsSceneMouseEvent * mouseEvent );
    virtual void mouseMoveEvent ( QGraphicsSceneMouseEvent * mouseEvent );
    virtual void mouseReleaseEvent ( QGraphicsSceneMouseEvent * mouseEvent );

private:
    bool m_selected;
    QSize m_size;
    QPixmap * m_pxm;
};


// ////////////////////////////////////////////////////////////////////////////////////////////////////////////
class SnapshotsContainer : public QGraphicsWidget
{
    Q_OBJECT
public:
    SnapshotsContainer ( int iWidth, int iHeight, QGraphicsItem * parent = 0 )
    : QGraphicsWidget( parent ), m_view(NULL), m_widget(NULL), m_layout(NULL), m_size(QSize(iWidth,iHeight))
	{
		m_layout = new QGraphicsLinearLayout ();
		m_layout->setSpacing (8);
		m_layout->setOrientation (Qt::Horizontal);


		QDir dir("/home/max/git/rombo/source/glviewer/snapshot/thumbs/");
		QStringList files = dir.entryList(QStringList() << "*.jpg");

		int fileCount = files.count();
		int row = 0;
		int col = 0;
		int counter = 0;
		for( int i=0;i< fileCount * 10;i++)
		{
			SnapshotItem * iItem = new SnapshotItem (dir.path()+"/"+files.at(i % fileCount), QString::number(counter++), iHeight);
			m_snapitems.push_back (iItem);

			m_layout->addItem (iItem);
			//m_layout->addItem( new SnapshotItem(dir.path()+"/"+files.at(i % fileCount), QString::number(counter++)),
			//									row, col++%3, Qt::AlignHCenter );
			//m_layout->addItem( iItem, row, col++%3, Qt::AlignHCenter );
			//if ( col % 3  == 0 ) ++row;
		}


		m_view = new PannableView (Qt::Horizontal, m_size.width(), m_size.height(), this );
		m_view->setPos(0,0);

		m_widget = new QGraphicsWidget ();
		m_widget->setLayout( m_layout );

		m_view->setWidget (m_widget);

		m_layout->invalidate();
		m_layout->activate();

		connect (	m_view, SIGNAL 	(scrolling (int)),
					this, 	SLOT 	(viewScrolling (int)) );
	}

    virtual ~SnapshotsContainer() { qDebug()<<"Deleting Snapshots Container!!"; delete m_view; }

signals:
	void widgetScrolling (int state);

protected slots:
	void viewScrolling(int state) { emit widgetScrolling (state); }

private:
    QSize m_size;
    QVector<SnapshotItem*> 	m_snapitems;
    QGraphicsWidget * 		m_widget;
    QGraphicsLinearLayout * m_layout;
    PannableView * 			m_view;
};


// ////////////////////////////////////////////////////////////////////////////////////////////////////////////
class SnapshotsViewer : public QGraphicsView
{
    Q_OBJECT
public:
    SnapshotsViewer (QWidget *parent = 0)
    	: m_parent(parent), m_scene(NULL), m_snapscontainer(NULL), m_lastin(-1), m_initialized(false) {}
    virtual ~SnapshotsViewer () { qDebug()<<"Deleting Snapshots Viewer!!"; }

    void initialize (int inY)
    {
    	if(inY)
    	{
			qDebug()  << "SnapshotsViewer->size: " << this->width() << ", " << this->height();
			qDebug()  << "SnapshotsViewer->QSplitter->size: " << m_parent->width() << ", " << m_parent->height();
			if(m_scene) delete m_scene;

			int tHeight = m_parent->height()-inY;
			qDebug()  << "SnapshotsViewer->tHeight: " << tHeight;

			m_snapscontainer = new SnapshotsContainer(this->width(), tHeight);
			m_scene = new QGraphicsScene(0,0, this->width(), tHeight);
			m_scene->addItem (m_snapscontainer);

			this->setScene (m_scene);

			this->setRenderHints (QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
			this->setHorizontalScrollBarPolicy (Qt::ScrollBarAlwaysOff);
			this->setVerticalScrollBarPolicy (Qt::ScrollBarAlwaysOff);
			this->setFrameStyle (QFrame::NoFrame);
			this->resize (this->width(), tHeight);

			//m_size = QSize(this->width(), tHeight);
			m_size.setWidth (this->width());
			m_size.setHeight (tHeight);

			connect (	m_snapscontainer, 	SIGNAL 	(widgetScrolling (int)),
						this, 				SLOT 	(snapContainerScrolled (int)) );

			this->show();

    	}else
    	{
    		delete m_scene;
    		m_scene = NULL;
    	}
    }

signals:
    void devicePainting (int ispainting);
    void snapContainerScrolling (int state);

public slots:
	void initializing (int in)
	{
		qDebug() << "Snapshots viewer initializing !!! " << in;

		m_initialized = true;
		m_lastin = in;
		initialize (m_lastin);
	}
	void snapContainerScrolled (int state) {
		qDebug() << "SnapshotsViewer->SnapContainer->Scrolling: " << state;
		if(state) emit devicePainting (2); //stop just the renderer
		else emit devicePainting (0);
	}

protected:
	void resizeEvent( QResizeEvent* e)
	{
		int tHeight = m_parent->height()-m_lastin;

		qDebug() << "Snapshots viewer resizing called ";
		qDebug() << "    is initialized: " << m_initialized;
		qDebug() << "    last resized height: " << tHeight;
		qDebug() << "    last size: " << m_size.width() << ", " << m_size.height();
		qDebug() << "    event size: " << e->size().width() << ", " << e->size().height();

		if(!m_initialized && abs(tHeight - e->size().height()) > 2 )
		{
			qDebug() << "Snapshots viewer resizing H ! " << e->size().height() << ", " << m_parent->height() - e->size().height();
			m_lastin = m_parent->height() - e->size().height();
			initialize (m_lastin);
			e->accept();
		}

		else if(!m_initialized && m_size.width() !=  e->size().width())
		{
			qDebug() << "Snapshots viewer resizing W ! ";
			initialize (m_lastin);
			e->accept();
		}

		else
		{
			m_initialized = false;
		}
	}

private:
	int m_lastin;
	bool m_initialized;
	QSize m_size;
	QGraphicsScene * m_scene;
	SnapshotsContainer * m_snapscontainer;
    QWidget * m_parent;
};


#endif // SNAPSHOTSVIEWER_H_
