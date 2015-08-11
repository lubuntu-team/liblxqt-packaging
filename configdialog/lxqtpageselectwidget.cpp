/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * LXQt - a lightweight, Qt based, desktop toolset
 * http://razor-qt.org
 *
 * Copyright: 2010-2011 Razor team
 * Authors:
 *   Alexander Sokoloff <sokoloff.a@gmail.com>
 *
 * This program or library is free software; you can redistribute it
 * and/or modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General
 * Public License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA
 *
 * END_COMMON_COPYRIGHT_HEADER */


#include "lxqtpageselectwidget.h"
#include <QDebug>
#include <QStyledItemDelegate>
#include <QScrollBar>

using namespace LxQt;

class PageSelectWidgetItemDelegate: public QStyledItemDelegate
{
public:
    explicit PageSelectWidgetItemDelegate(PageSelectWidget *parent = 0);
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;

private:
    PageSelectWidget* mView;
};


/************************************************

 ************************************************/
PageSelectWidgetItemDelegate::PageSelectWidgetItemDelegate(PageSelectWidget *parent):
    QStyledItemDelegate(parent),
    mView(parent)
{
}


/************************************************

 ************************************************/
QSize PageSelectWidgetItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QSize size = QStyledItemDelegate::sizeHint(option, index);
    size.setWidth(mView->viewport()->width() - 2*mView->spacing());
    return size;
}



/************************************************

 ************************************************/
PageSelectWidget::PageSelectWidget(QWidget *parent) :
    QListWidget(parent)
{
    setSelectionRectVisible(false);
    setViewMode(IconMode);
    setSpacing(2);
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
    setWordWrap(true);
    setDragEnabled(NoDragDrop);
    setEditTriggers(NoEditTriggers);
    setTextElideMode(Qt::ElideNone);

    setItemDelegate(new PageSelectWidgetItemDelegate(this));
}


/************************************************

 ************************************************/
PageSelectWidget::~PageSelectWidget()
{
}


/************************************************

 ************************************************/
QSize PageSelectWidget::sizeHint() const
{
    QSize size = QListWidget::sizeHint();
    int w = 0;
    for(int i=0; i< count(); ++i)
    {
        QRect rect = fontMetrics().boundingRect(QRect(), Qt::AlignLeft | Qt::TextWordWrap, item(i)->text());
        w = qMax(w, rect.width());
    }

    if (horizontalScrollBar()->isVisible())
        w += horizontalScrollBar()->width();
    size.setWidth(w + frameWidth() + spacing()*2 + 10);
    return size;
}
