#pragma once
#include <QtCore/QPointer>
#include <QtWidgets/QTextBrowser>

class MudCursor;
class MudScrollBar;

class MudBrowser : public QTextBrowser
{
  Q_OBJECT

public:
  explicit MudBrowser(QWidget* parent = nullptr);

  MudScrollBar* verticalScrollBar() const;
  MudCursor* cursor();

public slots:
  void setIgnoreKeypad(bool ignore);
  void setMaximumBlockCount(int maximum);

signals:
  void aliasMenuRequested(const QString& word);

protected:
  void keyPressEvent(QKeyEvent* event) override;
  void mouseMoveEvent(QMouseEvent* event) override;
  void mousePressEvent(QMouseEvent* event) override;

private:
  QPointer<MudCursor> mudCursor;
  bool ignoreKeypad = false;
};
