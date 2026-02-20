#include "../bytes.h"
#include "../casting.h"
#include "../layout.h"
#include "../spans.h"
#include "../timer_map.h"
#include "../ui/mudstatusbar/mudstatusbar.h"
#include "../ui/ui_worldtab.h"
#include "../ui/worldtab.h"
#include "callback/plugincallback.h"
#include "miniwindow/miniwindow.h"
#include "scriptapi.h"
#include "smushclient_qt/src/ffi/document.cxxqt.h"
#include "smushclient_qt/src/ffi/spans.cxx.h"
#include "smushclient_qt/src/ffi/util.cxx.h"
#include "sqlite3.h"
#include <QtCore/QFile>
#include <QtGui/QClipboard>
#include <QtGui/QGradient>
#include <QtGui/QGuiApplication>
#include <QtWidgets/QColorDialog>
#include <QtWidgets/QStatusBar>

using std::string;
using std::string_view;
using std::chrono::hours;
using std::chrono::milliseconds;
using std::chrono::minutes;
using std::chrono::seconds;

// Public static methods

QStringList
ScriptApi::GetAlphaOptionList() noexcept
{
  return ffi::util::get_alpha_option_list();
}

QStringList
ScriptApi::GetOptionList() noexcept
{
  return ffi::util::get_option_list();
}

QColor
ScriptApi::GetSysColor(SysColor sysColor)
{
  const QPalette palette = QGuiApplication::palette();
  switch (sysColor) {
    case SysColor::Background:
      return palette.color(QPalette::ColorRole::Base);
    case SysColor::Window:
      return palette.color(QPalette::ColorRole::Window);
    case SysColor::WindowText:
      return palette.color(QPalette::ColorRole::WindowText);
    case SysColor::ButtonFace:
      return palette.color(QPalette::ColorRole::Button);
    case SysColor::ButtonShadow:
      return palette.color(QPalette::ColorRole::Shadow);
    case SysColor::ButtonText:
      return palette.color(QPalette::ColorRole::ButtonText);
    case SysColor::ButtonHighlight:
      return palette.color(QPalette::ColorRole::BrightText);
    case SysColor::Dark:
      return palette.color(QPalette::ColorRole::Dark);
    case SysColor::Light:
      return palette.color(QPalette::Light);
    default:
      return QColor();
  }
}

int64_t
ScriptApi::GetUniqueNumber() noexcept
{
  static int64_t uniqueNumber = -1;
  if (uniqueNumber == INT64_MAX) [[unlikely]] {
    uniqueNumber = 0;
  } else {
    ++uniqueNumber;
  }
  return uniqueNumber;
}

QString
ScriptApi::MakeRegularExpression(string_view pattern) noexcept
{
  return ffi::regex::from_wildcards(bytes::slice(pattern));
}

void
ScriptApi::SetClipboard(const QString& text)
{
  QGuiApplication::clipboard()->setText(text);
}

// Public methods

void
ScriptApi::AnsiNote(std::string_view text) const
{
  for (const StyledSpan& span : client.ansiNote(bytes::slice(text))) {
    cursor->appendText(span.text, span.format);
  }
}

int64_t
ScriptApi::BroadcastPlugin(size_t index,
                           int64_t message,
                           string_view text) const
{
  const Plugin& callingPlugin = plugins[index];
  int64_t calledPlugins = 0;
  OnPluginBroadcast onBroadcast(
    message, callingPlugin.id(), callingPlugin.name(), text);
  for (const Plugin& plugin : plugins) {
    if (&plugin != &callingPlugin && plugin.runCallbackThreaded(onBroadcast)) {
      ++calledPlugins;
    }
  }
  return calledPlugins;
}

void
ScriptApi::ColourTell(const QColor& foreground,
                      const QColor& background,
                      const QString& text)
{
  QTextCharFormat format;
  if (foreground.isValid()) {
    format.setForeground(foreground);
  }
  if (background.isValid()) {
    format.setBackground(background);
  }
  cursor->appendTell(text, format);
}

int
ScriptApi::DatabaseClose(string_view databaseID)
{
  auto search = databases.find(databaseID);
  if (search == databases.end()) [[unlikely]] {
    return DatabaseConnection::Error::IdNotFound;
  }

  const int result = search->second.close();
  databases.erase(search);
  return result;
}

int
ScriptApi::DatabaseOpen(string_view databaseID, string_view filename, int flags)
{
  auto entry = databases.emplace(string(databaseID), filename);
  DatabaseConnection& db = entry.first->second;
  if (!entry.second) {
    return db.isFile(databaseID)
             ? SQLITE_OK
             : DatabaseConnection::Error::DatabaseAlreadyExists;
  }

  const int result = db.open(flags);
  if (result != SQLITE_OK) {
    databases.erase(entry.first);
  }

  return result;
}

