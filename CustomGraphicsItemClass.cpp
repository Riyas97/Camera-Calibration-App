#include "CustomGraphicsItemClass.h"
#include <QDebug>
#include <QKeyEvent>

/*
* To detect when user enters the delete key and subsequently call the main thread function to delete the respective image.
*/
void CustomGraphicsItemClass::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Delete) {
        emit(startDeleteCurrentImage(false));
    }
}

