/*
 * renderregion.h
 *
 *  Created on: Dec 20, 2012
 *      Author: max
 *
 * TODO:
 *
 * - fix rregion overflow when view is resized (CRASH!)
 * - fix/inspect new/delete for rregion, corner and mid pts classes !!!!!!!!!!!!!
 * - fix rregion x0 Y0 border defect (not mantaining the correct distance offset)
 * - fix rregion : for the intial rect drawing there's not a min bound to limit rect (as with ctrl pts)
 * - fix LEFTHANDCOORDS drag box bug
 *
 */

#ifndef RENDERREGION_H_
#define RENDERREGION_H_


//#include <QtGui>
#include <QApplication>

#include "glviewer.h"
#include"renderdevice.h"


#define GFXVIEW_xxx

#ifdef GFXVIEW
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#endif
QT_FORWARD_DECLARE_CLASS(QBypassWidget)


class GLViewer;
class RenderRegion;

//***********************************************************************************************************//
// RenderRegion	Mid Side Points *****************************************************************************//
//***********************************************************************************************************//
class RRDivider : public QObject
{
    Q_OBJECT
public:

    RRDivider (RenderRegion * rregion);
    ~RRDivider (){ m_rregion=NULL; }

    void setDivider(const QRectF &rr_bbox, bool isReset=true, const QPointF& offset=QPointF(0,0));
    void paint (QPainter* iPainter);

    inline int getDivPixels(){ return m_rr_divpixels; }

protected:
    bool eventFilter (QObject *object, QEvent *event);

private:
    QLineF m_rr_divline;
    QPointF m_rr_linecenter;

    int m_rr_divpixels;
    bool m_isMovingDivider;

    RenderRegion *m_rregion;
};


//***********************************************************************************************************//
// RenderRegion	Mid Side Points *****************************************************************************//
//***********************************************************************************************************//
class RRSlotPoints : public QObject
{
    Q_OBJECT
public:

    struct RRSlot
    {
        enum {
        	EMPTY,
        	FILLED,
        	ACTIVE,
        	INACTIVE	//never used for slots
        };				//just while emitting change

        RRSlot(): state(EMPTY){};
        RRSlot( QPointF iCenter): center(iCenter), state(EMPTY){};
        ~RRSlot(){};

    	QPointF center;
    	int	state;
    };

    RRSlotPoints (RenderRegion * rregion);
    ~RRSlotPoints ()
    {
    	m_rregion=NULL;

    	for (int i=0; i<m_slots.size(); ++i)
    		delete m_slots.at(i);
    	m_slots.clear();
    }

    void setPoints(const QRectF &rr_bbox, bool isReset=true, const QPointF& offset=QPointF(0,0));
    void paint (QPainter* iPainter);

    inline bool hasActiveSlot() {return m_activePoint >= 0 ? true : false;}
    void clearSlots();

protected:
    bool eventFilter (QObject *object, QEvent *event);

private:
    QVector<RRSlot*> m_slots;
    QPointF m_reset_slots;

    bool m_isTweakingSlots;
    int  m_activePoint;

    RenderRegion *m_rregion;
};


//***********************************************************************************************************//
// RenderRegion	Corner Points *******************************************************************************//
//***********************************************************************************************************//
class RRCornerPoints : public QObject
{
    Q_OBJECT
public:
    enum {
    	XALIGNED,
    	YALIGNED
    };

    RRCornerPoints (RenderRegion * rregion);
    ~RRCornerPoints (){ m_rregion=NULL; }

    void setPoints(const QRectF &rr_bbox);
    void paint (QPainter* iPainter);

protected:
    bool eventFilter (QObject *object, QEvent *event);

private:
    QPointF getPoint (int ip){ return m_points[ip]; }
    int getAlignedPoint (int axes, QPointF* o_pt);
    int getPivotPoint (QPointF* outP);

private:
    QVector<QPointF> m_points;
    bool m_isMovingPoints;
    int  m_activePoint;
    QPointF m_pivotPoint;
    int  m_nbPivotPt;
    int  m_dX;
    int  m_dY;
    int  m_offsX;
    int  m_offsY;

    RenderRegion *m_rregion;
};


//***********************************************************************************************************//
// RenderRegion	Mid Side Points *****************************************************************************//
//***********************************************************************************************************//
class RRMidPoints : public QObject
{
    Q_OBJECT
public:

    RRMidPoints (RenderRegion * rregion);
    ~RRMidPoints (){ m_rregion=NULL; }

    void setPoints(const QRectF &rr_bbox);
    void paint (QPainter* iPainter);

protected:
    bool eventFilter (QObject *object, QEvent *event);

private:
    QVector<QPointF> m_points;
    int  m_activePoint;
    bool m_isMovingPoints;

    RenderRegion *m_rregion;
};

//***********************************************************************************************************//
// RenderRegion	*********************************************************************************************//
//***********************************************************************************************************//
class RenderRegion : public IGLViewerDevice
{
    Q_OBJECT
    friend RRSlotPoints;
    friend RRCornerPoints;
    friend RRMidPoints;
    friend RRDivider;

signals:
	void rRegionHasChanged(int iX0, int iY0, int iX1, int iY1);
	void rrSlotHasChanged(int iSlot, int iState);

public:
#ifndef GFXVIEW
    RenderRegion(GLViewer *widget);
#else
    RenderRegion(QGraphicsScene *widget);
#endif
    ~RenderRegion ()
    {
    	delete m_slotpoints;
    	delete m_cornerpoints;
    	delete m_midpoints;
    	delete m_divider;
    	m_widget=NULL;
    }

