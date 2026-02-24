#include "notepads.h"
#include "notepad.h"
#include <QtWidgets/QApplication>

// Public methods

Notepads::Notepads(QWidget* parent)
  : QWidget(parent)
{
  hide();
}

void
Notepads::closeAll()
{
  qDeleteAll(children());
}

Notepad*
Notepads::createNotepad(const QString& name)
{
  return createNotepad(QUuid(), name);
}

Notepad*
Notepads::createNotepad(const QUuid& worldID, const QString& name)
{
  Notepad* notepad = new Notepad(worldID, name, this);
  notepad->show();
  if (QWidget* active = QApplication::activeWindow(); active != nullptr) {
    active->activateWindow();
  }
  return notepad;
}

Notepad*
Notepads::findNotepad(const QString& name) const
{
  return findNotepad(QUuid(), name);
}

Notepad*
Notepads::findNotepad(const QUuid& worldID, const QString& name) const
{
  if (name.isEmpty()) {
    return nullptr;
  }
  for (QObject* widget : children()) {
    if (widget->objectName().compare(
          name, Qt::CaseSensitivity::CaseInsensitive) != 0) {
      continue;
    }
    Notepad* notepad = qobject_cast<Notepad*>(widget);
    if (notepad == nullptr) {
      continue;
    }
    if (worldID.isNull() || notepad->world() == worldID) {
      return notepad;
    }
  }
  return nullptr;
}

QStringList
Notepads::listNotepads(const QUuid& worldID) const
{
  QStringList list;
  const QObjectList& childObjects = children();
  list.reserve(childObjects.size());
  for (QObject* widget : childObjects) {
    Notepad* notepad = qobject_cast<Notepad*>(widget);
    if (notepad == nullptr) {
      continue;
    }
    if (worldID.isNull() || notepad->world() == worldID) {
      list.append(notepad->objectName());
    }
  }
  return list;
}

QTextEdit*
Notepads::pad(const QString& name)
{
  return pad(QUuid(), name);
}

QTextEdit*
Notepads::pad(const QUuid& worldID, const QString& name)
{
  if (name.isEmpty()) {
    return createNotepad(worldID, name)->editor();
  }

  Notepad* notepad = findNotepad(worldID, name);

  if (notepad == nullptr) {
    notepad = createNotepad(worldID, name);
    notepad->setWindowTitle(name);
  }

  return notepad->editor();
}
