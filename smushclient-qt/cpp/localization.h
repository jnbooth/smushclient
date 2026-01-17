#pragma once
#include <QtCore/QObject>

class FileFilter : public QObject
{
  Q_OBJECT

public:
  static QString import();

  static QString lua();

  static QString plugin();

  static QString text();

  static QString world();
};
