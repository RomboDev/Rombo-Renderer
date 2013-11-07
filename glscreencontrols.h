/*
 * glscreencontrols.h
 *
 *  Created on: Jan 18, 2013
 *      Author: max
 */

#ifndef GLSCREENCONTROLS_H_
#define GLSCREENCONTROLS_H_


#include <QtGui>
QT_FORWARD_DECLARE_CLASS(QBypassWidget)


#include "glviewer.h"

#include <iostream> //temp for debug->cout

class GLViewer;
class OverlayItem;
class OverlayItemsController;

// ////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Interface to [sub]items custom builders
class OverlayItemsBuilder: public QObject
{
	Q_OBJECT
public:
	virtual ~OverlayItemsBuilder(){};
	virtual bool buildItems (	OverlayItemsController * const& iFactory,
								QVector<OverlayItem*> * const& iSettings, int iPos ) = 0;
};

// ////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Base Item
class OverlayItem : public QObject
{
    Q_OBJECT
    friend OverlayItemsController;

public:
    OverlayItem (OverlayItemsController * iFactory, int iID);
    OverlayItem(): m_factory(NULL), m_qpix_bck(NULL), m_qpix_over(NULL), m_qpix_icon (NULL) {}
    ~OverlayItem();

    void setID (int id) { m_id = id; }
    int getID () const { return m_id; }

    void setIsActive (bool active) { m_is_active = active; }
    bool isActive () const { return m_is_active; }

    void setIsDimmed (bool idim) { m_is_dimmed = idim; m_is_dimmed==true? m_opacity=0.8: m_opacity=1.0;}
    bool isDimmed () const { return m_is_dimmed; }

    int isHidden () const { return m_is_hidden; }

    void setIsOver (bool over) { m_is_over = over; }
    bool isOver () const { return m_is_over; }

    void setOpacity (qreal iStartOpacity, qreal iEndOpacity=-1.0 ) { m_opacity = iStartOpacity;
    																//this is mainly for animated items because
    																//only there we deal with m_end_opacity
    																//this->paint() uses the m_opacity
    																if(iEndOpacity!=-1) m_end_opacity = iEndOpacity;
    																else m_end_opacity = 0.8; /*force not fully opaque*/}
    qreal getOpacity () const { return m_opacity; }
    qreal getEndOpacity () const { return m_end_opacity; }

    void setPosition(const QRect & iPos){ m_qpix_rect_cur = m_qpix_rect_pos = iPos; }
    const QRect& getPosition () const { return m_qpix_rect_pos; }

    void setCurrentPos (const QRect & iPos) { m_qpix_rect_cur = iPos; };
    const QRect& getCurrentPos () const { return m_qpix_rect_cur; };

    void setBckPixmaps (const QString & iBck, const QString & iHover="", const QString & iIcon="");
    void setIconPixmap (const QString & iIcon);

    inline  OverlayItemsController * getFactory () const { return m_factory; }

    virtual void setIsHidden (int hideme) { m_is_hidden = hideme; }
    virtual void paint (QPainter* iPainter);

protected:
    virtual void reset () { m_qpix_rect_cur = m_qpix_rect_pos; setIsActive(false); }
    virtual bool eventFilter (QObject *object, QEvent *event);

    //virtual base
    virtual void animateForward (float iTimeElapsed) {}
    virtual void animateBackward (float iTimeElapsed) {}
    virtual void animateLonely (float iTimeElapsed) {}
    virtual void resetSubitems () {}
    virtual void resetHPosition (int hpos)
    {
    	QRect iPos (m_qpix_rect_cur.left(), hpos, m_qpix_rect_cur.width(), m_qpix_rect_cur.height());
    	m_qpix_rect_cur = iPos;

    	QRect iiPos (m_qpix_rect_pos.left(), hpos, m_qpix_rect_pos.width(), m_qpix_rect_pos.height());
    	m_qpix_rect_pos = iiPos;
    }

private:
    int m_id;
    bool m_is_active;
    bool m_is_dimmed;
    int m_is_hidden; //2 is forcehidden
    bool m_is_over;
    qreal m_opacity;
    qreal m_end_opacity;

    QRect m_qpix_rect_pos;
    QRect m_qpix_rect_cur;

    QPixmap * m_qpix_bck;
    QPixmap * m_qpix_over;
    QPixmap * m_qpix_icon;
    bool m_single_pixmap;

    OverlayItemsController * m_factory;
};

