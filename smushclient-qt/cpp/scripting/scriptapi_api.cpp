#include "../../layout.h"
#include "../../spans.h"
#include "../timer_map.h"
#include "../ui/mudstatusbar/mudstatusbar.h"
#include "../ui/ui_worldtab.h"
#include "../ui/worldtab.h"
#include "miniwindow.h"
#include "scriptapi.h"
#include "smushclient_qt/src/ffi/document.cxxqt.h"
#include "smushclient_qt/src/ffi/util.cxx.h"
#include "sqlite3.h"
#include <QtCore/QFile>
#include <QtGui/QClipboard>
#include <QtGui/QGradient>
#include <QtGui/QGuiApplication>
#include <QtWidgets/QColorDialog>
#include <QtWidgets/QStatusBar>

using std::nullopt;
using std::optional;
using std::string;
using std::string_view;
using std::chrono::hours;
using std::chrono::milliseconds;
using std::chrono::minutes;
using std::chrono::seconds;

// Private utils

inline QColor
getColorFromVariant(const QVariant& variant)
{
  if (variant.canConvert<QString>()) {
    QString colorName = variant.toString();
    return colorName.isEmpty() ? QColor::fromRgb(0, 0, 0, 0)
                               : QColor::fromString(colorName);
  }
  bool ok;
  int rgb = variant.toInt(&ok);
  if (!ok || rgb < 0 || rgb > 0xFFFFFF)
    return QColor();
  return QColor(rgb & 0xFF, (rgb >> 8) & 0xFF, (rgb >> 16) & 0xFF);
}

inline bool
isEmptyList(const QVariant& variant)
{
  switch (variant.typeId()) {
    case QMetaType::QStringList:
      return variant.toStringList().isEmpty();
    case QMetaType::QVariantList:
      return variant.toList().isEmpty();
    default:
      return false;
  }
}

// Public static methods

QStringList
ScriptApi::GetAlphaOptionList() noexcept
{
  return ffi::getAlphaOptionList();
}

QStringList
ScriptApi::GetOptionList() noexcept
{
  return ffi::getOptionList();
}

long
ScriptApi::GetUniqueNumber() noexcept
{
  static long uniqueNumber = -1;
  if (uniqueNumber == INT64_MAX) [[unlikely]]
    uniqueNumber = 0;
  else
    ++uniqueNumber;
  return uniqueNumber;
}

QString
ScriptApi::MakeRegularExpression(string_view pattern) noexcept
{
  return ffi::makeRegexFromWildcards(byteSlice(pattern));
}

void
ScriptApi::SetClipboard(const QString& text)
{
  QGuiApplication::clipboard()->setText(text);
}

// Public methods

long
ScriptApi::BroadcastPlugin(size_t index, long message, string_view text) const
{
  const Plugin& callingPlugin = plugins[index];
  long calledPlugins = 0;
  OnPluginBroadcast onBroadcast(
    message, callingPlugin.id(), callingPlugin.name(), text);
  for (const Plugin& plugin : plugins)
    if (&plugin != &callingPlugin)
      calledPlugins += plugin.runCallbackThreaded(onBroadcast);
  return calledPlugins;
}

void
ScriptApi::ColourTell(const QColor& foreground,
                      const QColor& background,
                      const QString& text)
{
  QTextCharFormat format = cursor.charFormat();
  if (foreground.isValid())
    format.setForeground(foreground);
  if (background.isValid())
    format.setBackground(background);
  appendTell(text, format);
}

int
ScriptApi::DatabaseClose(string_view databaseID)
{
  auto search = databases.find(databaseID);
  if (search == databases.end()) [[unlikely]]
    return -1;

  const int result = search->second.close();
  databases.erase(search);
  return result;
}

int
ScriptApi::DatabaseOpen(string_view databaseID, string_view filename, int flags)
{
  auto search = databases.emplace((string)databaseID, filename);
  DatabaseConnection& db = search.first->second;
  if (!search.second)
    return db.isFile(databaseID) ? SQLITE_OK : -6;

  const int result = db.open(flags);
  if (result != SQLITE_OK)
    databases.erase(search.first);

  return result;
}

