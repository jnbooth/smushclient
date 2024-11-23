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
  void setAutoScrollEnabled(bool enabled = true);
  void setPaused(bool paused = true);
  void setPausingEnabled(bool enabled = true);

  inline void setAutoScrollDisabled(bool disabled = true)
  {
    setAutoScrollEnabled(!disabled);
  }

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
  bool autoScroll;
  bool inInternalChange;
  bool isPaused;
  int lastValue;
  bool pausingEnabled;

private:
  void updateParentPolicy() const;
};
