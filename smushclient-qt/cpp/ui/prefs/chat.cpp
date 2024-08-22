#include "chat.h"
#include "ui_chat.h"

PrefsChat::PrefsChat(const World *world, QWidget *parent)
    : AbstractPrefsPane(parent), ui(new Ui::PrefsChat)
{
  ui->setupUi(this);
  CONNECT_WORLD(ChatName);
  CONNECT_WORLD(ChatName);
  CONNECT_WORLD(AutoAllowSnooping);
  CONNECT_WORLD(AcceptChatConnections);
  CONNECT_WORLD(ChatPort);
  CONNECT_WORLD(ValidateIncomingChatCalls);
  CONNECT_WORLD(ChatColorsForeground);
  CONNECT_WORLD(ChatColorsBackground);
  CONNECT_WORLD(IgnoreChatColors);
  CONNECT_WORLD(ChatMessagePrefix);
  CONNECT_WORLD(ChatMaxLinesPerMessage);
  CONNECT_WORLD(ChatMaxBytesPerMessage);
  CONNECT_WORLD(AutoAllowFiles);
  CONNECT_WORLD(ChatFileSaveDirectory);
}

PrefsChat::~PrefsChat()
{
  delete ui;
}