    bool init ( bool iEnabled );
    void paint (QPainter* iPainter);

    bool hasInvalidatedContext ();
    void setHasInvalidatedContext (bool ientry);

    void hostResized ();
    void hostPanned ();

    __forceinline bool isPainting () const { return g_rr_paint;}
    __forceinline int getDividerPixels () const { return m_divider->getDivPixels(); }

    inline const QRectF& getBBoxRect(){ return g_rr_screen_rect; };

protected:
#ifndef GFXVIEW
    inline GLViewer* getHostWidget () const { return m_widget; }
#else
    inline QGraphicsScene* getHostWidget () const { return m_widget; }
#endif
    bool eventFilter (QObject *object, QEvent *event);

    inline bool isActive  () const { return g_renderregion; }
    inline bool isBlocked () const { return g_rr_isBlocked;}
    inline bool isOverBBox () const { return g_rr_isOverBBox;}

    inline bool isDragging () const { return g_rr_isDragging;}
    inline bool isDrawingPoints () const { return g_rr_isDrawing;}
    inline bool isMovingPoints () const { return g_rr_isMovingPoints;}
    inline bool isTweakingSlots () const { return g_rr_isTweakingSlots;}

    inline void setIsBlocked (bool isit) { g_rr_isBlocked = isit;}
    inline void setIsMovingPoints (bool isit) { g_rr_isMovingPoints = isit;}
    inline void setIsTweakingSlots (bool isit) { g_rr_isTweakingSlots = isit;}
    inline void setIsMovingDivider (bool isit) { g_rr_isMovingDivider = isit;}

    inline bool hasActiveSlot () { return m_slotpoints->hasActiveSlot(); }

    void rebuildBBox(QPointF ipt, int iact, int mode, bool forceScale=false);
    inline QSize getFullViewSize() const { QSize ts; ts.setWidth(g_width); ts.setHeight(g_height); return ts; }

    inline QSize getViewOffsetLR () const { return QSize (m_border_left, m_border_right); }
    inline QSize getViewOffsetTB () const { return QSize (m_border_top, m_border_bottom); }

    void resetBorders (int& startX, int& startY, int& stopX, int& stopY, int lOffset=1);

private slots:
	virtual void viewerInit ();
	//virtual void viewerResized (int iwidth, int iheight);
	virtual void viewerResized ();
	virtual void viewerPaint (QPainter* iPainter) { this->paint(iPainter); }

private:
    void paintPts (QPainter* iPainter)
    {
    	m_slotpoints->paint (iPainter);
		m_cornerpoints->paint (iPainter);
		m_midpoints->paint (iPainter);
		m_divider->paint (iPainter);
    }
    void setPts ()
    {
		m_cornerpoints->setPoints (g_rr_screen_rect);
		m_midpoints->setPoints (g_rr_screen_rect);
		//when update here look also to update in rebuildBBox
    }
    void setSlots (bool isReset=true, const QPointF& offset=QPointF(0,0))
    {
    	m_slotpoints->setPoints (g_rr_screen_rect, isReset, offset);
    	m_divider->setDivider (g_rr_screen_rect, isReset, offset);
    }

    void setBorders ();

    template<class QRECTx=QRectF, class QPOINTx=QPointF, typename TYPE=qreal>
    QRECTx remapRegion (const QRECTx& oRect, const QRECTx& iRect, const QRECTx& iRegion,
    					bool noWremap=false, bool noHremap=false);

	virtual void registerDevice ();

private:
    enum {
    	FROMCORNERPOINT,
    	FROMMIDPOINT
    };

	struct tVec4f { float x, y, z, w; };	//!< Util for PBOs

    //init states
	bool g_renderregion;
	bool g_rr_isInit;
	bool g_rr_isBlocked;
	bool g_rr_isOverBBox;

	//tweaking states
	bool g_rr_paint;
	bool g_rr_isDrawing;
	bool g_rr_isDragging;
	bool g_rr_isMovingPoints;
	bool g_rr_isTweakingSlots;
	bool g_rr_isMovingDivider;


	bool g_rr_invalidadedContext;
	bool g_rr_lockslots;
	bool g_rr_paintInfo;
	bool g_rr_doNotPaintQuads;

	bool g_host_panned;

	//bbox rect and pivot pt
	QRectF g_rr_screen_rect;
	QPointF g_rr_fb_start;
	QPointF g_rr_start_drag;

	//widget/fb
	int g_width;
	int g_height;

	int m_offs_top;
	int m_offs_bottom;
	int m_offs_left;
	int m_offs_right;

	int m_fbscene_width;
	int m_fbscene_height;

	int m_border_top;
	int m_border_bottom;
	int m_border_left;
	int m_border_right;

	//Pts
	RRSlotPoints * m_slotpoints;
	RRCornerPoints * m_cornerpoints;
	RRMidPoints * 	m_midpoints;
	RRDivider * m_divider;

	//host widget
#ifndef GFXVIEW
	GLViewer * m_widget;
#else
	QGraphicsScene* m_widget;
#endif
};

#endif /* RENDERREGION_H_ */
