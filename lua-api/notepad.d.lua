---@meta

---Activates (makes the active window) the notepad window of the given title.
---@param title string Title of an existing notepad.
---@return boolean succeeded
---
---@see utils.activatenotepad - activate a non-world-associated notepad.
function ActivateNotepad(title) end

---Appends the text to a notepad window of the given title. If no such window exists, it is created.
---@param title string Notepad title.
---@param ... any Values to concatenate into text.
---@return boolean succeeded
---
---@see ReplaceNotepad - clear current text, then insert new text.
---@see SendToNotepad - create new notepad.
---@see utils.appendtonotepad - append to a non-world-associated notepad.
function AppendToNotepad(title, ...) end

---This closes a notepad window.
---@param title string Title of an existing notepad.
---@param querySave? boolean Whether to query to save changes, if the contents of the window have changed. Default: `false`.
---@return boolean succeeded
function CloseNotepad(title, querySave) end

---Find the length of the text in the specified notepad window.
---
---@param title string Title of an existing notepad.
---@return integer length Length in bytes, or 0 if a notepad of that name does not exist.
function GetNotepadLength(title) end

---Lists all open notepad windows.
---
---If more than one notepad has the same title you may get duplicates.
---@param all? boolean If `true`, all notepad windows are returned. If `false`, only related notepads for the current world are returned. Related notepads are those created from the world (eg. by [`AppendToNotepad`](lua://AppendToNotepad)), and are not ones manually created (eg. by the player or [`utils.appendtonotepad`](lua://utils.appendtonotepad)). Default: `false`.
---@return string[] notepadTitles List of notepad windows.
function GetNotepadList(all) end

---Gets the text from the specified notepad window.
---@param title string Title of an existing notepad.
---@return string text Notepad text, or an empty string if a notepad of that name does not exist.
function GetNotepadText(title) end

---This function returns the geometry of the specified notepad's window, in pixels.
---@param title string Title of an existing notepad.
---@return Rectangle | nil geometry Table containing position and size, or nil if a notepad of that name does not exist.
function GetNotepadWindowPosition(title) end

---Set the text and background colour of an open notepad window.
---@param title string Title of an existing notepad.
---@param foreground integer|string Integer BBGGRR colour code, string hex code, or string colour name.
---@param background integer|string Integer BBGGRR colour code, string hex code, or string colour name.
---@return boolean succeeded
function NotepadColour(title, foreground, background) end

---Changes the font, font size, style, and character set of the font for an open notepad window.
---@param title string Notepad title.
---@param fontName? string Font name. If nil or an empty string, font is not changed.
---@param size? number Font size. If nil or 0, font size is not changed.
---@param style? integer OR combination of style bits. Default: 0.
---bold: 1\
---italic: 2\
---underline: 4\
---strikeout: 8
---@return boolean succeeded
function NotepadFont(title, fontName, size, style) end

---This lets you mark a notepad window as read-only.
---
---This might be useful for help screens or other "output-only" information.
---@param title string Notepad title.
---@param readOnly? boolean Default: `true`.
---@return boolean succeeded
function NotepadReadOnly(title, readOnly) end

---This lets you change the save method for an open notepad window.
---@param title string Notepad title.
---@param method
---| 0 # Use default method (same as if this function had not been called)
---| 1 # Always offer to save this window if the contents have changed
---| 2 # Never offer to save this window if the contents have changed
---@return boolean succeeded
function NotepadSaveMethod(title, method) end

---Replaces the text in a notepad window of the given title. If no such window exists, it is created.
---@param title string Notepad title.
---@param ... any Values to concatenate into text.
---@return boolean succeeded
---
---@see AppendToNotepad - append text without clearing current text.
---@see SendToNotepad - create new notepad.
function ReplaceNotepad(title, ...) end

---Creates a notepad window with the nominated title and contents.
---
---The contents are copied "as is" without a trailing newline. If you want a newline you will need to add it yourself.
---@param title string Notepad title.
---@param ... any Values to concatenate into text.
---@return boolean succeeded
---
---@see AppendToNotepad - append text without clearing current text.
---@see ReplaceNotepad - clear current text, then insert new text.
function SendToNotepad(title, ...) end
