#pragma once

#include "rust/cxx.h"
#include <QtGui/QTextCursor>

struct SendRequest;
struct SendScriptRequest;
enum class AliasOutcome;
enum class DynamicColor : uint8_t;
enum class TelnetSource : uint8_t;
enum class TelnetVerb : uint8_t;

Q_DECLARE_FLAGS(AliasOutcomes, AliasOutcome);
Q_DECLARE_OPERATORS_FOR_FLAGS(AliasOutcomes);

class AbstractDocument : public QObject
{
public:
  explicit AbstractDocument(QObject* parent = nullptr)
    : QObject(parent)
  {
  }

  virtual void appendHtml(const QString& html) const = 0;
  virtual void appendExpiringLink(const QString& text,
                                  const QTextCharFormat& format,
                                  rust::Str expires) = 0;
  virtual void appendLine() = 0;
  virtual void appendText(const QString& text,
                          const QTextCharFormat& format) const = 0;
  virtual void applyStyles(int start,
                           int end,
                           const QTextCharFormat& format) const = 0;
  virtual void beep() const = 0;
  virtual void begin() const = 0;
  virtual void clear() const = 0;
  virtual void createMxpStat(const QString& entity,
                             const QString& caption,
                             const QString& max) const = 0;
  virtual void echo(const QString& text) const = 0;
  virtual void end(bool hadOutput) = 0;
  virtual void eraseCharacters(QTextCursor::MoveOperation direction,
                               int n) const = 0;
  virtual void eraseCurrentLine() const = 0;
  virtual void eraseLastLine() const = 0;
  virtual void expireLinks(rust::Str expires) = 0;
  virtual void handleMxpChange(bool enabled) const = 0;
  virtual void handleMxpEntity(rust::Str data) const = 0;
  virtual void handleMxpVariable(rust::Str name, rust::Str value) const = 0;
  virtual void handleServerStatus(rust::Slice<const uint8_t> variable,
                                  rust::Slice<const uint8_t> value) = 0;
  virtual void handleTelnetGoAhead() const = 0;
  virtual void handleTelnetNaws() const = 0;
  virtual void handleTelnetNegotiation(TelnetSource source,
                                       TelnetVerb verb,
                                       uint8_t code) = 0;
  virtual void handleTelnetSubnegotiation(
    uint8_t code,
    rust::Slice<const uint8_t> data) const = 0;
  virtual void moveCursor(QTextCursor::MoveOperation op, int count) const = 0;
  virtual bool permitLine(rust::Str line) const = 0;
  virtual bool permitSound(rust::Str file) const = 0;
  virtual void send(const SendRequest& request) const = 0;
  virtual void send(const SendScriptRequest& request) const = 0;
  virtual void setDynamicColor(DynamicColor dynamic,
                               const QColor& color) const = 0;
  virtual void setSuppressEcho(bool suppress) const = 0;
  virtual void updateMxpStat(const QString& entity,
                             const QString& value) const = 0;
};
