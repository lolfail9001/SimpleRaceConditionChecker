#ifndef _ANALYTICS_MODEL_HPP
#define _ANALYTICS_MODEL_HPP

#include "analytics.hpp"
#include <QAbstractTableModel>
#include <QModelIndex>
#include <QVariant>

class AnalyticsModel : public QAbstractTableModel
{
  Q_OBJECT

 private:
  std::vector<fail::ErrorData> ed;
 public:
  AnalyticsModel(const fail::Analytics& as,QObject* parent=0):QAbstractTableModel(parent),ed(as.getErrorData())
  {}
  virtual int rowCount(const QModelIndex& parent=QModelIndex()) const
  {
    if(parent.isValid()) return 0;
    else return ed.size();
  }
  virtual int columnCount(const QModelIndex& parent=QModelIndex()) const
  {
    if(parent.isValid()) return 0;
    else return 4;
  }
  virtual QVariant data(const QModelIndex& ind,int role=Qt::DisplayRole) const
  {
    if(!ind.isValid()) return QVariant();
    if(ind.column()>3) return QVariant();
    if(ind.row()>ed.size()) return QVariant();
    if(role!=Qt::DisplayRole) return QVariant();
    switch(ind.column())
    {
      case 0:
        return QVariant(tr("%1 %2").
                        arg(QString::fromStdString(ed[ind.row()].vd.type)).
                        arg(QString::fromStdString(ed[ind.row()].vd.name)));
      case 1:
        return QVariant(ed[ind.row()].totalReads);
      case 2:
        return QVariant(ed[ind.row()].totalWrites);
      case 3:
        return QVariant(ed[ind.row()].errorCheck);
      default:
        return QVariant();
    }
  }
  virtual QVariant headerData(int section,Qt::Orientation orn,int role=Qt::DisplayRole) const
  {
    if(role!=Qt::DisplayRole) return QVariant();
    switch(orn)
    {
      case Qt::Vertical:
        return (section>ed.size())?QVariant():
            (section<0)?QVariant():QVariant(section);
      case Qt::Horizontal:
        switch(section)
        {
          case 0:
            return QVariant(tr("Variable name"));
          case 1:
            return QVariant(tr("Amount of reads"));
          case 2:
            return QVariant(tr("Amount of writes"));
          case 3:
            return QVariant(tr("Error check"));
          default:
            return QVariant();
        }
    }
  }
};



#endif
