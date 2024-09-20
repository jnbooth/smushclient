#pragma once
#include <compare>
#include <string>
#include <vector>
#include <unordered_map>
#include <QtWidgets/QTextBrowser>
#include "miniwindow.h"

class MiniWindow;

struct WindowSort
{
  static void sort(
      std::vector<WindowSort> &buffer,
      QTextBrowser *browser,
      const std::unordered_map<std::string, MiniWindow *> &windows);

  char drawOrder;
  int zOrder;
  std::string_view name;
  QWidget *widget;

  WindowSort(MiniWindow *widget, std::string_view name)
      : drawOrder(widget->drawsUnderneath() ? 0 : 2),
        zOrder(widget->getZOrder()),
        name(name),
        widget(widget) {}

  explicit WindowSort(QTextBrowser *widget)
      : drawOrder(1),
        zOrder(0),
        name(),
        widget(widget) {}

  std::strong_ordering operator<=>(const WindowSort &) const = default;
};
