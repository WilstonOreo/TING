#include <QGuiApplication>
#include <QQmlApplicationEngine>

//#include "Graph.h"



int main(int argc, char* argv[]) {
  QGuiApplication::setApplicationName("TING");
  QGuiApplication::setOrganizationName("33C3");
  QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

  QGuiApplication app(argc, argv);

  QQmlApplicationEngine engine;
  engine.load(QUrl("Graph.qml"));
  if (engine.rootObjects().isEmpty()) return -1;

  return app.exec();
}