ApiCode
ScriptApi::DeleteVariable(size_t plugin, string_view key) const
{
  return client()->unsetVariable(plugin, key) ? ApiCode::OK
                                              : ApiCode::VariableNotFound;
}

ApiCode
ScriptApi::DoAfter(size_t plugin,
                   double seconds,
                   const QString& text,
                   SendTarget target)
{
  if (seconds < 0.1 || seconds > 86399)
    return ApiCode::TimeInvalid;
  const milliseconds duration = milliseconds{ (int)(seconds * 1000.0) };
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
  if (index == noSuchPlugin)
    return ApiCode::NoSuchPlugin;
  plugins[index].disable();
  client()->setPluginEnabled(index, enabled);
  OnPluginListChanged onListChanged;
  sendCallback(onListChanged);
  return ApiCode::OK;
}

VariableView
ScriptApi::GetAlphaOption(size_t plugin, string_view name) const
{
  return client()->worldAlphaOption(plugin, byteSlice(name));
}

int
ScriptApi::GetLinesInBufferCount() const
{
  return cursor.document()->lineCount();
}

QVariant
ScriptApi::GetCurrentValue(size_t pluginIndex, string_view option) const
{
  return client()->worldVariantOption(pluginIndex, byteSlice(option));
}

long
ScriptApi::GetOption(size_t plugin, string_view name) const
{
  return client()->worldOption(plugin, byteSlice(name));
}

VariableView
ScriptApi::GetVariable(size_t index, string_view key) const
{

  return client()->getVariable(index, key);
}

VariableView
ScriptApi::GetVariable(string_view pluginID, string_view key) const
{
  const size_t index = findPluginIndex(pluginID);
  if (index == noSuchPlugin)
    return VariableView(nullptr, 0);
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
  format.setAnchorHref(
    encodeLink(url ? SendTo::Internet : SendTo::World, action));
  format.setToolTip(hint.isEmpty() ? action : hint);
  if (foreground.isValid())
    format.setForeground(foreground);
  if (background.isValid())
    format.setBackground(background);
  if (!noUnderline)
    format.setAnchor(true);
  appendTell(text, format);
}

QColor
ScriptApi::PickColour(const QColor& hint) const
{
  return QColorDialog::getColor(hint, tab);
}

ApiCode
ScriptApi::PlaySound(size_t channel, string_view path, bool loop, float volume)
{
  return client()->playFile(channel, byteSlice(path), volume, loop);
}

ApiCode
ScriptApi::PlaySoundMemory(size_t channel,
                           QByteArrayView sound,
                           bool loop,
                           float volume)
{
  return client()->playBuffer(channel, byteSlice(sound), volume, loop);
}

ApiCode
ScriptApi::PluginSupports(string_view pluginID, PluginCallbackKey routine) const
{
  const size_t index = findPluginIndex(pluginID);
  if (index == noSuchPlugin) [[unlikely]]
    return ApiCode::NoSuchPlugin;
  return plugins[index].hasFunction(routine) ? ApiCode::OK
                                             : ApiCode::NoSuchRoutine;
}

ApiCode
ScriptApi::SendPacket(QByteArrayView view)
{
  ++totalPacketsSent;
  if (socket->write(view.data(), view.size()) == -1) [[unlikely]]
    return ApiCode::WorldClosed;

  return ApiCode::OK;
}

ApiCode
ScriptApi::SetAlphaOption(size_t plugin, string_view name, string_view value)
{
  return tab->setWorldAlphaOption(plugin, name, value);
}

ApiCode
ScriptApi::SetCursor(Qt::CursorShape cursorShape) const
{
  tab->ui->area->setCursor(cursorShape);
  return ApiCode::OK;
}

