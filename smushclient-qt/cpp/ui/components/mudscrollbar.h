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
  constexpr bool paused() const
  {
    return isPaused;
  }
  void setPaused(bool paused);
  void setPausingEnabled(bool enabled);
  void toEnd();

protected:
  virtual void sliderChange(QAbstractSlider::SliderChange change) override;

private:
  int lastValue;
  bool isPaused;
  bool pausingEnabled;

private:
  void updateParentPolicy() const;
};