// ////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Animated item
class OverlayAnimItem: public OverlayItem
{
	Q_OBJECT
    friend OverlayItemsController;

public:
    OverlayAnimItem ():m_utime (0.0f) {}
	OverlayAnimItem (OverlayItemsController * iFactory, int iID)
	: OverlayItem (iFactory, iID), m_utime (0.0f) {};

    void setPositionEnd (const QRect & iPos);
    inline const QRect& getPositionEnd () const { return m_qpix_rect_end; };

protected:
    void animateForward (float iTimeElapsed);
    void animateBackward (float iTimeElapsed);
    inline QRect animate (float iTimeElapsed, const QRect& iStart, const QRect& iEnd);

    void setLastPos (const QRect & iPos) { m_pix_rect_last = iPos; };
    inline const QRect& getLastPos () const { return m_pix_rect_last; };

    inline bool isMoving () const { return (m_utime!=0.0 && m_utime!=1.0); }

    virtual void reset () { OverlayItem::reset(); m_pix_rect_last = getPosition(); }

    virtual void resetHPosition (int hpos)
    {
    	OverlayItem::resetHPosition(hpos);

    	QRect iPos (m_qpix_rect_end.left(), hpos, m_qpix_rect_end.width(), m_qpix_rect_end.height());
    	m_qpix_rect_end = iPos;

    	QRect iiPos (m_pix_rect_last.left(), hpos, m_pix_rect_last.width(), m_pix_rect_last.height());
    	m_pix_rect_last = iiPos;
    }

protected slots:
	void anim_has_ended (int state, int id)	{ emit global_anim_ended (state,id); }
signals:
	void global_anim_ended (int state, int id);

private:
    qreal m_utime;

    QRect m_pix_rect_last;
    QRect m_qpix_rect_end;
};

// ////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Group items (WIP)
class OverlayGroupItem: public OverlayAnimItem
{
	Q_OBJECT
public:
	OverlayGroupItem (OverlayItemsController * iFactory, int iID)
	: OverlayAnimItem (iFactory, iID) {}
	~OverlayGroupItem();

	void addItem (OverlayItem*item);

protected:
    virtual void paint (QPainter* iPainter);

private:
    QVector<OverlayItem*> m_subitems;
};


// ////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Animated Item with Subitems
class OverlayAnimSubItem: public OverlayAnimItem
{
	Q_OBJECT
    friend OverlayItemsController;

public:
	OverlayAnimSubItem (OverlayItemsController * iFactory, int iID, OverlayItemsBuilder * iBuilder)
	: OverlayAnimItem (iFactory, iID), m_builder (iBuilder), m_subitems_activated (false), m_anim_toalone_done (false) {};
	~OverlayAnimSubItem();

    void setPositionAlone (const QRect & iTarget) { m_qpix_rect_alone = iTarget; }
    inline const QRect& getPositionAlone () const { return m_qpix_rect_alone; };

signals:
    void subitems_ready (int iID);

protected:
    void buildSubItems ();
    void animateSubitems ();
    void finalizeSubitems ();

    inline QVector<OverlayItem*> * getSubitems () { return &m_subitems; }
    inline OverlayItem* getSubitem (int iID) { return m_subitems[iID]; }

    virtual void animateLonely (float iTimeElapsed);
    virtual void setIsHidden (int hideme);
    virtual void resetSubitems();
    virtual void reset () { OverlayAnimItem::reset(); this->resetSubitems (); }

    virtual void resetHPosition (int hpos)
    {
    	OverlayAnimItem::resetHPosition(hpos);

		QRect iPos (m_qpix_rect_alone.left(), hpos, m_qpix_rect_alone.width(), m_qpix_rect_alone.height());
		m_qpix_rect_alone = iPos;
    }

    virtual void paint (QPainter* iPainter);
    virtual bool eventFilter (QObject *object, QEvent *event);

protected slots:
	void anim_has_ended (int state, int id);

private:
    bool m_subitems_activated;
    bool m_anim_toalone_done;
    QRect m_qpix_rect_alone;

    OverlayItemsBuilder * m_builder;
    QVector<OverlayItem*> m_subitems;
};


