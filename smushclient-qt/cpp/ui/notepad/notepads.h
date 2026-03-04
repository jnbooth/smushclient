#pragma once
#include <QtCore/QUuid>
#include <QtWidgets/QWidget>

class Notepad;

class Notepads : public QWidget
{
  Q_OBJECT

public:
  explicit Notepads(QWidget* parent = nullptr);

  void closeAll();

  Notepad* createNotepad(const QString& name = {});
  Notepad* createNotepad(const QUuid& worldID, const QString& name = {});

  Notepad* findNotepad(const QUuid& worldID, const QString& name) const;
  Notepad* findNotepad(const QString& name) const;

  QStringList listNotepads(const QUuid& worldID = {}) const;

  Notepad* pad(const QUuid& worldID, const QString& name = {});
  Notepad* pad(const QString& name = {});
};
