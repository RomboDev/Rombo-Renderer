/*
 * glrendererctrls.h
 *
 *  Created on: Nov 19, 2013
 *      Author: max
 */

#ifndef GLRENDERERCTRLS_H_
#define GLRENDERERCTRLS_H_


#include "gloverlaycontrols.h"

#include <QUndoCommand>

// base class with common signaling
class OverlayRendererSettingsItemBase: public OverlayAnimSubItem
{
	Q_OBJECT

public:
	OverlayRendererSettingsItemBase (	OverlayItemsController * iFactory, int iID, OverlayItemsBuilder * iBuilder,
										const QRect & iPos, const QRect & iEnd, const QRect & iTarget )
	: OverlayAnimSubItem (iFactory, iBuilder, iID)
	{
		setPosition (iPos);
		setPositionEnd (iEnd);
		setPositionAlone (iTarget);

		///////////////////////////////////////////////////////////
		connect (	this, SIGNAL 	(subitems_ready	(int)), 		// this comes from base class (OverlayAnimSubItem)
					this, SLOT 		(bind_subitems	(int)) );		// calling the specialized slot in this class
	};

	~OverlayRendererSettingsItemBase ()
	{
		disconnect (	this, SIGNAL 	(subitems_ready	(int)),		// probably not needed
						this, SLOT 		(bind_subitems	(int)) );
	}

signals:
	void undoredo_slot (int id, QVariant idata);

private slots:
	virtual void bind_subitems (int iID) =0;
	virtual void valueChanged_slot (int data, int decdigits, int id) =0;
	virtual void undoredo_called (int id, QVariant idata) =0;
};



// Render settings ////////////////////////////////////////////////////////////////////////////////////////////

class OverlayRenderSettingsDataBinder_maxDepth: public OverlayItemDataBinder
{
public:
	virtual ~OverlayRenderSettingsDataBinder_maxDepth(){}
	virtual QVariant getBindedStuff ( OverlayItemsController * const& iFactory );
	virtual void setBindedStuff ( OverlayItemsController * const& iFactory, QVariant idata );
};

class OverlayRenderSettingsDataBinder_spp: public OverlayItemDataBinder
{
public:
	virtual ~OverlayRenderSettingsDataBinder_spp(){}
	virtual QVariant getBindedStuff ( OverlayItemsController * const& iFactory );
	virtual void setBindedStuff ( OverlayItemsController * const& iFactory, QVariant idata );
};


// render settings subitems builder
class OverlayRenderSettingsBuilder: public OverlayItemsBuilder
{
public:
	bool buildItems (OverlayItemsController * const& iFactory, QVector<OverlayItem*> * const& iSubitems, int iPos);
};

// render settings
class OverlayRendererSettingsItem: public OverlayRendererSettingsItemBase
{
	Q_OBJECT
public:
	OverlayRendererSettingsItem (OverlayItemsController * iFactory, int iID, OverlayItemsBuilder * iBuilder,
							   const QRect & iPos, const QRect & iEnd, const QRect & iTarget)
	: OverlayRendererSettingsItemBase (iFactory, iID, iBuilder, iPos, iEnd, iTarget)
	, wasDraggin(false), initialData(-999) {}

private slots:
	void bind_subitems (int iID);
	void valueChanged_slot (int data, int decdigits, int id);
	void undoredo_called (int id, QVariant idata) { emit undoredo_slot (id,idata); };

private:
    bool wasDraggin;
    QVariant initialData;
};

// render settings undo/redo
class OverlayRenderSettingsCommand : public QUndoCommand
{
public:
	OverlayRenderSettingsCommand(int iID, QVariant iData, GLViewer *iViewer, QUndoCommand *parent = 0);
	~OverlayRenderSettingsCommand ();

    void undo();
    void redo();

private:
    int lastID;
    QVariant lastData;
    QVariant newData;
    GLViewer * glViewer;
};



// Camera settings ////////////////////////////////////////////////////////////////////////////////////////////

// custom camera settings subitems builder
class OverlayCameraSettingsBuilder: public OverlayItemsBuilder
{
public:
	bool buildItems (OverlayItemsController * const& iFactory, QVector<OverlayItem*> * const& iSubitems, int iPos);
};

// camera settings class
class OverlayCameraSettingsItem: public OverlayRendererSettingsItemBase
{
	Q_OBJECT

public:
	OverlayCameraSettingsItem (OverlayItemsController * iFactory, int iID, OverlayItemsBuilder * iBuilder,
							   const QRect & iPos, const QRect & iEnd, const QRect & iTarget)
	: OverlayRendererSettingsItemBase (iFactory, iID, iBuilder, iPos, iEnd, iTarget),
	  m_rr_active (false), m_cammouse_mode (0),
	  m_cammouse_clickX (0), m_cammouse_clickY (0), m_cammouse_tracking (false),
	  m_active_id (-1), m_in_action (false) {}

protected:
	void drawInstructions(QPainter *painter, int context, int width, int height);
	void resetActionButton ();

	virtual void paint (QPainter* iPainter);
    virtual bool eventFilter (QObject *object, QEvent *event);

private slots:
	void bind_subitems (int iID);
	void valueChanged_slot (int data, int decdigits, int id);
	void button_pressed_slot (bool pressed, int id);
	void undoredo_called (int id, QVariant idata) { emit undoredo_slot (id,idata); };

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

// custom tonemapper settings subitems builder
class OverlayTonemapperSettingsBuilder: public OverlayItemsBuilder
{
public:
	bool buildItems (OverlayItemsController * const& iFactory, QVector<OverlayItem*> * const& iSubitems, int iPos);
};

// tonemapper settings class
class OverlayTonemapperSettingsItem: public OverlayRendererSettingsItemBase
{
	Q_OBJECT
public:
	OverlayTonemapperSettingsItem (OverlayItemsController * iFactory, int iID, OverlayItemsBuilder * iBuilder,
							   	   const QRect & iPos, const QRect & iEnd, const QRect & iTarget)
	: OverlayRendererSettingsItemBase (iFactory, iID, iBuilder, iPos, iEnd, iTarget) {}

private slots:
	void bind_subitems (int iID);
	void valueChanged_slot (int data, int decdigits, int id);
	void button_pressed_slot (bool pressed, int id);
	void undoredo_called (int id, QVariant idata) { emit undoredo_slot (id,idata); };
};




///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Renderer Controller ////////////////////////////////////////////////////////////////////////////////////////

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
	OverlayRendererCtrls (GLViewer *iWidget);

private slots:
    void slot_items_ready () {}
	void slot_subitems_ready (int iID);
};

#endif /* GLRENDERERCTRLS_H_ */
