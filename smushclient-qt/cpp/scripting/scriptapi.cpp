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

ScriptApi::ScriptApi(QTextDocument *document, QLineEdit *input)
    : QObject(document),
      cursor(document),
      document(document),
      input(input)
{
}

void ScriptApi::ColourTell(const QColor &foreground, const QColor &background, const QString &text)
{
  cursor.insertText(text, colorFormat(foreground, background));
}