// ////////////////////////////////////////////////////////////////////////////////////////////////////////////
//OverlayNumericPadDigit
class OverlayNumericPadDigit: public OverlayItem
{
	Q_OBJECT

signals:
	void digit_pressed (QChar);

public:
	OverlayNumericPadDigit (OverlayItemsController * iFactory, int iID)
	: OverlayItem (iFactory, iID),
	  m_digit (-1), m_qpix_custom (NULL) {}
	~OverlayNumericPadDigit () { if(m_qpix_custom!=NULL) delete m_qpix_custom; }

	void setDigit (QChar idig) { m_digit = idig; }
	QChar getDigit () const { return m_digit; }

    void setCustomPixmap (const QString & iclickPix) {	if(!iclickPix.isEmpty() || !iclickPix.isNull())
    													m_qpix_custom = new QPixmap (iclickPix); }

   virtual void paint (QPainter* iPainter);
   virtual bool eventFilter (QObject *object, QEvent *event);

private:
	QChar m_digit;
    QPixmap * m_qpix_custom;
};

//OverlayNumericPad
class OverlaySliderItem;
class OverlayNumericPad: public OverlayItem
{
	friend OverlaySliderItem;
	Q_OBJECT

signals:
	void dataChanged (int idata);
	void dataChanging (int idata,int idel=-1);

public:
	OverlayNumericPad (OverlayItemsController * iFactory, int iID)
	: OverlayItem (iFactory, iID),
	  m_dot_allowed (true),
	  m_dot_isthere (false),
	  m_dot_needed (false),
	  m_sign_allowed (true),
	  m_sign_isthere (false),
	  m_dec_digits (0),
	  m_cur_decdigits (0),
	  m_cur_intdigits(0),
	  m_data_changed (false),
	  m_not_hidden (false) {}
	~OverlayNumericPad () { destroyPad(); }

	bool isDecimal () const { return m_dot_allowed; }
	void setIsDecimal (bool isdec) { m_dot_allowed = isdec; }
	bool isSigned () const { return m_sign_allowed; }
	void setIsSigned (bool issig) { m_sign_allowed = issig; }
	int  getDecDigits () const { return m_dec_digits; }
	void setDecDigits (int idec) { m_dec_digits = idec; }

	void setFormat (bool allow_sign, bool allow_decimals, int n_decdigits)
	{
		m_sign_allowed = allow_sign;
		m_dot_allowed = allow_decimals;
		m_dec_digits = n_decdigits;
	}

	void setData (int idata, const QPoint& irange)
	{
		m_data = idata;
		m_vrange = irange;
	}
	void updateData (int idata)
	{
		m_data = idata;

		m_sign_isthere = false;
		if(idata<0) m_sign_isthere = true;

		m_cur_intdigits = 0;

		m_dot_needed = false;
		m_dot_isthere = false;
		if(m_dec_digits>0) m_dot_isthere = true;

		m_cur_decdigits = 0;

		m_hold_digits = "";
		m_data_changed = false;
	}

	void setHiddenPad (bool ihidden=true) { m_not_hidden = ihidden; }

	void buildPad ();
	void destroyPad ();
	void paintPad (QPainter* iPainter);

    virtual void paint (QPainter* iPainter)
    {
    	if(!m_not_hidden) return;

    	OverlayItem::paint(iPainter);

    	//paint numeric digit buttons
    	paintPad (iPainter);
    }
    virtual bool eventFilter (QObject *object, QEvent *event)
    {
		OverlayItem::eventFilter (object, event);

    	if(isOver() && event->type()==QEvent::MouseButtonPress && m_not_hidden)
    	{
    		for (int i=0; i<m_pad_buttons.size(); i++)
    			//do it manually, as returning true from here prevents
    			//pad_buttons respective eventfilter to be engaged
    			m_pad_buttons[i]->eventFilter (object,event);

    		//! < stop event propagation so that if we fall clicking
    		//! < between buttons it does not update camera or whatever
    		return true;
    	}
    	else
    	{
    		//map numeric keys to apped_digit event
    		if(event->type()==QEvent::KeyPress)
    		{
    			QKeyEvent *e = (QKeyEvent *) event;

    			if(e->key()==Qt::Key_0) 				{ append_digit ('0'); }
    			else if(e->key()==Qt::Key_1) 			{ append_digit ('1'); }
    			else if(e->key()==Qt::Key_2) 			{ append_digit ('2'); }
    			else if(e->key()==Qt::Key_3) 			{ append_digit ('3'); }
    			else if(e->key()==Qt::Key_4) 			{ append_digit ('4'); }
    			else if(e->key()==Qt::Key_5) 			{ append_digit ('5'); }
    			else if(e->key()==Qt::Key_6) 			{ append_digit ('6'); }
    			else if(e->key()==Qt::Key_7) 			{ append_digit ('7'); }
    			else if(e->key()==Qt::Key_8) 			{ append_digit ('8'); }
    			else if(e->key()==Qt::Key_9) 			{ append_digit ('9'); }
    			else if(e->key()==Qt::Key_Minus) 		{ append_digit ('-'); }
    			else if(e->key()==Qt::Key_Backspace) 	{ append_digit ('D'); }
    			else if(e->key()==Qt::Key_Escape)		{ append_digit ('C'); }
    			else if(e->key()==Qt::Key_Delete)		{ append_digit ('C'); }
    			else if(e->key()==Qt::Key_Enter)		{ append_digit ('X'); }
    			else if(e->key()==Qt::Key_Return)		{ append_digit ('X'); }
    			else if(e->key()==Qt::Key_Period)		{ append_digit ('.'); }
    			else { /*std::cout << "PRESSED KEY: " << e->key() << std::endl;*/ }
    		}

    		return false;
    	}
    }

protected slots:
	void append_digit (QChar idig);

private:
	bool m_dot_allowed;
	bool m_dot_isthere;
	bool m_dot_needed;

