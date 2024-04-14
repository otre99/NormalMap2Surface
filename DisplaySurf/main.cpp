#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[]) {
  QApplication a(argc, argv);
  MainWindow w;
  w.showMaximized();
  auto &&args = a.arguments();
  if (args.size() > 1) {
    w.OpenFileFromCMD(args[1]);
  }
  return a.exec();
}
