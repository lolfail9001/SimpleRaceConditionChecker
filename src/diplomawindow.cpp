#include "diplomawindow.hpp"
#include "analyticsmode.hpp"
#include "reftablemode.hpp"
#include <QString>
#include <QTextStream>
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <time.h>

DiplomaWindow::DiplomaWindow(QWidget* parent) : QMainWindow(parent),dw(new Ui::DiplomaWindow()),p(nullptr),as(nullptr),ts(),mergeDepth(4)
{
  dw->setupUi(this);
  ts.threadCreateFunc="pthread_create";
  ts.threadEntryFunc="main";
  ts.threadFuncArgNum=2;
  ts.threadSyncFuncs.push_back("pthread_join");
  ts.threadSyncFuncs.push_back("pthread_mutex_lock");
  this->setMenuWidget(dw->menubar);
  this->setStatusBar(dw->statusbar);
  this->setCentralWidget(dw->centralwidget);
  dw->errorView->setVisible(false);
  dw->accessView->setVisible(false);
  connect(dw->openAction,SIGNAL(triggered()),
          this,SLOT(loadFile()));
}

void DiplomaWindow::loadFile()
{
  QString fileName = QFileDialog::getOpenFileName
                     (this,tr("Choose file"),tr("./"));
  QFile file(fileName);
  if(file.open(QIODevice::ReadOnly))
  {
    QTextStream data(&file);
    dw->sourceEdit->setPlainText(data.readAll());
    clock_t start=clock();
    if(p!=nullptr)
      delete p;
    p = new fail::Parser(fileName.toStdString(),
                         &ts,mergeDepth);
    if(as!=nullptr)
      delete as;
    as = new fail::Analytics(p->getParserDataPtr());
    clock_t end = clock();
    dw->errorView->setModel(new AnalyticsModel(*as,this));
    dw->errorView->resizeColumnsToContents();
    dw->errorView->setVisible(true);
    dw->accessView->setModel
        (new ReftableModel
         (*(p->getParserDataPtr()->functions),
          as->getFaultyVars(),
          this));
    dw->accessView->resizeColumnsToContents();
    dw->accessView->setVisible(true);
    double timeUsed = (double)(end-start)/CLOCKS_PER_SEC;
    dw->statusbar->showMessage
        (tr("Executed in %1 seconds").arg(timeUsed,0,'g',4));
    file.close();
    delete as;
    delete p;
    p=nullptr;
    as=nullptr;
  }
  else
  {
    QMessageBox::warning(this,tr("Error!"),
                         tr("Failure upon opening a file!"));
  }
}
