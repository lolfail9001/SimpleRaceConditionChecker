#include "diplomawindow.hpp"
#include <QApplication>


int main(int argc,char** argv)
{
  QApplication app(argc,argv);
  DiplomaWindow mainWindow;
  mainWindow.show();
  return app.exec();
}
