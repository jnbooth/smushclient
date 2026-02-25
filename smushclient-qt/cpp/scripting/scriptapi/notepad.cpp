#include "../../ui/notepad/notepads.h"
#include "../scriptapi.h"

#define TRY_NOTEPAD(name)                                                      \
  notepads.findNotepad(worldID, name);                                         \
  if (notepad == nullptr) [[unlikely]] {                                       \
    return {};                                                                 \
  }

// Public methods

bool
ScriptApi::ActivateNotepad(const QString& name) const
{
  Notepad* notepad = TRY_NOTEPAD(name);
  activateWindow(notepad);
  return true;
}

void
ScriptApi::AppendToNotepad(const QString& name, const QString& text) const
{
  notepads.pad(worldID, name)->appendText(text);
}

bool
ScriptApi::CloseNotepad(const QString& name, bool querySave) const
{
  Notepad* notepad = TRY_NOTEPAD(name);
  notepad->setSaveMethod(querySave ? Notepad::SaveMethod::AlwaysPrompt
                                   : Notepad::SaveMethod::NeverPrompt);
  notepad->close();
  return true;
}

int
ScriptApi::GetNotepadLength(const QString& name) const
{
  Notepad* notepad = TRY_NOTEPAD(name);
  return notepad->document()->characterCount();
}

QStringList
ScriptApi::GetNotepadList(bool all) const
{
  return all ? notepads.listNotepads() : notepads.listNotepads(worldID);
}

QString
ScriptApi::GetNotepadText(const QString& name) const
{
  Notepad* notepad = TRY_NOTEPAD(name);
  return notepad->document()->toPlainText();
}

QRect
ScriptApi::GetNotepadWindowPosition(const QString& name) const
{
  Notepad* notepad = TRY_NOTEPAD(name);
  return notepad->window()->rect();
}

bool
ScriptApi::NotepadColour(const QString& name,
                         const QColor& foreground,
                         const QColor& background) const
{
  if (!foreground.isValid() || !background.isValid()) {
    return false;
  }
  Notepad* notepad = TRY_NOTEPAD(name);
  QPalette palette;
  palette.setColor(QPalette::ColorRole::Text, foreground);
  palette.setColor(QPalette::ColorRole::Base, background);
  notepad->editor()->setPalette(palette);
  return true;
}

bool
ScriptApi::NotepadFont(const QString& name, const QTextCharFormat& format) const
{
  Notepad* notepad = TRY_NOTEPAD(name);
  notepad->editor()->mergeCurrentCharFormat(format);
  return true;
}

bool
ScriptApi::NotepadReadOnly(const QString& name, bool readOnly) const
{
  Notepad* notepad = TRY_NOTEPAD(name);
  notepad->editor()->setReadOnly(readOnly);
  return true;
}

bool
ScriptApi::NotepadSaveMethod(const QString& name,
                             Notepad::SaveMethod method) const
{
  Notepad* notepad = TRY_NOTEPAD(name);
  notepad->setSaveMethod(method);
  return true;
}

bool
ScriptApi::ReplaceNotepad(const QString& name, const QString& text) const
{
  notepads.pad(worldID, name)->setText(text);
  return true;
}

bool
ScriptApi::SaveNotepad(const QString& name,
                       const QString& path,
                       bool replace) const
{
  Notepad* notepad = TRY_NOTEPAD(name);
  return !(path.isEmpty() ? notepad->saveAsNew(path, !replace)
                          : notepad->save())
            .isEmpty();
}

bool
ScriptApi::SendToNotepad(const QString& name, const QString& text) const
{
  notepads.createNotepad(worldID, name)->editor()->setPlainText(text);
  return true;
}