ApiCode
ScriptApi::DeleteVariable(size_t plugin, string_view key) const
{
  return client.unsetVariable(plugin, key) ? ApiCode::OK
                                           : ApiCode::VariableNotFound;
}

ApiCode
ScriptApi::DoAfter(size_t plugin,
                   double seconds,
                   const QString& text,
                   SendTarget target)
{
  if (seconds < 0.1 || seconds > 86399) {
    return ApiCode::TimeInvalid;
  }
  const milliseconds duration =
    milliseconds{ clamped_cast<int64_t>(seconds * 1000.0) };
  sendQueue->start(duration,
                   { .plugin = plugin,
                     .sendTo = target,
                     .text = text,
                     .destination = QString() });
  return ApiCode::OK;
}

ApiCode
ScriptApi::EnablePlugin(string_view pluginID, bool enabled)
{
  const size_t index = findPluginIndex(pluginID);
  if (index == noSuchPlugin) {
    return ApiCode::NoSuchPlugin;
  }
  setPluginEnabled(index, enabled);
  client.setPluginEnabled(index, enabled);
  return ApiCode::OK;
}

VariableView
ScriptApi::GetAlphaOption(size_t plugin, string_view name) const
{
  return client.worldAlphaOption(plugin, bytes::slice(name));
}

int
ScriptApi::GetLinesInBufferCount() const
{
  return cursor->document()->lineCount();
}

QVariant
ScriptApi::GetCurrentValue(size_t pluginIndex, string_view option) const
{
  return client.worldVariantOption(pluginIndex, bytes::slice(option));
}

int64_t
ScriptApi::GetOption(size_t plugin, string_view name) const
{
  return client.worldOption(plugin, bytes::slice(name));
}

VariableView
ScriptApi::GetVariable(size_t index, string_view key) const
{

  return client.getVariable(index, key);
}

VariableView
ScriptApi::GetVariable(string_view pluginID, string_view key) const
{
  const size_t index = findPluginIndex(pluginID);
  if (index == noSuchPlugin) {
    return VariableView(nullptr, 0);
  }
  return GetVariable(index, key);
}

const string&
ScriptApi::GetPluginID(size_t index) const
{
  return plugins.at(index).id();
}

void
ScriptApi::Hyperlink(const QString& action,
                     const QString& text,
                     const QString& hint,
                     const QColor& foreground,
                     const QColor& background,
                     bool url,
                     bool noUnderline)
{
  QTextCharFormat format;
  format.setAnchorHref(action);
  spans::setSendTo(format, url ? SendTo::Internet : SendTo::World);
  format.setToolTip(hint.isEmpty() ? action : hint);
  if (foreground.isValid()) {
    format.setForeground(foreground);
  }
  if (background.isValid()) {
    format.setBackground(background);
  }
  if (!noUnderline) {
    format.setAnchor(true);
  }
  cursor->appendTell(text, format);
}

QColor
ScriptApi::PickColour(const QColor& hint) const
{
  return QColorDialog::getColor(hint, &tab);
}

ApiCode
ScriptApi::PlaySound(size_t channel,
                     string_view path,
                     bool loop,
                     float volume) const
{
  return client.playFile(channel, bytes::slice(path), volume, loop);
}

ApiCode
ScriptApi::PlaySound(size_t channel,
                     const QString& path,
                     bool loop,
                     float volume) const
{
  const QByteArray utf8 = path.toUtf8();
  return PlaySound(
    channel, std::string_view(utf8.data(), utf8.size()), loop, volume);
}

ApiCode
ScriptApi::PlaySoundMemory(size_t channel,
                           QByteArrayView sound,
                           bool loop,
                           float volume) const
{
  return client.playBuffer(channel, bytes::slice(sound), volume, loop);
}

ApiCode
ScriptApi::PluginSupports(string_view pluginID, PluginCallbackKey routine) const
{
  const size_t index = findPluginIndex(pluginID);
  if (index == noSuchPlugin) [[unlikely]] {
    return ApiCode::NoSuchPlugin;
  }
  return plugins[index].hasFunction(routine) ? ApiCode::OK
                                             : ApiCode::NoSuchRoutine;
}

ApiCode
ScriptApi::SendPacket(QByteArrayView bytes)
{
  ++totalPacketsSent;
  if (socket.write(bytes.data(), bytes.size()) == -1) [[unlikely]] {
    return ApiCode::WorldClosed;
  }

  return ApiCode::OK;
}

ApiCode
ScriptApi::SetAlphaOption(size_t plugin, string_view name, string_view value)
{
  return tab.setWorldAlphaOption(plugin, name, value);
}

QColor
ScriptApi::SetBackgroundColour(const QColor& color) const
{
  QWidget* background = tab.ui->background;
  QPalette palette = background->palette();
  const QColor oldColor = palette.color(QPalette::Base);
  if (color == Qt::GlobalColor::black) {
    palette.setColor(QPalette::Base, Qt::GlobalColor::transparent);
  } else if (color.isValid()) {
    palette.setColor(QPalette::Base, color);
  } else {
    palette.setColor(QPalette::Base, palette.color(QPalette::AlternateBase));
  }
  background->setPalette(palette);
  return oldColor == Qt::GlobalColor::transparent ? Qt::GlobalColor::black
                                                  : oldColor;
}

