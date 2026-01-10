#pragma once
#include <QtGui/QKeyEvent>
#include <QtGui/QMouseEvent>
#include <QtWidgets/QTextBrowser>

class MudScrollBar;

class MudBrowser : public QTextBrowser
{
  Q_OBJECT

public:
  explicit MudBrowser(QWidget* parent = nullptr);
  virtual ~MudBrowser() {};

  MudScrollBar* verticalScrollBar() const;

public slots:
  void setIgnoreKeypad(bool ignore);
  void setMaximumBlockCount(int maximum);

signals:
  void aliasMenuRequested(const QString& word);

protected:
  virtual void keyPressEvent(QKeyEvent* event) override;
  virtual void mouseMoveEvent(QMouseEvent* event) override;
  virtual void mousePressEvent(QMouseEvent* event) override;

private:
  bool ignoreKeypad = false;
};
