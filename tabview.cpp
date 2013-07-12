#include "tabview.h"
TabView::TabView(QWidget *parent) :
    QTableView(parent)
{
    //Standard QTableView constructor and destructor. Nothing to write here.
}

TabView::~TabView()
{
}

void TabView::refresh()                                                                 //Keep things nice and tidy
{
    this->resizeColumnsToContents();
    this->resizeRowsToContents();
    this->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
}

void TabView::mouseMoveEvent(QMouseEvent *event)                                        //Drag-drop functionality here
{
    QModelIndex index = this->indexAt(event->pos());
    if (!(event->buttons() & Qt::LeftButton))
        return;
    if ((event->pos() - m_dragStartPosition).manhattanLength() < QApplication::startDragDistance())
        return;
    QDrag *dr = new QDrag(this);
    // The data to be transferred by the drag and drop operation is contained in a QMimeData object
    QMimeData *data = new QMimeData;
    if (this->objectName() == QStringLiteral("tabFirstDB"))
        data->setData("text/first-text", index.data().toByteArray());
    if (this->objectName() == QStringLiteral("tabSecondDB"))
        data->setData("text/second-text", index.data().toByteArray());
    dr->setMimeData(data);
    dr->start();

}

void TabView::mousePressEvent(QMouseEvent *event)                                       //Get mousepos at click before resuming normal operation
{
    if (event->buttons() == Qt::LeftButton) {
        m_dragStartPosition = event->pos();
    }
    QTableView::mousePressEvent(event);
}

void TabView::dropEvent(QDropEvent *de)
{
    // Unpack dropped data and handle it
    QModelIndex index = this->indexAt(de->pos());

    if (index.isValid()) {
        this->model()->insertRow(index.row());
        if (de->mimeData()->hasFormat(QStringLiteral("text/second-text"))) {
            this->model()->setData(index, de->mimeData()->data(QStringLiteral("text/second-text")));
            emit dbTableMoved(de->mimeData()->data(QStringLiteral("text/second-text")), SECOND);
        }
        if (de->mimeData()->hasFormat(QStringLiteral("text/first-text"))) {
            this->model()->setData(index, de->mimeData()->data(QStringLiteral("text/first-text")));
            emit dbTableMoved(de->mimeData()->data(QStringLiteral("text/first-text")), FIRST);
        }
        this->refresh();
    }
    else{
        this->model()->insertRow(this->model()->rowCount());
        if (de->mimeData()->hasFormat(QStringLiteral("text/second-text"))) {
            this->model()->setData(this->model()->index(this->model()->rowCount()-1, 0), \
                                   de->mimeData()->data(QStringLiteral("text/second-text")));
            emit dbTableMoved(de->mimeData()->data(QStringLiteral("text/second-text")), SECOND);
        }
        if (de->mimeData()->hasFormat(QStringLiteral("text/first-text"))) {
            this->model()->setData(this->model()->index(this->model()->rowCount()-1, 0), \
                                   de->mimeData()->data(QStringLiteral("text/first-text")));
            emit dbTableMoved(de->mimeData()->data(QStringLiteral("text/first-text")), FIRST);
        }
        this->refresh();
    }
}

void TabView::dragMoveEvent(QDragMoveEvent *de)
{
    // The event needs to be accepted here.
    de->accept();
}

void TabView::dragEnterEvent(QDragEnterEvent *event)
{
    // Set the drop action to be the proposed action. Again, to know who calls the drag.
    if ((event->mimeData()->hasFormat(QStringLiteral("text/first-text")))
            && (this->objectName() == QStringLiteral("tabSecondDB")))
        event->acceptProposedAction();
    if ((event->mimeData()->hasFormat(QStringLiteral("text/second-text")))
            && (this->objectName() == QStringLiteral("tabFirstDB")))
        event->acceptProposedAction();
}
