#pragma once
#include <QtWidgets/QTextBrowser>

class MudBrowser : public QTextBrowser
{
public:
  explicit MudBrowser(QWidget *parent = nullptr);
  virtual ~MudBrowser() {};

protected:
  virtual void mouseMoveEvent(QMouseEvent *event) override;
};
