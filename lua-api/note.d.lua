---@meta

---Sends "Text" to the output window, changing colour, bold, underline etc. if it gets ANSI sequences in the text. ANSI sequences are things like `ESC [ 30 m`.
---
---WARNING: Bold italic, and underlined styles will only be visible if the appropriate configurations are enabled in the world Appearance: Output configuration. Otherwise they will be ignored (the same as for text arriving from the MUD).
---@param ... any Values to concatenate into text.
---
---@see ColourNote
---@see Note
function AnsiNote(...) end

---Sends a message to the output window in specified colours, terminating it with a newline.
---@param foreground integer|string|nil Integer BBGGRR colour code, string hex code, string colour name, or nil.
---@param background integer|string|nil Integer BBGGRR colour code, string hex code, string colour name, or nil.
---@param text any Value to concatenate into text.
---@param ... any More triples of the above.
---
---@see ColourTell - note without newline termination.
---@see Note - note with default colours.
function ColourNote(foreground, background, text, ...) end

---Sends a message to the output window in specified colours without appending a newline.
---@param foreground integer|string|nil Integer BBGGRR colour code, string hex code, string colour name, or nil.
---@param background integer|string|nil Integer BBGGRR colour code, string hex code, string colour name, or nil.
---@param text any Value to concatenate into text.
---@param ... any More triples of the above.
---
---@see ColourNote - note with automatic newline termination.
---@see Tell - tell with default colours.
function ColourTell(foreground, background, text, ...) end

---This gets the text style for [`Note`](lua://Note) and similar functions.
---@return integer style OR combination of style bits:
---bold: 1\
---underline: 2\
---italic: 4\
---inverse: 8\
---strike-through: 32
---
---@see NoteStyle - setter.
---@see NoteColourRGB - set note foreground and background colour.
function GetNoteStyle() end

---Creates a hyperlink in the output window.
---@param action string Action that will be sent if the link is clicked.
---@param text string Content of hyperlink.
---@param hint? string Prompt if the mouse hovers over the hyperlink. If empty, defaults to `action`.
---@param foreground? integer|string Integer BBGGRR colour code, string hex code, string colour name, or nil.
---@param background? integer|string Integer BBGGRR colour code, string hex code, string colour name, or nil.
---@param isUrl? boolean|0|1 Whether to open the link in a browser, rather than treating it as a command. Default: `false`.
---@param noUnderline? boolean Whether to remove the underline of the link. Default: `false`.
function Hyperlink(action, text, hint, foreground, background, isUrl, noUnderline) end

---This makes a note in the output window. It is not transmitted to the world. It is useful for making comments to yourself. The note is automatically terminated by a newline, so each Note will appear on a separate line.
---
---The note appears in the RGB colours selected by [`NoteColourRGB`](lua://NoteColourRGB) (or [`SetNoteColourFore`](lua://SetNoteColourFore) and [`SetNoteColourBack`](lua://SetNoteColourBack)) and the style selected by [`NoteStyle`](lua://NoteStyle).
---
---If you want to change colours in the middle of a note, use [`Tell`](lua://Tell) instead.
---@param ... any Values to concatenate into text.
---
---@see ColourNote - note with custom colours.
---@see NoteHr - draw a horizontal rule in the output window.
---@see Tell - note without newline termination.
function Note(...) end

---This draws a horizontal rule in the output window, the same as is done by the MXP `<hr>` sequence.
---
---@see Note
function NoteHr() end


---This sets the text style for [`Note`](lua://Note) and similar functions.
---
---The styles are 1 bit each, which can be added or OR'ed together.
---
---Therefore, `NoteStyle(0)` resets to normal style.
---@param style integer OR combination of style bits:
---bold: 1\
---underline: 2\
---italic: 4\
---inverse: 8\
---strike-through: 32
---
---@see NoteColourRGB - set default note colours.
function NoteStyle(style) end

---This makes a note in the output window. It is not transmitted to the world. It is useful for making comments to yourself. The note is NOT automatically terminated by a newline, so you can use `Tell` multiple times on one line (eg. to change colour in mid-line).
---
---The note appears in the RGB colours selected by [`NoteColourRGB`](lua://NoteColourRGB) (or [`SetNoteColourFore`](lua://SetNoteColourFore) and [`SetNoteColourBack`](lua://SetNoteColourBack)) and the style selected by [`NoteStyle`](lua://NoteStyle).
---
---A new line will automatically be started by output from the MUD, or a command that you type being echoed to the output window.
---@param ... any Values to concatenate into text.
---
---@see Note - note with automatic newline termination.
function Tell(...) end
