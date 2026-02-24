#pragma once
#include <QtCore/QUuid>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QWidget>

class Notepad;

class Notepads : public QWidget
{
  Q_OBJECT

public:
  explicit Notepads(QWidget* parent = nullptr);

  void closeAll();

  Notepad* createNotepad(const QString& name = QString());
  Notepad* createNotepad(const QUuid& worldID, const QString& name = QString());

  Notepad* findNotepad(const QUuid& worldID, const QString& name) const;
  Notepad* findNotepad(const QString& name) const;

  QStringList listNotepads(const QUuid& worldID = QUuid()) const;

  QTextEdit* pad(const QUuid& worldID, const QString& name = QString());
  QTextEdit* pad(const QString& name = QString());
};