ApiCode
ScriptApi::SetCursor(Qt::CursorShape cursorShape) const
{
  tab.ui->area->setCursor(cursorShape);
  return ApiCode::OK;
}

QColor
ScriptApi::SetForegroundColour(const QColor& color) const
{
  QWidget* background = tab.ui->background;
  QPalette palette = background->palette();
  const QColor oldColor = palette.color(QPalette::Text);
  palette.setColor(QPalette::Text, color);
  palette.setColor(QPalette::HighlightedText, color);
  background->setPalette(palette);
  return oldColor;
}

QColor
ScriptApi::SetHighlightColour(const QColor& color) const
{
  QWidget* background = tab.ui->background;
  QPalette palette = background->palette();
  const QColor oldColor = palette.color(QPalette::Highlight);
  palette.setColor(QPalette::Highlight, color);
  background->setPalette(palette);
  return oldColor;
}

ApiCode
ScriptApi::SetOption(size_t plugin, string_view name, int64_t value)
{
  const ApiCode code = tab.setWorldOption(plugin, name, value);
  if (code != ApiCode::OK) {
    return code;
  }

  cursor->setOption(name, value);

  if (name == "display_my_input") {
    echoInput = value == 1;
  } else if (name == "naws") {
    doNaws = value == 1;
  } else if (name == "enable_scripts" && worldScriptIndex != noSuchPlugin) {
    setPluginEnabled(worldScriptIndex, value == 1);
  }

  return code;
}

void
ScriptApi::SetStatus(const QString& status) const
{
  statusBar->setMessage(status);
}

bool
ScriptApi::SetVariable(size_t index, string_view key, string_view value) const
{
  return client.setVariable(index, key, value);
}

void
ScriptApi::Simulate(string_view output) const
{
  tab.simulateOutput(output);
}

void
ScriptApi::StopEvaluatingTriggers() const
{
  client.stopSenders(SenderKind::Trigger);
}

ApiCode
ScriptApi::StopSound(size_t channel) const
{
  return client.stopSound(channel);
}

void
ScriptApi::Tell(const QString& text)
{
  cursor->appendTell(text);
}

ApiCode
ScriptApi::TextRectangle(const QMargins& margins,
                         int borderOffset,
                         const QColor& borderColor,
                         int borderWidth,
                         const QBrush& outsideFill) const
{
  Ui::WorldTab& ui = *tab.ui;
  QTextDocument& doc = *ui.output->document();
  doc.setLayoutEnabled(false);
  QPalette palette;

  palette.setBrush(QPalette::ColorRole::Window, outsideFill);
  ui.area->setPalette(palette);
  ui.area->setContentsMargins(margins);

  palette.setBrush(QPalette::ColorRole::Window, borderColor);
  ui.outputBorder->setPalette(palette);
  ui.outputBorder->setContentsMargins(
    borderWidth, borderWidth, borderWidth, borderWidth);

  ui.background->setContentsMargins(
    borderOffset, borderOffset, borderOffset, borderOffset);
  doc.setLayoutEnabled(true);
  return ApiCode::OK;
}

ApiCode
ScriptApi::TextRectangle(const OutputLayout& layout) const
{
  return TextRectangle(layout.margins,
                       layout.borderOffset,
                       layout.borderColor,
                       layout.borderWidth,
                       layout.outsideFill);
}

ApiCode
ScriptApi::TextRectangle(const QRect& rect,
                         int borderOffset,
                         const QColor& borderColor,
                         int borderWidth,
                         const QBrush& outsideFill)
{
  assignedTextRectangle = rect;
  const QSize size = tab.ui->area->size();
  const QMargins margins(
    rect.left(),
    rect.top(),
    rect.right() > 0 ? size.width() - rect.right() : -rect.right(),
    rect.bottom() > 0 ? size.height() - rect.bottom() : -rect.bottom());
  const OutputLayout layout{
    .margins = margins,
    .borderOffset = clamped_cast<int16_t>(borderOffset),
    .borderColor = borderColor,
    .borderWidth = clamped_cast<int16_t>(borderWidth),
    .outsideFill = outsideFill,
  };
  client.setMetavariable("output/layout", layout.save());
  return TextRectangle(layout);
}

ApiCode
ScriptApi::TextRectangle() const
{
  const QByteArrayView variable = client.getMetavariable("output/layout");
  if (variable.isNull()) {
    return ApiCode::OK;
  }

  OutputLayout layout;
  if (!layout.restore(QByteArray(variable))) {
    return ApiCode::VariableNotFound;
  }

  return TextRectangle(layout);
}
