#ifndef _REFTABLE_MODEL_HPP
#define _REFTABLE_MODEL_HPP

#include <QAbstractTableModel>
#include "commons.hpp"
#include <QVariant>
#include <QBrush>
#include <QColor>
#include <algorithm>

class ReftableModel : public QAbstractTableModel
{
  Q_OBJECT

 private:
  std::vector<fail::FuncSeq*> funcs;
  std::vector<fail::RefTable> refs;
  std::vector<bool> isErr;
 public:
  ReftableModel(const std::vector<fail::FuncSeq*>& fs,
                const std::vector<fail::VarDecl>& errs,
                QObject* parent=0):QAbstractTableModel(parent),funcs(fs)
  {
    for(const auto& f: funcs)
    {
      for(const auto& vr : f->refs)
      {
        refs.push_back(fail::RefTable(f->func,vr));
        if(std::find(errs.begin(),errs.end(),vr.vd)!=errs.end())
          isErr.push_back(true);
        else isErr.push_back(false);
      }
    }
  }

  virtual int rowCount(const QModelIndex& parent=QModelIndex()) const
  {
    if(parent.isValid()) return 0;
    else return refs.size();
  }

  virtual int columnCount(const QModelIndex& parent=QModelIndex()) const
  {
    if(parent.isValid()) return 0;
    else return 4;
  }

  virtual QVariant data(const QModelIndex& ind,int role=Qt::DisplayRole) const
  {
    if(role!=Qt::DisplayRole&&role!=Qt::BackgroundRole) return QVariant();
    if(!ind.isValid()) return QVariant();
    if(ind.column()>3) return QVariant();
    if(ind.row()>refs.size()) return QVariant();
    if(role==Qt::BackgroundRole)
    {
      if(isErr[ind.row()])
        return QVariant(QBrush(QColor(Qt::darkRed)));
      else return QVariant();
    }
    switch(ind.column())
    {
      case 0:
        return QVariant(tr("%1 %2").
                        arg(QString::fromStdString(refs[ind.row()].func.type)).
                        arg(QString::fromStdString(refs[ind.row()].func.name)));
      case 1:
        return QVariant(tr("%1 %2").
                        arg(QString::fromStdString(refs[ind.row()].ref.vd.type)).
                        arg(QString::fromStdString(refs[ind.row()].ref.vd.name)));
      case 2:
        switch(refs[ind.row()].ref.type)
        {
          case fail::VarRef::READ:
              return QVariant(tr("Read"));
          case fail::VarRef::WRITE:
              return QVariant(tr("Write"));
        }
      case 3:
        return QVariant(tr("%1:%2").
                        arg(refs[ind.row()].ref.fslStart.line).
                        arg(refs[ind.row()].ref.fslStart.col));
    }
  }
  virtual QVariant headerData(int section,Qt::Orientation orn,int role=Qt::DisplayRole) const
  {
    if(role!=Qt::DisplayRole) return QVariant();
    switch(orn)
    {
      case Qt::Vertical:
        return (section>refs.size())?QVariant():
            (section<0)?QVariant():QVariant(section);
      case Qt::Horizontal:
        switch(section)
        {
          case 0:
            return QVariant(tr("Function name"));
          case 1:
            return QVariant(tr("Variable name"));
          case 2:
            return QVariant(tr("Access type"));
          case 3:
            return QVariant(tr("Source code location"));
          default:
            return QVariant();
        }
    }
  }
};


#endif
