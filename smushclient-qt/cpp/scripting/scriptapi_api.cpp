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
#include "worldproperties.h"
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

inline QColor getColorFromVariant(const QVariant &variant) {
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

inline bool isEmptyList(const QVariant &variant) {
  switch (variant.typeId()) {
  case QMetaType::QStringList:
    return variant.toStringList().isEmpty();
  case QMetaType::QVariantList:
    return variant.toList().isEmpty();
  default:
    return false;
  }
}

constexpr ApiCode convertSoundResult(SoundResult result) noexcept {
  switch (result) {
  case SoundResult::Ok:
    return ApiCode::OK;
  case SoundResult::SoundError:
    return ApiCode::CannotPlaySound;
  case SoundResult::BadParameter:
    return ApiCode::BadParameter;
  case SoundResult::NotFound:
    return ApiCode::FileNotFound;
  }
}

inline ApiCode updateWorld(WorldTab &worldtab) {
  return worldtab.updateWorld() ? ApiCode::OK : ApiCode::OptionOutOfRange;
}

// Public static methods

int ScriptApi::GetUniqueNumber() noexcept {
  static int uniqueNumber = -1;
  if (uniqueNumber == INT_MAX) [[unlikely]]
    uniqueNumber = 0;
  else
    ++uniqueNumber;
  return uniqueNumber;
}

QString ScriptApi::MakeRegularExpression(const QString &pattern) noexcept {
  return ffi::makeRegexFromWildcards(pattern);
}

void ScriptApi::SetClipboard(const QString &text) {
  QGuiApplication::clipboard()->setText(text);
}

// Public methods

int ScriptApi::BroadcastPlugin(size_t index, int message,
                               string_view text) const {
  const Plugin &callingPlugin = plugins[index];
  int calledPlugins = 0;
  OnPluginBroadcast onBroadcast(message, callingPlugin.id(),
                                callingPlugin.name(), text);
  for (const Plugin &plugin : plugins)
    if (&plugin != &callingPlugin)
      calledPlugins += plugin.runCallbackThreaded(onBroadcast);
  return calledPlugins;
}

void ScriptApi::ColourTell(const QColor &foreground, const QColor &background,
                           const QString &text) {
  QTextCharFormat format = cursor.charFormat();
  if (foreground.isValid())
    format.setForeground(QBrush(foreground));
  if (background.isValid())
    format.setBackground(QBrush(background));
  appendTell(text, format);
}

int ScriptApi::DatabaseClose(string_view databaseID) {
  auto search = databases.find(databaseID);
  if (search == databases.end()) [[unlikely]]
    return -1;

  const int result = search->second.close();
  databases.erase(search);
  return result;
}

int ScriptApi::DatabaseOpen(string_view databaseID, string_view filename,
                            int flags) {
  auto search = databases.emplace((string)databaseID, filename);
  DatabaseConnection &db = search.first->second;
  if (!search.second)
    return db.isFile(databaseID) ? SQLITE_OK : -6;

  const int result = db.open(flags);
  if (result != SQLITE_OK)
    databases.erase(search.first);

  return result;
}

ApiCode ScriptApi::DeleteVariable(size_t plugin, string_view key) const {
  return client()->unsetVariable(plugin, key) ? ApiCode::OK
                                              : ApiCode::VariableNotFound;
}

ApiCode ScriptApi::DoAfter(size_t plugin, double seconds, const QString &text,
                           SendTarget target) {
  if (seconds < 0.1 || seconds > 86399)
    return ApiCode::TimeInvalid;
  const milliseconds duration = milliseconds{(int)(seconds * 1000.0)};
  sendQueue->start(duration,
                   {.plugin = plugin, .target = target, .text = text});
  return ApiCode::OK;
}

ApiCode ScriptApi::EnablePlugin(string_view pluginID, bool enabled) {
  const size_t index = findPluginIndex(pluginID);
  if (index == noSuchPlugin)
    return ApiCode::NoSuchPlugin;
  plugins[index].disable();
  client()->setPluginEnabled(index, enabled);
  OnPluginListChanged onListChanged;
  sendCallback(onListChanged);
  return ApiCode::OK;
}

int ScriptApi::GetLinesInBufferCount() const {
  return cursor.document()->lineCount();
}

QVariant ScriptApi::GetOption(string_view name) const {
  const char *prop = WorldProperties::canonicalName(name);
  if (!prop)
    return QVariant();

  return tab->world.property(prop);
}

VariableView ScriptApi::GetVariable(size_t index, string_view key) const {

  return client()->getVariable(index, key);
}

VariableView ScriptApi::GetVariable(string_view pluginID,
                                    string_view key) const {
  const size_t index = findPluginIndex(pluginID);
  if (index == noSuchPlugin)
    return VariableView(nullptr, 0);
  return GetVariable(index, key);
}

const string &ScriptApi::GetPluginID(size_t index) const {
  return plugins.at(index).id();
}

void ScriptApi::Hyperlink(const QString &action, const QString &text,
                          const QString &hint, const QColor &foreground,
                          const QColor &background, bool url,
                          bool noUnderline) {
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

QColor ScriptApi::PickColour(const QColor &hint) const {
  return QColorDialog::getColor(hint, tab);
}

ApiCode ScriptApi::PlaySound(size_t channel, const QString &path, bool loop,
                             float volume) {
  return convertSoundResult(client()->playFile(channel, path, volume, loop));
}

ApiCode ScriptApi::PlaySoundMemory(size_t channel, QByteArrayView sound,
                                   bool loop, float volume) {
  return convertSoundResult(
      client()->playBuffer(channel, rust::Slice(sound), volume, loop));
}

ApiCode ScriptApi::PluginSupports(string_view pluginID,
                                  PluginCallbackKey routine) const {
  const size_t index = findPluginIndex(pluginID);
  if (index == noSuchPlugin) [[unlikely]]
    return ApiCode::NoSuchPlugin;
  return plugins[index].hasFunction(routine) ? ApiCode::OK
                                             : ApiCode::NoSuchRoutine;
}

ApiCode ScriptApi::Send(QByteArray &bytes) {
  echo(QString::fromUtf8(bytes));
  return SendNoEcho(bytes);
}

ApiCode ScriptApi::Send(const QString &text) {
  echo(text);
  QByteArray bytes = text.toUtf8();
  return SendNoEcho(bytes);
}

ApiCode ScriptApi::SendNoEcho(QByteArray &bytes) {
  OnPluginSend onSend(bytes);
  sendCallback(onSend);
  if (onSend.discarded())
    return ApiCode::OK;
  bytes.append("\r\n");

  const qsizetype size = bytes.size();
  if (socket->write(bytes.constData(), size) == -1) [[unlikely]]
    return ApiCode::WorldClosed;
  bytes.truncate(size - 2);

  OnPluginSent onSent(bytes);
  sendCallback(onSent);
  return ApiCode::OK;
}

ApiCode ScriptApi::SendPacket(QByteArrayView view) const {
  if (socket->write(view.data(), view.size()) == -1) [[unlikely]]
    return ApiCode::WorldClosed;

  return ApiCode::OK;
}

ApiCode ScriptApi::SetCursor(Qt::CursorShape cursorShape) const {
  tab->ui->area->setCursor(cursorShape);
  return ApiCode::OK;
}

ApiCode ScriptApi::SetOption(string_view name, const QVariant &variant) const {
  World &world = tab->world;
  const char *prop = WorldProperties::canonicalName(name);
  if (!prop) [[unlikely]]
    return ApiCode::UnknownOption;
  QVariant property = world.property(prop);
  if (world.setProperty(prop, variant))
    return updateWorld(*tab);

  switch (property.typeId()) {
  case QMetaType::QColor:
    if (QColor color = getColorFromVariant(variant);
        color.isValid() && world.setProperty(prop, color))
      return updateWorld(*tab);
  case QMetaType::QVariantHash:
    if (isEmptyList(variant) && world.setProperty(prop, QVariantHash()))
      return updateWorld(*tab);
  case QMetaType::QVariantMap:
    if (isEmptyList(variant) && world.setProperty(prop, QVariantMap()))
      return updateWorld(*tab);
  }

  return ApiCode::OptionOutOfRange;
}

void ScriptApi::SetStatus(const QString &status) const {
  statusBar->setMessage(status);
}

bool ScriptApi::SetVariable(size_t index, string_view key,
                            string_view value) const {
  return client()->setVariable(index, key, value);
}

void ScriptApi::StopEvaluatingTriggers() const {
  return client()->stopTriggers();
}

ApiCode ScriptApi::StopSound(size_t channel) {
  return convertSoundResult(client()->stopSound(channel));
}

void ScriptApi::Tell(const QString &text) { appendTell(text, noteFormat); }

ApiCode ScriptApi::TextRectangle(const QMargins &margins, int borderOffset,
                                 const QColor &borderColor, int borderWidth,
                                 const QBrush &outsideFill) const {
  Ui::WorldTab *ui = tab->ui;
  QTextDocument *doc = ui->output->document();
  doc->setLayoutEnabled(false);
  ui->area->setContentsMargins(margins);
  QPalette areaPalette = ui->area->palette();
  QBrush outsideBrush = outsideFill;

  if (!outsideBrush.color().isValid())
    outsideBrush.setColor(tab->world.getAnsi0());

  const QColor &borderColorFill =
      borderColor.isValid() ? borderColor : outsideBrush.color();

  areaPalette.setBrush(QPalette::ColorRole::Window, outsideBrush);
  ui->area->setPalette(areaPalette);
  ui->outputBorder->setContentsMargins(borderWidth, borderWidth, borderWidth,
                                       borderWidth);
  QPalette borderPalette = ui->outputBorder->palette();
  borderPalette.setBrush(QPalette::ColorRole::Window, borderColorFill);
  ui->outputBorder->setPalette(borderPalette);
  ui->background->setContentsMargins(borderOffset, borderOffset, borderOffset,
                                     borderOffset);
  doc->setLayoutEnabled(true);
  return ApiCode::OK;
}

ApiCode ScriptApi::TextRectangle(const OutputLayout &layout) const {
  return TextRectangle(layout.margins, layout.borderOffset, layout.borderColor,
                       layout.borderWidth, layout.outsideFill);
}

ApiCode ScriptApi::TextRectangle(const QRect &rect, int borderOffset,
                                 const QColor &borderColor, int borderWidth,
                                 const QBrush &outsideFill) const {
  const QSize size = tab->ui->area->size();
  const QMargins margins(rect.left(), rect.top(), size.width() - rect.right(),
                         size.height() - rect.bottom());
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

ApiCode ScriptApi::TextRectangle() const {
  const QByteArrayView variable = client()->getMetavariable("output/layout");
  if (!variable.data())
    return ApiCode::OK;

  OutputLayout layout;
  if (!layout.restore(QByteArray(variable)))
    return ApiCode::VariableNotFound;

  return TextRectangle(layout);
}