	bool m_sign_allowed;
	bool m_sign_isthere;

	int m_dec_digits;
	int m_cur_decdigits;
	int m_cur_intdigits;
	bool m_data_changed;

	bool m_not_hidden;

    QPoint m_vrange;
    int m_data;

    QVector<OverlayNumericPadDigit*> m_pad_buttons;
    QString m_hold_digits;
};


// ////////////////////////////////////////////////////////////////////////////////////////////////////////////
//OverlaySliderItem
class OverlaySliderItem: public OverlayAnimItem
{
	Q_OBJECT

signals:
	//void actionTriggered (int action);
	//void sliderPressed ();
	//void sliderReleased ();
	//void sliderMoved (int value);
	void dataChanged (int value, int decdigits, int ID);

public:
	OverlaySliderItem (OverlayItemsController * iFactory, int iID)
	: OverlayAnimItem (iFactory, iID),
	  m_num_pad (NULL), m_numpad_clicked (false), m_numpad_destroy (false), m_numpad_updating (false),
	  m_numpad_decdigits (-1),
	  m_override_cur_once (true), m_restore_cur_once (false),
	  m_over_cursor (false), m_draggin_cursor (false),
	  m_has_slider (true), m_show_numpad (true), m_force_repaint (false)
	{
		//set default bck pixmap
		if(!m_has_slider)
			setBckPixmaps (	"./images/gloverlay/slider_num_bck.png");
		else
			setBckPixmaps (	"./images/gloverlay/slider_bck.png");
	}
	~OverlaySliderItem () { if (m_num_pad!=NULL) clearNumericPad(); }

	void setData (const QString& iparamname, int idata, const QPoint& irange, int idecdigits=0)
	{
		m_paramname = iparamname;

		m_decdigits = idecdigits;
		m_data = idata;
		m_vrange = irange;

		if(m_data<m_vrange.x()) m_vrange.setX(m_data);
		if(m_data>m_vrange.y()) m_vrange.setY(m_data);

		m_dec_inv_pow = 1.0f / pow(10, m_decdigits);
	}

	bool hasDecimals () const { return m_decdigits>0; }
	int getDecDigitsNb () const { return m_decdigits; }
	bool hasNegativeRange () const { return m_vrange.x()<0; }

	void setHasNoSlider (bool ihasslider=false) { m_has_slider = ihasslider; }
	void setHasNoNumPad (bool ishowpad=false) { m_show_numpad = ishowpad; }

protected:

    virtual void resetHPosition (int hpos)
    {
    	OverlayAnimItem::resetHPosition(hpos);

    	clearNumericPad ();
    	m_force_repaint = true;
    }

