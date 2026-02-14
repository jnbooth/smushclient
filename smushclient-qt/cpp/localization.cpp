#include "localization.h"

QString
FileFilter::image()
{
  return tr("Images (*.png *.jpeg *.jpg *.bmp *.svg);;All Files (*.*)");
}

QString
FileFilter::import()
{
  return tr("MUSHclient world files (*.MCL);;All Files (*.*)");
}

QString
FileFilter::lua()
{
  return tr("Lua files (*.lua);;All Files (*.*)");
}

QString
FileFilter::plugin()
{
  return tr("Plugin files (*.xml);;All Files(*.*)");
}

QString
FileFilter::text()
{
  return tr("Text files (*.txt);;All Files (*.*)");
}

QString
FileFilter::world()
{
  return tr("World files (*.smush);;All Files (*.*)");
}
