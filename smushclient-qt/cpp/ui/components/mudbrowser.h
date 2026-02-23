#pragma once
#include <QtCore/QPointer>
#include <QtWidgets/QTextBrowser>

class MudCursor;
class MudScrollBar;
enum class SendTo : uint8_t;

class MudBrowser : public QTextBrowser
{
  Q_OBJECT
  Q_PROPERTY(bool keypadIgnored READ keypadIgnored WRITE setKeypadIgnored)

public:
  explicit MudBrowser(QWidget* parent = nullptr);

  MudCursor* cursor() const;
  MudScrollBar* verticalScrollBar() const;
  bool keypadIgnored() const noexcept { return m_keypadIgnored; }

public slots:
  void setKeypadIgnored(bool ignored = true);
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
  QPointer<MudCursor> cursorPtr;
  bool m_keypadIgnored = false;
};