    virtual void paint (QPainter* iPainter);
    virtual bool eventFilter (QObject *object, QEvent *event);

protected slots:
	//once the number has been finalized and passed here
	void dataPadChanged(int idata)
	{
		m_data = idata;

		//we cannot delete on the go the numpad, because we
		//fired this slot from there and there we need to return
		//do it on first paint() occasion
		m_numpad_destroy = true;
		m_numpad_updating = true;

		//reset dec paint stuff
		m_numpad_decdigits = -1;

		//!<broadcast data changed
		emit dataChanged (m_data, m_decdigits, getID());
	}
	//while composing the number from numpad
	void dataPadChanging(int idata, int idel)
	{
		if(idel!=-1){
			//Deleting decimals.. remaing to display: idel
			//reset dec paint stuff
			m_numpad_decdigits = idel;
		}

		//just update visually the data,
		//will broadcast it when confirm is pressed
		//std::cout << "dataPadChanging : " << idata << std::endl;
		m_data = idata;
		m_numpad_updating = true;
	}
	//when the slider or keys have been used while numpad open
	void updatePadData (int value)
	{
		if(m_num_pad)
		{
			//reset dec paint stuff
			m_numpad_decdigits = -1;
			m_num_pad->updateData (value);
		}
	}

private:
    int getXSliderFromValue (int val=-666, bool toInt=true );
    int getValueFromXSlider ();
    void buildNumericPad ();
	void clearNumericPad ();

private:
    enum {
    	HOFFSET = 7,
    	WOFFSET = 10
    };

    OverlayNumericPad * m_num_pad;
    bool m_numpad_clicked;
    bool m_numpad_destroy;
    bool m_numpad_updating;
    int m_numpad_decdigits;

    QRect m_numfield_rect;

    QRect m_activefield_rect;
    QRect m_bslider_rect;

    QRectF m_scursor_rect;
    bool m_over_cursor;
    bool m_draggin_cursor;
    bool m_override_cur_once;
    bool m_restore_cur_once;

    bool m_has_slider;
    bool m_show_numpad;
    bool m_force_repaint;

    QString m_paramname;

    int m_decdigits;
    float m_dec_inv_pow;

    QPoint m_vrange;
    int m_data;
};


// ////////////////////////////////////////////////////////////////////////////////////////////////////////////
//OverlayButtonItem
class OverlayButtonItem: public OverlayAnimItem
{
	Q_OBJECT

signals:
	void button_pressed (bool isClicked, int ID);

public:
	OverlayButtonItem (OverlayItemsController * iFactory, int iID)
	: OverlayAnimItem (iFactory, iID), m_qpix_click (NULL) {}
	~OverlayButtonItem () { if(m_qpix_click!=NULL) delete m_qpix_click; }

    void setClickedPixmap (const QString & iclickPix) {	if(!iclickPix.isEmpty() || !iclickPix.isNull())
    														m_qpix_click = new QPixmap (iclickPix); }
    void setParamName (const QString & ipname) { m_paramname = ipname; }

protected:
    virtual void paint (QPainter* iPainter);
    virtual bool eventFilter (QObject *object, QEvent *event);

private:
    QString m_paramname;
    QPixmap * m_qpix_click;
};


// ////////////////////////////////////////////////////////////////////////////////////////////////////////////
//OverlayBoolItem
class OverlayBoolItem: public OverlayAnimItem
{
	Q_OBJECT

signals:
	void button_pressed (bool isClicked, int ID);

public:
	OverlayBoolItem (OverlayItemsController * iFactory, int iID)
	: OverlayAnimItem (iFactory, iID), m_qpix_click (NULL), m_bool_data (false) {}
	~OverlayBoolItem () { if(m_qpix_click!=NULL) delete m_qpix_click; }

    void setClickedPixmap (const QString & iclickPix) {	if(!iclickPix.isEmpty() || !iclickPix.isNull())
    														m_qpix_click = new QPixmap (iclickPix); }
    void setData (bool idata) { m_bool_data = idata; }

protected:
    virtual void paint (QPainter* iPainter);
    virtual bool eventFilter (QObject *object, QEvent *event);

private:
    QPixmap * m_qpix_click;
    bool m_bool_data;
};



// ////////////////////////////////////////////////////////////////////////////////////////////////////////////
//OverlayNavigatorItem
class OverlayNavigatorItem: public OverlayAnimItem
{
	Q_OBJECT

public:
	OverlayNavigatorItem (OverlayItemsController * iFactory, int iID)
	: OverlayAnimItem (iFactory, iID),
	  m_force_blockme (-1), m_rr_isdragging (false), m_renderer_alredy_paused (false),
	  m_subregion (QRect(200,100,70,70)) {}
	~OverlayNavigatorItem () {}

