use enumeration::Enum;
use serde::{Deserialize, Serialize};

#[derive(
    Copy, Clone, Debug, PartialEq, Eq, PartialOrd, Ord, Hash, Enum, Serialize, Deserialize,
)]
pub enum Callback {
    Install,
    Open,
    Close,
    ListChanged,

    Connect,
    Disconnect,

    SaveState,
    WorldSave,

    Enable,
    Disable,

    GetFocus,
    LoseFocus,

    Trace,
    PacketDebug,
    Broadcast,
    Screendraw,
    SelectionChanged,

    PlaySound,

    Send,
    Sent,
    PartialLine,
    LineReceived,
    PacketReceived,

    TelnetOption,
    TelnetRequest,
    TelnetSubnegotiation,
    IacGa,

    Command,
    CommandEntered,
    CommandChanged,
    TabComplete,

    WorldOutputResized,
    Tick,
    MouseMoved,

    MxpStart,
    MxpStop,
    MxpOpenTag,
    MxpCloseTag,
    MxpSetVariable,
    MxpSetEntity,
    MxpError,

    ChatAccept,
    ChatMessage,
    ChatMessageOut,
    ChatDisplay,
    ChatNewUser,
    ChatUserDisconnect,

    DrawOutputWindow,
}
