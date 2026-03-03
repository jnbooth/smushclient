#pragma once

#include <QtCore/QElapsedTimer>
#include <QtWidgets/QWidget>
#include <chrono>

namespace Ui {
class TimerWidget;
} // namespace Ui

class TimerWidget : public QWidget
{
  Q_OBJECT
  Q_PROPERTY(bool secondsVisible READ secondsVisible WRITE setSecondsVisible)
  Q_PROPERTY(std::chrono::seconds time READ time WRITE setTime)

public:
  explicit TimerWidget(QWidget* parent = nullptr);
  ~TimerWidget() override;

  bool secondsVisible() const noexcept { return m_secondsVisible; }
  std::chrono::seconds time() const noexcept { return m_time.to_duration(); }

public slots:
  virtual void clear();
  virtual void setSecondsVisible(bool visible = true);
  void setTime(std::chrono::seconds time)
  {
    setTime(std::chrono::hh_mm_ss{ time });
  }
  void setTime(std::chrono::milliseconds time)
  {
    setTime(std::chrono::hh_mm_ss{
      std::chrono::duration_cast<std::chrono::seconds>(time) });
  }
  void setTime(std::chrono::nanoseconds time)
  {
    setTime(std::chrono::hh_mm_ss{
      std::chrono::duration_cast<std::chrono::seconds>(time) });
  }

protected:
  virtual void setTime(std::chrono::hh_mm_ss<std::chrono::seconds> time);

private:
  Ui::TimerWidget* ui;
  bool m_secondsVisible = true;
  std::chrono::hh_mm_ss<std::chrono::seconds> m_time;
};

class ElapsedTimerWidget : public TimerWidget
{
  Q_OBJECT

public:
  explicit ElapsedTimerWidget(QWidget* parent = nullptr);
  bool isRunning() const noexcept { return elapsed.isValid(); }

public:
  void clear() override;
  void setSecondsVisible(bool visible = true) override;
  void timerEvent(QTimerEvent* event) override;

public slots:
  void pause();
  void resume();
  void start();
  void stop();

private:
  void updateTime();

private:
  QElapsedTimer elapsed;
  int timerId = -1;
};