	qreal remapBoxWidth (const QSizeF& isize, int hfix=70); //70 is the standard height TODO:ENUM IT !!!!!!!

protected:
    virtual void paint (QPainter* iPainter);
	virtual bool eventFilter (QObject *object, QEvent *event);

protected slots:
	void global_anim_ended_slot (int state, int id);

public slots:
	void factoryResized ();

private:
	int m_force_blockme;
    bool m_rr_isdragging;
    bool m_renderer_alredy_paused;
    QRectF m_subregion;
};





// ////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Controller, where we install the various items
class OverlayItemsController : public IGLViewerDevice
{
    Q_OBJECT
    friend OverlayItem;
    friend OverlayAnimItem;
    friend OverlayGroupItem;
    friend OverlayAnimSubItem;
    friend OverlaySliderItem;
    friend OverlayNumericPad;
    friend OverlayNavigatorItem;

public:

    enum LAYOUTTYPE
    {
    	BOTTOMLEFT = 0,
    	BOTTOMRIGHT,
    	TOPLEFT,
    	TOPRIGHT
    };

    //OverlayItemsController () {}
	#ifndef GFXVIEW
    OverlayItemsController (GLViewer *widget, OverlayItemsBuilder * iBuilder);
	#endif
    OverlayItemsController (QGraphicsScene *widget, OverlayItemsBuilder * iBuilder);
    ~OverlayItemsController();

    inline bool isEnabled () const { return m_is_enabled; }

    inline bool isPainting () const { return m_is_painting; }

    inline int getParentHeight () const;
    inline int getParentWidth () const;

    inline QVector<OverlayItem*> * getItems () { return &m_ctrls; }
    inline QVector<OverlayItem*> * getSubitems () const { return m_host_ctrls; }
    inline OverlayItem* getItem (int iID) { return m_ctrls[iID]; }
    inline OverlayItem* getSubitem (int iID) const { return (*m_host_ctrls)[iID]; }

    void setBckPixmaps (const QString & iBck);

    void setLayout (int ilay);
    void setPositionFromLayout (int ilay);
    void setItemsStartingPos (int ipos) { m_items_startpos = ipos; }

    void setItemsHasNoSubItems (bool isubitems=true) { m_hasno_subitems = isubitems;}
    void setDeleteItemsAtToBackAnim (bool idel=true) { m_deleteon_toback_anim = idel; }

    void reset ();

	#ifndef GFXVIEW
	inline GLViewer * getParentWidget () const;
	#else
	inline QGraphicsScene const * getParentWidget () const { return m_widget; }
	#endif

    void paint (QPainter* iPainter);

signals:
	void anim_ended (int, int);
	void items_ready ();
	void resized ();

protected:

    enum
    {
		ANINSTEP = 20,
		ANIMTIME = 160
    };
    enum ANIMSTATES
    {
    	TODISPLAY = 0,
    	TOBACK,
    	TOALONE,
    	TOHOST
    };
    enum ANIMMODES
    {
    	REGULAR = 0,
    	HOST,
    	TIMERONLY
    };

    void setIsPainting (bool ipaint=true)
    {
    	m_is_painting = ipaint;

    	emit devicePainting (ipaint);	//!< emit painting signal
    }

    void requestAnimation (int iID, bool isReDisplay=false);
    void requestHostAnimation (int iID, QVector<OverlayItem*> * iQVectorItems);

    bool getNumPadSlotFilled () const { return m_numpad_slot_filled; }
    void setNumPadSlotFilled (bool ipad) { m_numpad_slot_filled = ipad; }

    bool eventFilter (QObject *object, QEvent *event);
    void installEventFilter (OverlayItem* iItem);

private slots:
    void anim_ctrl();

	virtual void viewerInit () {};
    //virtual void viewerResized (int iwidth, int iheight);
    virtual void viewerResized ();
	virtual void viewerPaint (QPainter* iPainter) { this->paint(iPainter); }

private:
    virtual void registerDevice ();

private:

    QSize m_widget_size;

    bool m_is_enabled;
    bool m_is_painting;
    int m_anim_cur_item;
    int m_anim_last_item;
    bool m_anim_todisplay;
    int m_anim_mode;
    int m_anim_state;

    bool m_deleteon_toback_anim;

    bool m_numpad_slot_filled;

    int m_layout_type;
    int m_items_startpos;

    bool m_hasno_subitems;

    QPixmap * m_qpixmap;
    QRect m_qpix_rect;


    QTimer *m_timer;
    int m_elapsed;

