#pragma once
#include <QtCore/QPointer>
#include <QtWidgets/QTextBrowser>

class MudCursor;
class MudScrollBar;
enum class SendTo : uint8_t;

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
  void linkActivated(const QString& link, SendTo sendTo);

protected:
  void keyPressEvent(QKeyEvent* event) override;
  void mouseMoveEvent(QMouseEvent* event) override;
  void mousePressEvent(QMouseEvent* event) override;
  void mouseReleaseEvent(QMouseEvent* event) override;

private:
  QPoint mapToContents(const QPoint& point) const;

private:
  QPointer<MudCursor> mudCursor;
  bool ignoreKeypad = false;
};
