#pragma once
#include <QtWidgets/QScrollBar>

class MudScrollBar : public QScrollBar
{
  Q_OBJECT
  Q_PROPERTY(
    bool autoScrollEnabled READ autoScrollEnabled WRITE setAutoScrollEnabled)
  Q_PROPERTY(bool pausingEnabled READ pausingEnabled WRITE setPausingEnabled)
  Q_PROPERTY(bool paused READ paused WRITE setPaused)

public:
  explicit MudScrollBar(QWidget* parent = nullptr);

  bool autoScrollEnabled() const noexcept { return m_autoScroll; }
  bool pausingEnabled() const noexcept { return m_pausingEnabled; }
  bool paused() const noexcept { return m_paused; }

public slots:
  void setAutoScrollEnabled(bool enabled = true);
  void setAutoScrollDisabled(bool disabled = true)
  {
    setAutoScrollEnabled(!disabled);
  }

  void setPaused(bool paused = true);
  void setUnpaused(bool unpaused = true) { setPaused(!unpaused); }

  void setPausingEnabled(bool enabled = true);
  void setPausingDisabled(bool disabled = true)
  {
    setPausingEnabled(!disabled);
  }

protected:
  void sliderChange(QAbstractSlider::SliderChange change) override;

private:
  void updateParentPolicy() const;

private:
  int lastValue = 0;
  bool inInternalChange : 1 = false;
  bool m_autoScroll : 1 = true;
  bool m_paused : 1 = false;
  bool m_pausingEnabled : 1 = true;
};