    OverlayItemsBuilder * m_builder;
    QVector<OverlayItem*> m_ctrls;
    QVector<OverlayItem*> * m_host_ctrls;

	//host widget
	#ifndef GFXVIEW
    GLViewer * m_widget;
	#else
    QGraphicsScene * m_widget;
	#endif
};



// TO BE MOVED ////////////////////////////////////////////////////////////////////////////////////////////////



// Renderer settings //////////////////////////////////////////////////////////////////////////////////////////

// custom rendersettings subitems builder
class OverlayRenderSettingsBuilder: public OverlayItemsBuilder
{
public:
	bool buildItems (OverlayItemsController * const& iFactory, QVector<OverlayItem*> * const& iSubitems, int iPos);
};

// render settings class
class OverlayRendererSettingsItem: public OverlayAnimSubItem
{
	Q_OBJECT
public:
	OverlayRendererSettingsItem (OverlayItemsController * iFactory, int iID, OverlayItemsBuilder * iBuilder)
	: OverlayAnimSubItem (iFactory, iID, iBuilder) {};
	OverlayRendererSettingsItem (OverlayItemsController * iFactory, int iID, OverlayItemsBuilder * iBuilder,
							   const QRect & iPos, const QRect & iEnd, const QRect & iTarget)
	: OverlayAnimSubItem (iFactory, iID, iBuilder)
	{
		setPosition (iPos);
		setPositionEnd (iEnd);
		setPositionAlone (iTarget);

		///////////////////////////////////////////////////////////
		connect (	this, SIGNAL 	(subitems_ready	(int)), 		//this comes from base class (OverlayAnimSubItem)
					this, SLOT 		(bind_subitems	(int)) );		//calling this specialized slot
	};
	~OverlayRendererSettingsItem ()
	{
		disconnect (	this, SIGNAL 	(subitems_ready	(int)),		//probably not needed
						this, SLOT 		(bind_subitems	(int)) );
	}
private slots:
	void bind_subitems (int iID);
	void valueChanged_slot (int data, int decdigits, int id);
};



// Camera settings ////////////////////////////////////////////////////////////////////////////////////////////

// custom rendersettings subitems builder
class OverlayCameraSettingsBuilder: public OverlayItemsBuilder
{
public:
	bool buildItems (OverlayItemsController * const& iFactory, QVector<OverlayItem*> * const& iSubitems, int iPos);
};

// render settings class
class OverlayCameraSettingsItem: public OverlayAnimSubItem
{
	Q_OBJECT
public:
	OverlayCameraSettingsItem (OverlayItemsController * iFactory, int iID, OverlayItemsBuilder * iBuilder,
							   const QRect & iPos, const QRect & iEnd, const QRect & iTarget)
	: OverlayAnimSubItem (iFactory, iID, iBuilder),
	  m_rr_active (false), m_cammouse_mode (0),
	  m_cammouse_clickX (0), m_cammouse_clickY (0), m_cammouse_tracking (false),
	  m_active_id (-1), m_in_action (false)
	{
		setPosition (iPos);
		setPositionEnd (iEnd);
		setPositionAlone (iTarget);

		///////////////////////////////////////////////////////////
		connect (	this, SIGNAL 	(subitems_ready	(int)), 		//this comes from base class (OverlayAnimSubItem)
					this, SLOT 		(bind_subitems	(int)) );		//calling this specialized slot
	};
	~OverlayCameraSettingsItem ()
	{
		disconnect (	this, SIGNAL 	(subitems_ready	(int)),		//probably not needed
						this, SLOT 		(bind_subitems	(int)) );
	}

protected:
	void drawInstructions(QPainter *painter, int context, int width, int height);
	void resetActionButton ();

	virtual void paint (QPainter* iPainter);
    virtual bool eventFilter (QObject *object, QEvent *event);

private slots:
	void bind_subitems (int iID);
	void valueChanged_slot (int data, int decdigits, int id);
	void button_pressed_slot (bool pressed, int id);

private:
	bool m_rr_active;
	int m_cammouse_mode;
	int m_cammouse_clickX;
	int m_cammouse_clickY;
	bool m_cammouse_tracking;

	int m_active_id;
	bool m_in_action;
};



// Tonemapper settings ////////////////////////////////////////////////////////////////////////////////////////

// custom rendersettings subitems builder
class OverlayTonemapperSettingsBuilder: public OverlayItemsBuilder
{
public:
	bool buildItems (OverlayItemsController * const& iFactory, QVector<OverlayItem*> * const& iSubitems, int iPos);
};

