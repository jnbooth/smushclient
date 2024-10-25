#pragma once
#include <QtWidgets/QTextBrowser>

class MudScrollBar;

class MudBrowser : public QTextBrowser
{
public:
  explicit MudBrowser(QWidget *parent = nullptr);
  virtual ~MudBrowser() {};

  MudScrollBar *verticalScrollBar() const;

protected:
  virtual void mouseMoveEvent(QMouseEvent *event) override;
};
