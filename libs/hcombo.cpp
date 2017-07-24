/*
 * English:
 *
 *       This program is free software: you can redistribute it and/or modify
 *       it under the terms of the GNU General Public License as published by
 *       the Free Software Foundation, either version 3 of the License, or
 *       (at your option) any later version.
 *
 *       This program is distributed in the hope that it will be useful,
 *       but WITHOUT ANY WARRANTY; without even the implied warranty of
 *       MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *       GNU General Public License for more details.
 *
 *       You should have received a copy of the GNU General Public License
 *       along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *       Author: Panther <http://www.emuverse.ru/wiki/User:Panther>
 *
 * Russian:
 *       Этот файл является свободным программным обеспечением, вы можете
 *       распространять и изменять его на условиях лицензии GNU General Public
 *       License, опубликованной Free Software Foundation, версии 3, или
 *       более поздней, на ваше усмотрение.
 *
 *       Программа распространяется с надеждой, что она окажется полезной,
 *       но БЕЗ КАКИХ-ЛИБО ГАРАНТИЙ, в том числе подразумеваемых гарантий
 *       КОММЕРЧЕСКОЙ ЦЕННОСТИ или ПРИГОДНОСТИ ДЛЯ ОПРЕДЕЛЕННЫХ ЦЕЛЕЙ.
 *       Подробнее смотрите текст лицензии GNU General Public License.
 *
 *       Копия текста лицензии должна поставляться вместе с этим файлом,
 *       в противном случае вы можете получить ее по адресу
 *       <http://www.gnu.org/licenses/>
 *
 *       Автор: Panther <http://www.emuverse.ru/wiki/User:Panther>
 */

#include <QString>
#include <QVariant>

#include "hcombo.h"

ComboBoxDelegate::ComboBoxDelegate(QObject *parent):QItemDelegate(parent){}

void ComboBoxDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if(index.data(Qt::AccessibleDescriptionRole).toString() == QLatin1String("separator"))
    {
        painter->setPen(Qt::gray);
        painter->drawLine(option.rect.left(), option.rect.center().y(), option.rect.right(), option.rect.center().y());
    }
    else if(index.data(Qt::AccessibleDescriptionRole).toString() == QLatin1String("parent"))
    {
        QStyleOptionViewItem parentOption = option;
        parentOption.state |= QStyle::State_Enabled;
        QItemDelegate::paint( painter, parentOption, index );
    }
    else if ( index.data(Qt::AccessibleDescriptionRole).toString() == QLatin1String( "child" ) ) {
        QStyleOptionViewItem childOption = option;
        int indent = option.fontMetrics.width( QString( 4, QChar( ' ' ) ) );
        childOption.rect.adjust( indent, 0, 0, 0 );
        childOption.textElideMode = Qt::ElideNone;
        QItemDelegate::paint( painter, childOption, index );
    }
    else
    {
        QItemDelegate::paint(painter, option, index);
    }
}

QSize ComboBoxDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QString type = index.data(Qt::AccessibleDescriptionRole).toString();
    if(type == QLatin1String("separator"))
        return QSize(0, 10);
    return QItemDelegate::sizeHint( option, index );
}

HierarhicalItemModel::HierarhicalItemModel():QStandardItemModel(){}

void HierarhicalItemModel::addParentItem(const QString& text)
{
    QStandardItem* item = new QStandardItem(text);
    item->setFlags(item->flags() & ~(Qt::ItemIsEnabled | Qt::ItemIsSelectable));
    item->setData("parent", Qt::AccessibleDescriptionRole);
    QFont font = item->font();
    font.setBold(true);
    //font.setItalic(true);
    item->setFont(font);
    this->appendRow(item);
}

void HierarhicalItemModel::addChildItem(const QString& text, const QVariant& data)
{
    QStandardItem* item = new QStandardItem(text + QString(4, QChar( ' ' )));
    item->setData(data, Qt::UserRole);
    item->setData("child", Qt::AccessibleDescriptionRole);
    this->appendRow(item);
}

void HierarhicalItemModel::addIndependentItem(const QString& text, const QVariant& data)
{
    QStandardItem* item = new QStandardItem(text + QString(4, QChar( ' ' )));
    item->setData(data, Qt::UserRole);
    this->appendRow(item);
}
