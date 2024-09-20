#include "windowsort.h"
#include <compare>

using std::string;
using std::string_view;
using std::unordered_map;
using std::vector;

void WindowSort::sort(
    vector<WindowSort> &buffer,
    QTextBrowser *browser,
    const unordered_map<string, MiniWindow *> &windows)
{
  buffer.reserve(windows.size() + 1);
  buffer.emplace_back(browser);
  for (const auto &entry : windows)
    buffer.emplace_back(entry.second, string_view(entry.first));

  std::sort(buffer.begin(), buffer.end());

  for (auto it = buffer.rbegin(), end = buffer.rend(); it != end; ++it)
    it->widget->lower();

  buffer.clear();
}
