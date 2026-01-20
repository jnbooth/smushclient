#pragma once
#include <QtWidgets/QScrollBar>

class MudScrollBar : public QScrollBar
{
  Q_OBJECT

public:
  explicit MudScrollBar(QWidget* parent = nullptr);

  void disablePausing() { setPausingEnabled(false); }
  void enablePausing() { setPausingEnabled(true); }
  constexpr bool isPaused() const { return paused; }

public slots:
  void setAutoScrollEnabled(bool enabled = true);
  void setPaused(bool paused = true);
  void setPausingEnabled(bool enabled = true);

  void setAutoScrollDisabled(bool disabled = true)
  {
    setAutoScrollEnabled(!disabled);
  }

  void setPausingDisabled(bool disabled = true)
  {
    setPausingEnabled(!disabled);
  }

  void setUnpaused(bool unpaused = true) { setPaused(!unpaused); }

protected:
  void sliderChange(QAbstractSlider::SliderChange change) override;

private:
  void updateParentPolicy() const;

private:
  bool autoScroll = true;
  bool inInternalChange = false;
  int lastValue = 0;
  bool paused = false;
  bool pausingEnabled = true;
};
