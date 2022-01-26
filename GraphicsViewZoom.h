#include <QObject>
#include <QGraphicsView>

/*
 * This class zooms QGraphicsView using mouse wheel. 
 * Point under the cursor is motionless while possible (not possible when scene's size is not large enough comparing to the viewport size).
 *
 * When user scrolls to zoom, this class remembers the original scene position and retains it while scrolling.
 *
 * Keyboard modifiers can be used by calling setModifiers(). 
 * This would ensure zooming is performed only on exact match of modifiers 
 * 
 * Zoom velocity can be changed by calling setZoomFactorBase().
 */

class GraphicsViewZoom : public QObject {
	Q_OBJECT

public:
	GraphicsViewZoom(QGraphicsView* view); /* to store the graphics view element */
	void setModifiers(Qt::KeyboardModifiers modifiers); /* to set keyboard modifier */
	void setZoomConfig(bool isEnableZoom); /* to set whether to enable zoom */
	void setZoomFactorBase(double value); /* to change the zoom velocity */
	void zoom(double factor); /* to zoom into graphics view */
	void setDefaultSize(); /* to reset zoom and resize to original size */
	//void keyPressEvent(QKeyEvent* event);
	//void contextMenuEvent(QGraphicsSceneContextMenuEvent* event);

//protected:
//	void keyPressEvent(QKeyEvent* event) override;

private:
	QGraphicsView* _graphicView; /* graphics view element */
	Qt::KeyboardModifiers _modifiers; /* keyboard modifier */
	bool eventFilter(QObject* object, QEvent* event); /* event filter to detect mouse wheel and move events */
	bool isZoomEnabled = false; /* stores whether zoom is enabled */
	double _zoom_factor_base; /* stores the zoom velocity */
	QPointF target_scene_pos, target_viewport_pos; /* stores the scene and viewport mouse positions */
	double min_zoom_limit = 0.8; /* minimum to which zoom out */
	double max_zoom_limit = 18; /* maximum to which zoom in */
};
