#pragma once
#include <QtWidgets/QTextBrowser>

class MudScrollBar;

class MudBrowser : public QTextBrowser
{
public:
  explicit MudBrowser(QWidget *parent = nullptr);
  virtual ~MudBrowser() {};

  MudScrollBar *verticalScrollBar() const;

  void setIgnoreKeypad(bool ignore);

protected:
  virtual void keyPressEvent(QKeyEvent *event) override;
  virtual void mouseMoveEvent(QMouseEvent *event) override;

private:
  bool ignoreKeypad;
};
