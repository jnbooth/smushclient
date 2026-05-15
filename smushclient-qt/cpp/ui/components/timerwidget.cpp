#include "timerwidget.h"
#include "ui_timerwidget.h"
#include <QtCore/QTimer>

using std::chrono::duration;
using std::chrono::minutes;
using std::chrono::seconds;

using Qt::StringLiterals::operator""_L1;

namespace {
std::array<QString, 60>
buildTimeLookup()
{
  std::array<QString, 60> list;
  int i = 0;
  for (QString& s : list) {
    s = QString::number(i);
    if (i < 10) {
      s.prepend("0"_L1);
    }
    ++i;
  }
  return list;
}

const std::array<QString, 60> timeLookup = buildTimeLookup();

template<typename Rep, typename Period>
const QString&
formatTime(duration<Rep, Period> time) noexcept
{
  return timeLookup.at(time.count());
}
} // namespace

// TimerWidget

// Public methods

TimerWidget::TimerWidget(QWidget* parent)
  : QWidget(parent)
  , ui(new Ui::TimerWidget)
{
  ui->setupUi(this);
}

TimerWidget::~TimerWidget()
{
  delete ui;
}

// Public slots

void
TimerWidget::clear()
{
  setTime(hh_mm_ss{});
}

void
TimerWidget::setSecondsVisible(bool visible)
{
  m_secondsVisible = visible;
  if (visible) {
    ui->seconds->setText(formatTime(m_time.seconds()));
  }
  ui->minutes_seconds->setVisible(visible);
  ui->seconds->setVisible(visible);
}

// Private methods

void
TimerWidget::setTime(hh_mm_ss time)
{
  if (m_secondsVisible && time.seconds() != m_time.seconds()) {
    ui->seconds->setText(formatTime(time.seconds()));
  }
  if (time.minutes() != m_time.minutes()) {
    ui->minutes->setText(formatTime(time.minutes()));
  }
  if (time.hours() != m_time.hours()) {
    ui->hours->setText(QString::number(time.hours().count()));
  }
  m_time = time;
}

// ElapsedTimerWidget

// public methods

ElapsedTimerWidget::ElapsedTimerWidget(QWidget* parent)
  : TimerWidget(parent)
{
}

// public overrides

void
ElapsedTimerWidget::clear()
{
  stop();
  TimerWidget::clear();
}

void
ElapsedTimerWidget::setSecondsVisible(bool visible)
{
  if (visible == secondsVisible()) {
    return;
  }
  if (elapsed.isValid()) {
    resume();
  }
  TimerWidget::setSecondsVisible(visible);
}

void
ElapsedTimerWidget::timerEvent(QTimerEvent* event)
{
  if (event->id() != timerId) [[unlikely]] {
    killTimer(event->id());
    return;
  }
  updateTime();
}

// public slots

void
ElapsedTimerWidget::pause()
{
  timerId = { -1 };
}

void
ElapsedTimerWidget::resume()
{
  updateTime();
  if (secondsVisible()) {
    timerId = { startTimer(seconds{ 1 }) };
  } else {
    timerId = { startTimer(minutes{ 1 }) };
  }
}

void
ElapsedTimerWidget::start()
{
  elapsed.start();
  resume();
}

void
ElapsedTimerWidget::stop()
{
  pause();
  updateTime();
  elapsed.invalidate();
}

// private methods

void
ElapsedTimerWidget::updateTime()
{
  setTime(elapsed.durationElapsed());
}
