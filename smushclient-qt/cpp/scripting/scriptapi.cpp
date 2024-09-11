#include "scriptapi.h"

QTextCharFormat colorFormat(const QColor &foreground, const QColor &background)
{
  QTextCharFormat format;
  if (foreground.isValid())
    format.setForeground(QBrush(foreground));
  if (background.isValid())
    format.setBackground(QBrush(background));
  return format;
}

ScriptApi::ScriptApi(QTextDocument *document, QLineEdit *input, QTcpSocket *socket)
    : QObject(document),
      cursor(document),
      document(document),
      input(input),
      socket(socket)
{
}

void ScriptApi::ColourTell(const QColor &foreground, const QColor &background, const QString &text)
{
  cursor.insertText(text, colorFormat(foreground, background));
  needsNewline = true;
}

ScriptReturnCode ScriptApi::Send(const QByteArrayView &view)
{
  if (view.isEmpty())
    return ScriptReturnCode::OK;

  if (!socket->isOpen())
    return ScriptReturnCode::WorldClosed;

  if (view.back() == '\n')
    socket->write(view.constData(), view.size());

  else
  {
    QByteArray bytes;
    bytes.reserve(view.size() + 1);
    bytes.append(view);
    bytes.append('\n');
    socket->write(bytes);
  }
  return ScriptReturnCode::OK;
}

void ScriptApi::ensureNewline()
{
  if (!needsNewline)
    return;
  needsNewline = false;
  if (cursor.atBlockStart())
    return;
  cursor.insertBlock();
}

void ScriptApi::insertBlock()
{
  cursor.insertBlock();
  needsNewline = false;
}