ApiCode
ScriptApi::SetOption(size_t plugin, string_view name, long value)
{
  const ApiCode code = tab->setWorldOption(plugin, name, value);
  if (code != ApiCode::OK)
    return code;

  if (name == "echo_colour")
    echoFormat.setForeground(QColor(value));
  else if (name == "echo_background_colour")
    echoFormat.setBackground(QColor(value));
  else if (name == "display_my_input")
    echoInput = value == 1;
  else if (name == "error_text_colour")
    errorFormat.setForeground(QColor(value));
  else if (name == "error_background_colour")
    errorFormat.setBackground(QColor(value));
  else if (name == "indent_paras")
    indentText = QStringLiteral(" ").repeated(value);
  else if (name == "keep_commands_on_same_line")
    echoOnSameLine = value == 1;
  else if (name == "log_notes")
    logNotes = value == 1;
  else if (name == "naws")
    doNaws = value == 1;
  else if (name == "note_text_colour")
    noteFormat.setForeground(QColor(value));
  else if (name == "note_background_colour")
    noteFormat.setBackground(QColor(value));
  else if (name == "no_echo_off") {
    if (value == 1)
      suppressEcho = false;
  } else if (name == "enable_scripts") {
    if (worldScriptIndex == noSuchPlugin)
      ;
    else if (value == 1)
      plugins[worldScriptIndex].enable();
    else
      plugins[worldScriptIndex].disable();
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
  return client()->setVariable(index, key, value);
}

void
ScriptApi::Simulate(string_view line) const
{
  tab->simulateOutput(line);
}

void
ScriptApi::StopEvaluatingTriggers() const
{
  return client()->stopTriggers();
}

ApiCode
ScriptApi::StopSound(size_t channel)
{
  return client()->stopSound(channel);
}

void
ScriptApi::Tell(const QString& text)
{
  appendTell(text, noteFormat);
}

ApiCode
ScriptApi::TextRectangle(const QMargins& margins,
                         int borderOffset,
                         const QColor& borderColor,
                         int borderWidth,
                         const QBrush& outsideFill) const
{
  Ui::WorldTab* ui = tab->ui;
  QTextDocument* doc = ui->output->document();
  doc->setLayoutEnabled(false);
  ui->area->setContentsMargins(margins);
  QPalette areaPalette = ui->area->palette();
  QBrush outsideBrush = outsideFill;

  if (!outsideBrush.color().isValid())
    outsideBrush.setColor(ui->background->palette().color(QPalette::Base));

  const QColor& borderColorFill =
    borderColor.isValid() ? borderColor : outsideBrush.color();

  areaPalette.setBrush(QPalette::ColorRole::Window, outsideBrush);
  ui->area->setPalette(areaPalette);
  ui->outputBorder->setContentsMargins(
    borderWidth, borderWidth, borderWidth, borderWidth);
  QPalette borderPalette = ui->outputBorder->palette();
  borderPalette.setBrush(QPalette::ColorRole::Window, borderColorFill);
  ui->outputBorder->setPalette(borderPalette);
  ui->background->setContentsMargins(
    borderOffset, borderOffset, borderOffset, borderOffset);
  doc->setLayoutEnabled(true);
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
  const QSize size = tab->ui->area->size();
  const QMargins margins(
    rect.left(),
    rect.top(),
    rect.right() > 0 ? size.width() - rect.right() : -rect.right(),
    rect.bottom() > 0 ? size.height() - rect.bottom() : -rect.bottom());
  const OutputLayout layout{
    .margins = margins,
    .borderOffset = (int16_t)borderOffset,
    .borderColor = borderColor,
    .borderWidth = (int16_t)borderWidth,
    .outsideFill = outsideFill,
  };
  client()->setMetavariable("output/layout", layout.save());
  return TextRectangle(layout);
}

ApiCode
ScriptApi::TextRectangle() const
{
  const QByteArrayView variable = client()->getMetavariable("output/layout");
  if (!variable.data())
    return ApiCode::OK;

  OutputLayout layout;
  if (!layout.restore(QByteArray(variable)))
    return ApiCode::VariableNotFound;

  return TextRectangle(layout);
}
