#include "GraphicsViewZoom.h"
#include <QMouseEvent>
#include <QApplication>
#include <QScrollBar>
#include <qmath.h>
#include <QDebug>
#include <QGraphicsView>
#include <QtWidgets>
#include <QKeyEvent>


GraphicsViewZoom::GraphicsViewZoom(QGraphicsView* view)
    : QObject(view), _graphicView(view)
{
    // initializations to install event filter, set mouse tracking, set keyboard modifier, set zoom velocity and disable zoom
    _graphicView->viewport()->installEventFilter(this);
    _graphicView->setMouseTracking(true);
    _modifiers = Qt::ControlModifier;
    _zoom_factor_base = 1.0015;
    isZoomEnabled = false;
}

void GraphicsViewZoom::zoom(double factor) 
{
    // zoom wrt to factor
    _graphicView->scale(factor, factor);
    // center the graphics view wrt mosue position 
    _graphicView->centerOn(target_scene_pos);
    QPointF delta_viewport_pos = target_viewport_pos - QPointF(_graphicView->viewport()->width() / 2.0,
        _graphicView->viewport()->height() / 2.0);
    QPointF viewport_center = _graphicView->mapFromScene(target_scene_pos) - delta_viewport_pos;
    _graphicView->centerOn(_graphicView->mapToScene(viewport_center.toPoint()));
}

void GraphicsViewZoom::setModifiers(Qt::KeyboardModifiers modifiers) {
    // set keyboard modifier
    // once enabled, user would need to press this key to zoom
    _modifiers = modifiers;

}

void GraphicsViewZoom::setZoomConfig(bool isEnableZoom)
{
    // to enable zoom
    isZoomEnabled = isEnableZoom;
}

void GraphicsViewZoom::setZoomFactorBase(double value) 
{
    // to control the zoom velocity
    _zoom_factor_base = value;
}

void GraphicsViewZoom::setDefaultSize() 
{
    // to reset zoom changes and resize graphics view to default size
    _graphicView->resetMatrix();
}

bool GraphicsViewZoom::eventFilter(QObject* object, QEvent* event) 
{  
    /**
    if (event->type() == QEvent::KeyPress) {
        qDebug() << "KeyPress";
    }
   

    if (event->type() == QEvent::MouseButtonPress) {
        qDebug() << "Right\n";
        QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
        if (mouseEvent->buttons() == Qt::LeftButton) {
            qDebug() << "Left\n";
        }
        else if (mouseEvent->buttons() == Qt::RightButton) {
            qDebug() << "Right\n";
        }
    }

        /**
        qDebug() << "HEREEEEEEEEEEEEEEEEEEEEEEEEEEEEE2";
        QKeyEvent* key = static_cast<QKeyEvent*>(event);
        if (key->key() == Qt::Key_Delete) {
            qDebug() << "HEREEEEEEEEEEEEEEEEEEEEEEEEEEEEE";
        }
        
    **/
    if (isZoomEnabled) {
        if (event->type() == QEvent::MouseMove) {
            // on mouse move
            // but not used since graphicsview is set to dragmode in the main.cpp
            QMouseEvent* mouse_event = static_cast<QMouseEvent*>(event);
            QPointF delta = target_viewport_pos - mouse_event->pos();
            if (qAbs(delta.x()) > 5 || qAbs(delta.y()) > 5) {
                target_viewport_pos = mouse_event->pos();
                target_scene_pos = _graphicView->mapToScene(mouse_event->pos());
            }
        } else if (event->type() == QEvent::Wheel) {
            // on mouse wheel
            QWheelEvent* wheel_event = static_cast<QWheelEvent*>(event);
            // check for keyboard modifier
            if (QApplication::keyboardModifiers() == _modifiers) {
                if (wheel_event->orientation() == Qt::Vertical) {
                    double angle = wheel_event->angleDelta().y();
                    double factor = qPow(_zoom_factor_base, angle);
                    
                    // ensure zoom is confined to a limit                
                    qreal check_factor = _graphicView->transform().scale(factor, factor).mapRect(QRectF(0, 0, 1, 1)).width();
                    if (check_factor < min_zoom_limit || check_factor > max_zoom_limit) {
                        // not within limit and so don't zoom
                        return true;
                    }
                    else {
                        // within limit and so zoom
                        zoom(factor);
                    }
                    return true;
                }
            }
        }
    }
    
    Q_UNUSED(object)
        return false;
}

/**
void GraphicsViewZoom::keyPressEvent(QKeyEvent* event) {
    qDebug() << "ITS COMING HOMEEEEEEE";
}

void GraphicsViewZoom::contextMenuEvent(QGraphicsSceneContextMenuEvent* event)
{
    QMenu menu(event->widget());
    menu.addAction("Delete image");
    //menu.exec(event->globalPos());
}
**/
