#ifndef _DIPLOMAWINDOW_HPP
#define _DIPLOMAWINDOW_HPP

#include "ui_diplomawindow.h"
#include "parser.hpp"
#include "analytics.hpp"

class DiplomaWindow : public QMainWindow
{
  Q_OBJECT

 private:
  Ui::DiplomaWindow* dw;
  fail::Parser* p;
  fail::Analytics* as;
  fail::ThreadSetup ts;
  unsigned mergeDepth;
 public:
  DiplomaWindow(QWidget* parent = 0);
 protected slots:
  void loadFile();
 private:
  void loadVarTable();
};



#endif
