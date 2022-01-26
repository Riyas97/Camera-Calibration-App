#include <QGraphicsPixmapItem>

/*
* This class is a custom implementation of QGraphicsPixmapItem so as to detect key press events (e.g when a user clicks the delete key)
*/

class CustomGraphicsItemClass: public QObject, public QGraphicsPixmapItem
{
	Q_OBJECT
public:
	void keyPressEvent(QKeyEvent* event); /* invoked when user presses a key in QGraphicsPixmapItem*/

signals:
	void startDeleteCurrentImage(bool isDisplayInMultiView); /* to call the function in main thread to delete the respective image */
};

