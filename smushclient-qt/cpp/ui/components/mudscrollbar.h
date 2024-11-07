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

public slots:
  void setPaused(bool paused = true);
  void setPausingEnabled(bool enabled = true);
  void toEnd();

  inline void setPausingDisabled(bool disabled = true)
  {
    setPausingEnabled(!disabled);
  }
  inline void setUnpaused(bool unpaused = true)
  {
    setPaused(!unpaused);
  }

protected:
  virtual void sliderChange(QAbstractSlider::SliderChange change) override;

private:
  int lastValue;
  bool isPaused;
  bool pausingEnabled;

private:
  void updateParentPolicy() const;
};