// render settings class
class OverlayTonemapperSettingsItem: public OverlayAnimSubItem
{
	Q_OBJECT
public:
	OverlayTonemapperSettingsItem (OverlayItemsController * iFactory, int iID, OverlayItemsBuilder * iBuilder,
							   const QRect & iPos, const QRect & iEnd, const QRect & iTarget)
	: OverlayAnimSubItem (iFactory, iID, iBuilder)
	{
		setPosition (iPos);
		setPositionEnd (iEnd);
		setPositionAlone (iTarget);

		///////////////////////////////////////////////////////////
		connect (	this, SIGNAL 	(subitems_ready	(int)), 		//this comes from base class (OverlayAnimSubItem)
					this, SLOT 		(bind_subitems	(int)) );		//calling this specialized slot
	};
	~OverlayTonemapperSettingsItem ()
	{
		disconnect (	this, SIGNAL 	(subitems_ready	(int)),		//probably not needed
						this, SLOT 		(bind_subitems	(int)) );
	}
private slots:
	void bind_subitems (int iID);
	void valueChanged_slot (int data, int decdigits, int id);
	void button_pressed_slot (bool pressed, int id);
};



// renderer controller ////////////////////////////////////////////////////////////////////////////////////////

// renderer controller item builder
class OverlayRendererCtrlsBuilder: public OverlayItemsBuilder
{
public:
	bool buildItems (OverlayItemsController * const& iFactory, QVector<OverlayItem*> * const& iItems, int iPos);
};


// renderer bar controller
class OverlayRendererCtrls: public OverlayItemsController
{
	Q_OBJECT
public:
	OverlayRendererCtrls (GLViewer *iWidget)
	: OverlayItemsController(iWidget, new OverlayRendererCtrlsBuilder())
	{}

private slots:
    void slot_items_ready () {}
	void slot_subitems_ready (int iID)
	{
		/*std::cout << "Subitems READY: " << iID << std::endl;
		QVector<OverlayItem*> * tt = getSubitems ();
		OverlayItem* myItem = getSubitem (0);
		std::cout << tt->size() << std::endl;
		std::cout << myItem->getCurrentPos().topLeft().x() << std::endl;*/
	}
};




///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////




///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Framebuffer controls ///////////////////////////////////////////////////////////////////////////////////////




// Main controller ////////////////////////////////////////////////////////////////////////////////////////////

// main controller item builder
class OverlayFramebufferCtrlsBuilder: public OverlayItemsBuilder
{
public:
	bool buildItems (OverlayItemsController * const& iFactory, QVector<OverlayItem*> * const& iItems, int iPos);
};


// main bar controller
class OverlayFramebufferCtrls: public OverlayItemsController
{
	Q_OBJECT
public:
	OverlayFramebufferCtrls (GLViewer *iWidget)
	: OverlayItemsController(iWidget, new OverlayFramebufferCtrlsBuilder())
	{

		///////////////////////////////////////////////////////////
		connect (	this, SIGNAL 	(items_ready ()),
					this, SLOT 		(slot_items_ready ()) );
		connect (	this, SIGNAL 	(anim_ended	(int,int)),
					this, SLOT 		(slot_items_endanimready (int,int)) );

		connect (	this, SIGNAL 	(resized ()),
					this, SLOT 		(slot_resized ()) );
	}

private slots:
	void button_pressed_slot (bool pressed, int id);
	void valueChanged_slot (int data, int decdigits, int id);

    void slot_items_ready ()
    {
    	for (int i=0; i<getItems()->size(); i++)
    	{
    		if(i<2)
    		{
    			connect (	getItem(i), 	SIGNAL 	( dataChanged (int,int, int)),
    						this, 			SLOT 	( valueChanged_slot (int,int, int)) );

    		}else
    		if(i<6)
    		{
    			connect (	getItem(i), 	SIGNAL 	( button_pressed (bool,int)),
    						this, 			SLOT 	( button_pressed_slot (bool,int)) );
    		}else
			if(i==6)
			{
    			connect (	getItem(i), 	SIGNAL 	( dataChanged (int,int, int)),
    						this, 			SLOT 	( valueChanged_slot (int,int, int)) );
			}
    	}
    }
	void slot_items_endanimready (int animstate, int last_item) {}
	void slot_resized ();
};






#endif /* GLSCREENCONTROLS_H_ */
