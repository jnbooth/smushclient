#include "../../ui/ui_worldtab.h"
#include "../../ui/worldtab.h"
#include "../scriptapi.h"

using std::string_view;

// Public methods

void
ScriptApi::DeleteCommandHistory() const
{
  tab.ui->input->clearLog();
}

ApiCode
ScriptApi::Execute(const QString& command) const noexcept
{
  if (!tab.isConnected()) {
    return ApiCode::WorldClosed;
  }
  tab.sendCommand(command, CommandSource::Execute);
  return ApiCode::OK;
}

QString
ScriptApi::GetCommand() const
{
  return tab.ui->input->toPlainText();
}

const QStringList&
ScriptApi::GetCommandList() const
{
  return tab.ui->input->log();
}

ApiCode
ScriptApi::LogSend(QByteArray& bytes)
{
  return sendToWorld(bytes, SendFlag::Echo | SendFlag::Log);
}

QString
ScriptApi::PasteCommand(const QString& command) const
{
  QTextCursor cursor = tab.ui->input->textCursor();
  const QString selectedText = cursor.selectedText();
  if (!selectedText.isEmpty()) {
    cursor.removeSelectedText();
  }
  cursor.insertText(command);
  tab.ui->input->setTextCursor(cursor);
  return selectedText;
}

QString
ScriptApi::PushCommand() const
{
  const QString command = tab.ui->input->toPlainText();
  if (!command.isEmpty()) {
    tab.ui->input->remember(command);
    tab.ui->input->clear();
  }
  return command;
}

void
ScriptApi::SelectCommand() const
{
  tab.ui->input->selectAll();
}

ApiCode
ScriptApi::Send(string_view text)
{
  QByteArray bytes(text);
  return sendToWorld(bytes, SendFlag::Echo);
}

ApiCode
ScriptApi::Send(const QString& text)
{
  return sendToWorld(text, SendFlag::Echo);
}

ApiCode
ScriptApi::Send(QByteArray& bytes)
{
  return sendToWorld(bytes, SendFlag::Echo);
}

ApiCode
ScriptApi::SendImmediate(QByteArray& bytes)
{
  return sendToWorld(bytes, SendFlag::Echo | SendFlag::Immediate);
}

ApiCode
ScriptApi::SendNoEcho(QByteArray& bytes)
{
  return sendToWorld(bytes, SendFlags());
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
ScriptApi::SendPush(QByteArray& bytes)
{
  const QString command = QString::fromUtf8(bytes);
  tab.ui->input->remember(command);
  return sendToWorld(bytes, command, SendFlag::Echo);
}

ApiCode
ScriptApi::SetCommand(const QString& command) const
{
  if (!tab.ui->input->toPlainText().isEmpty()) {
    return ApiCode::CommandNotEmpty;
  }
  tab.ui->input->setPlainText(command);
  return ApiCode::OK;
}

ApiCode
ScriptApi::SetCommandSelection(int first, int last) const
{
  QTextCursor cursor = tab.ui->input->textCursor();
  if (first == 0) {
    cursor.clearSelection();
    tab.ui->input->setTextCursor(cursor);
    return ApiCode::OK;
  }
  cursor.setPosition(first - 1, QTextCursor::MoveMode::MoveAnchor);
  if (last == -1) {
    cursor.movePosition(QTextCursor::MoveOperation::End,
                        QTextCursor::MoveMode::KeepAnchor);
  } else {
    cursor.setPosition(last - 1, QTextCursor::MoveMode::KeepAnchor);
  }
  tab.ui->input->setTextCursor(cursor);
  return ApiCode::OK;
}

ApiCode
ScriptApi::SetCommandWindowHeight(int height) const
{
  if (height < 0) {
    return ApiCode::BadParameter;
  }
  if (height == 0) {
    tab.ui->input->hide();
    return ApiCode::OK;
  }
  tab.ui->input->show();
  tab.ui->input->setFixedHeight(height);
  return ApiCode::OK;
}
