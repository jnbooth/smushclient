#pragma once
#include <QtWidgets/QScrollBar>

class MudScrollBar : public QScrollBar
{
  Q_OBJECT

public:
  explicit MudScrollBar(QWidget *parent = nullptr);
  virtual ~MudScrollBar() {}

  inline void disablePausing() { setPausingEnabled(false); }
  inline void enablePausing() { setPausingEnabled(true); }
  void setPausingEnabled(bool enabled);
  void toEnd();
  void unpause();

protected:
  virtual void sliderChange(QAbstractSlider::SliderChange change) override;

private:
  int lastValue;
  bool paused;
  bool pausingEnabled;
};
