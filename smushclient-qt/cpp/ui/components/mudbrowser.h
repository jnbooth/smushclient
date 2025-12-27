#pragma once
#include <QtGui/QKeyEvent>
#include <QtGui/QMouseEvent>
#include <QtWidgets/QTextBrowser>

class MudScrollBar;

class MudBrowser : public QTextBrowser
{
public:
  explicit MudBrowser(QWidget* parent = nullptr);
  virtual ~MudBrowser() {};

  MudScrollBar* verticalScrollBar() const;

public slots:
  void setIgnoreKeypad(bool ignore);
  void setMaximumBlockCount(int maximum);

protected:
  virtual void keyPressEvent(QKeyEvent* event) override;
  virtual void mouseMoveEvent(QMouseEvent* event) override;

private:
  bool ignoreKeypad = false;
};
