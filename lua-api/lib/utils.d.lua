---@meta

utils = {}

---@alias MsgBoxType
---| "ok" # The message box contains one push button: OK.
---| "abortretryignore" # The message box contains three push buttons: Abort, Retry, and Ignore.
---| "okcancel" # The message box contains two push buttons: OK and Cancel.
---| "retrycancel" # The message box contains two push buttons: Retry and Cancel.
---| "yesno" # The message box contains two push buttons: Yes and No.
---| "yesnocancel" # The message box contains three push buttons: Yes, No, and Cancel.

---@alias MsgBoxIcon
---| "!" # An exclamation-point icon appears in the message box.
---| "?" # A question-mark icon appears in the message box.
---| "i" # An icon consisting of a lowercase letter i in a circle appears in the message box.
---| "." # A stop-sign icon appears in the message box.

---@class utils.inputconfig
---@field max_length integer? Maximum number of characters they can type.
---@field validate (fun(input: string): boolean)?  When the user clicks the OK button, this function is called with their input. If it returns `true`, the process completes. If not, they are required to alter their input.
---@field ok_button string? What text to put on the OK button. Default: `"OK"`.
---@field cancel_button string? What text to put on the Cancel button. Default: `"Cancel"`.
---@field read_only boolean? If `true`, input area is read-only.
---@field no_default boolean? IF `true`, pressing \<enter\> does not dismiss the dialog.

---@class utils.font
---@field name string Chosen name (eg. "Comic Sans MS").
---@field size number Size in points (eg. 12).
---@field colour integer Integer BBGGRR colour code, string hex code, or string colour name.
---@field underline boolean Use underline.
---@field strikeout boolean Use strike-out.
---@field italic boolean Use italic.
---@field bold boolean Font weight is bold or heavier.
---@field style string The style as a string (eg. "Bold Italic").

---@class utils.infotable
---@field current_directory string The current directory.
---@field app_directory string The directory in which the application executable resides.
---@field world_files_directory string The default world files directory.
---@field log_files_directory string The default log files directory.
---@field plugins_files_directory string The default plugins directory.
---@field startup_directory string The application startup directory.
---@field locale string Current locale identifier (eg. EN).
---@field fixed_pitch_font string Font used for fixed-pitch dialogs.
---@field fixed_pitch_font_size number Point size of font used for fixed-pitch dialogs.
---@field translator_file string File used for translating messages.

---Activates the nominated notepad window, which does not belong to any world.
---
---This is intended to be used for debugging or displaying information (eg. during development of localization scripts) in a notepad window, which is not related to any particular world.
---@param title string Notepad title.
---@return boolean succeeded (Notepad exists.)
---
---@see ActivateNotepad - activate a world-associated notepad.
function utils.activatenotepad(title) end

---Appends text to the nominated notepad window, which does not belong to any world.
---
---This is intended to be used for debugging or displaying information (eg. during development of localization scripts) in a notepad window, which is not related to any particular world.
---@param title string Notepad title.
---@param message string Text to append.
---@param replace boolean If `true`, erase the previous contents of the notepad.
---@return boolean succeeded
---
---@see AppendToNotepad - append to a world-associated notepad.
function utils.appendtonotepad(title, message, replace) end

---Decodes a base-64 string.
---@param encoded string
---@return string text
function utils.base64decode(encoded) end

---Encodes a string to base-64.
---@param text string
---@return string encoded
function utils.base64encode(text) end

---This returns a table of all the internal plugin callback function names.
---
---This could be used to detect whether or not a particular callback was available in this version of SmushClient.
---@return string[] t Array of internal plugin callback function names.
function utils.callbackslist() end

---This displays a dialog box with a predetermined list of items for the user to choose from, in the form of a combo-box.
---@generic T
---@param msg string Message to display.
---@param title? string Title of box. Default: "SmushClient".
---@param t? { [T]: string|number } Table of key/value pairs. The value is displayed, however the corresponding key is returned. The values are automatically sorted into ascending alphabetic order.
---@param default? T Default (preselected) key, or nil for no selection. If there is no entry associated with that key in *t*, nothing is preselected.
---@return T|nil result The key of the selected item, or nil if cancelled, or nothing selected.
---
---@see utils.listbox - Displays a dialog box with a list control in it - single selection. Suitable for longer lists.
---@see utils.multilistbox - Displays a dialog box with a list control in it - multiple selections allowed. This is useful when you want the user to be able to select multiple items.
function utils.choose(msg, title, t, default) end

---Compresses string s and returns the compressed form.
---
---For short strings the compressed data may be longer than the uncompressed data because of a 12-byte "compression information" header that is prepended to the compressed data. For longer text, the compression ratio is about 50%.
---@param s string Uncompressed data.
---@param method? integer Desired level of compression. 0 = no compression, 1 = best speed, 9 = best compression. Default: 6.
---@return string s Compressed form of `s`. May contain nulls (bytes with zero value).
function utils.compress(s, method) end

---Decompresses string s and returns the decompressed form. Raises an error if decompression cannot be done (eg. bad compressed data).
---
---The two functions utils.compress and utils.decompress should be complementary, so that this should always be true:
---
---x = "some string" -- for any (string) data whatsoever\
---y = utils.decompress (utils.compress (x)) --> y should be same as x
---@param s string Compressed data.
---@return string s Decompressed form of `s`.
function utils.decompress(s) end

---This invokes the Windows standard "directory picker" dialog box, which lets you choose a directory.
---@param title? string Title to appear on the dialog box (eg. "Directory for backups").
---@param initial? string Initial directory. Default: current directory.
---@return string|nil directoryname Chosen directory, or nil if dialog dismissed. The directory name will not haave a trailing slash.
function utils.directorypicker(title, initial) end

---This is almost identical to [`utils.inputbox`](lua://utils.inputbox), except that the response field:
---
---* is much larger - for entering large amounts of text
---* has scroll bars - for scrolling through it
---
---Otherwise, the arguments are the same as for [`utils.inputbox`](lua://utils.inputbox).
---@param msg string Message to display.
---@param title? string Title of box. Default: "SmushClient".
---@param default? string Default text. Default: no text.
---@param font? string Font to use in response field. Default: standard font.
---@param fontsize? number Size of font to use.
---@param extras? utils.inputconfig Extra parameters.
---@return string|nil input User input, or nil if the user dismisses the dialog.
function utils.editbox(msg, title, default, font, fontsize, extras) end

---This invokes the standard "font picker" dialog box, which lets you choose a font from those installed on your system.
---
---@param name? string Preselected font.
---@param size? integer Preselected point size.
---@param colour? integer Integer BBGGRR colour code, string hex code, or string colour name for preselected colour.
---@return utils.font|nil font Chosen font details, or nil if the user dismisses the dialog.
function utils.fontpicker(name, size, colour) end

---This converts the supplied hexadecimal string *s* back to a normal string. The converted string may contain binary zeroes.
---@param s string
---@return string s
function utils.fromhex(s) end

---This lets you find the names of all the fonts installed on your current system.
---
---The intention here is for scripts to decide if a font is available, before setting the output or command window to use that font.
---@return string[] fontNames
function utils.getfontfamilies() end

---This lets you test if a particular glyph can be rendered in the specified font. This lets a script writer see if a suitable Unicode font has been installed when using UTF-8 mode.
---
---The intention of this function is to let script (plugin) writers warn users if they are planning to use certain glyphs (for example, mapping symbols) which are not in the currently-selected font.
---@param font string Font name.
---@param glyph integer Unicode code point.
---@return BooleanInt available
function utils.glyph_available(font, glyph) end

---@param fontType? 0|1|2|3
---@return string
---@return integer
function utils.getsystemfont(fontType) end

---This produces a 160-bit hash of the supplied text, returned as a 40-character hex string (40 X 4 bits = 160 bits).
---
---You might use this to store a hash of a long string (eg. a script file) to see if it has changed rather than storing the text itself.
---
---There is no limit to the length of the string that can be hashed.
---@param text string
---@return string hex 40-character hex string.
function utils.hash(text) end

---Returns a table with system information in it.
---@return utils.infotable info
function utils.info() end

---This lets you display a message box and accept a free-format reply. The intention is to allow you to display (in a small dialog box) a question and accept a typed response.
---@param msg string Message to display.
---@param title? string Title of box. Default: "SmushClient".
---@param default? string Default text. Default: no text.
---@param font? string Font to use in response field. Default: standard font.
---@param fontsize? number Size of font to use.
---@param extras? utils.inputconfig Extra parameters.
---@return string|nil input User input, or nil if the user dismisses the dialog.
function utils.inputbox(msg, title, default, font, fontsize, extras) end

---Returns a list with a description corresponding to each [`GetInfo`](lua://GetInfo) selector.
---@return string[] descriptions
function utils.infotypes() end

---This displays a dialog box with a predetermined list of items for the user to choose from, in the form of a single-selection listbox.
---@generic T
---@param msg string Message to display.
---@param title? string Title of box. Default: "SmushClient".
---@param t? { [T]: string|number } Table of key/value pairs. The value is displayed, however the corresponding key is returned. The values are automatically sorted into ascending alphabetic order.
---@param default? T Default (preselected) key, or nil for no selection. If there is no entry associated with that key in *t*, nothing is preselected.
---@return T|nil result The key of the selected item, or nil if cancelled, or nothing selected.
---
---@see utils.choose - Displays a dialog box with a combo box control in it - single selection. Suitable for shorter lists.
---@see utils.multilistbox - Displays a dialog box with a list control in it - multiple selections allowed. This is useful when you want the user to be able to select multiple items.
function utils.listbox(msg, title, t, default) end

---This returns a 128-bit MD5 hash of the string `s`, which may contain binary zeroes. Unlike the [`utils.hash`](lua://utils.hash) function this returns the result as a straight 16-byte (128-bit) field (that is, not converted to printable hex). If you want it in readable form you must then convert it yourself (eg. with [`utils.tohex`](lua://utils.tohex)).
---@param s string Binary string.
---@return string s 16-byte MD5 hash.
function utils.md5(s) end


---Displays a message box. The intention is to allow you to display (in a small dialog box), information of an urgent nature, or ask a yes/no type question.
---@param msg string Message to display.
---@param title? string Title of box. Default: "SmushClient".
---@param type? "ok" The message box contains one push button: OK.
---@param icon? MsgBoxIcon Type of icon. Default: "!".
---@param default? 1 The default button (the one with the focus). Default: OK.
---@return "ok"|nil
function utils.msgbox(msg, title, type, icon, default) end

---Displays a message box. The intention is to allow you to display (in a small dialog box), information of an urgent nature, or ask a yes/no type question.
---@param msg string Message to display.
---@param title? string Title of box. Default: "SmushClient".
---@param type? "abortretryignore" The message box contains three push buttons: Abort, Retry, and Ignore.
---@param icon? MsgBoxIcon Type of icon. Default: "!".
---@param default? 1|2|3 The default button (the one with the focus). Default: the first button (Abort).
---@return "abort"|"retry"|"ignore"|nil
function utils.msgbox(msg, title, type, icon, default) end

---Displays a message box. The intention is to allow you to display (in a small dialog box), information of an urgent nature, or ask a yes/no type question.
---@param msg string Message to display.
---@param title? string Title of box. Default: "SmushClient".
---@param type? "okcancel" The message box contains two push buttons: OK and Cancel.
---@param icon? MsgBoxIcon Type of icon. Default: "!".
---@param default? 1|2 The default button (the one with the focus). Default: the first button (OK).
---@return "ok"|"cancel"|nil
function utils.msgbox(msg, title, type, icon, default) end

---Displays a message box. The intention is to allow you to display (in a small dialog box), information of an urgent nature, or ask a yes/no type question.
---@param msg string Message to display.
---@param title? string Title of box. Default: "SmushClient".
---@param type? "retrycancel" The message box contains two push buttons: Retry and Cancel.
---@param icon? MsgBoxIcon Type of icon. Default: "!".
---@param default? 1|2 The default button (the one with the focus). Default: the first button (Retry).
---@return "retry"|"cancel"|nil
function utils.msgbox(msg, title, type, icon, default) end

---Displays a message box. The intention is to allow you to display (in a small dialog box), information of an urgent nature, or ask a yes/no type question.
---@param msg string Message to display.
---@param title? string Title of box. Default: "SmushClient".
---@param type? "yesno" The message box contains two push buttons: Yes and No.
---@param icon? MsgBoxIcon Type of icon. Default: "!".
---@param default? 1|2 The default button (the one with the focus). Default: the first button (Yes).
---@return "yes"|"no"|nil
function utils.msgbox(msg, title, type, icon, default) end

---Displays a message box. The intention is to allow you to display (in a small dialog box), information of an urgent nature, or ask a yes/no type question.
---@param msg string Message to display.
---@param title? string Title of box. Default: "SmushClient".
---@param type? "yesnocancel" The message box contains three push buttons: Yes, No, and Cancel.
---@param icon? MsgBoxIcon Type of icon. Default: "!".
---@param default? 1|2|3 The default button (the one with the focus). Default: the first button (Yes).
---@return "yes"|"no"|"cancel"|nil
function utils.msgbox(msg, title, type, icon, default) end


---This displays a dialog box with a predetermined list of items for the user to choose from, in the form of a multiple-selection listbox.
---@generic T
---@param msg string Message to display.
---@param title? string Title of box. Default: "SmushClient".
---@param t? { [T]: string|number } Table of key/value pairs. The value is displayed, however the corresponding key is returned. The values are automatically sorted into ascending alphabetic order.
---@param default? T[] Default (preselected) keys, or nil for no selection. Keys that are not associated with an entry in *t* are ignored.
---@return { [T]: true }|nil result A table of chosen items, or nil if the user cancels the dialog. This table might be empty if nothing was chosen.
---
---@see utils.choose - Displays a dialog box with a combo box control in it - single selection. Suitable for shorter lists.
---@see utils.listbox - Displays a dialog box with a list control in it - single selection. Suitable for longer lists.
function utils.multilistbox(msg, title, t, default) end

---This returns a 256-bit SHA hash (Secure Hash Algorithm) of the string *s*, which may contain binary zeroes. Unlike the [`utils.hash`](lua://utils.hash) function this returns the result as a straight 32-byte (256-bit) field (that is, not converted to printable hex). If you want it in readable form you must then convert it yourself (eg. with [`utils.tohex`](lua://utils.tohex)).
---
---This is a more secure hash than the standard [`utils.hash`](lua://utils.hash) algorithm, which returns a 160-bit hash.
---@param s string
---@return string s
function utils.sha256(s) end

---The function utils.split is intended to do the reverse of table.concat. That is, it takes a string and generates a table of entries, delimited by single-character delimiters (such as comma or newline).
---
---Example:
---
---```lua
---test = "the,quick,brown,dog,jumped"
---t = utils.split (test, ",")
---tprint (t)
---print (table.concat (t, ","))
---```
---
---Output:
---
---```text
---1="the"
---2="quick"
---3="brown"
---4="dog"
---5="jumped"
---the,quick,brown,dog,jumped
---```
---
---If the 3rd argument is not supplied, or is zero, then the entire string is split. Otherwise, it will be split the number of times you specify. eg.
---
---```lua
---t = utils.split (test, ",", 2)
---tprint (t)
---```
---
---Output:
---
---```text
---1="the"
---2="quick"
---3="brown,dog,jumped"
---```
---
--- In this case the remaining text is placed in the 3rd table item.
---@param s string The string to be split.
---@param delim string The split delimiter.
---@param count? integer The maximum number of splits to do.
---@return string[] split
function utils.split(s, delim, count) end

---This returns the current output from the high-resolution timer (exactly the same as GetInfo (232) ). However this is easier to remember than GetInfo (232) every time you want to time some code.
---
---If the high-performance timer is not available then the output from the lower-resolution timer is returned instead (seconds since midnight (00:00:00), January 1, 1970).
---
---The intention of this is to allow you to time events, for example:
---
---```lua
---start = utils.timer ()
----- do stuff here
---time_taken = utils.timer () - start
---```
function utils.timer() end

---This converts the string s to hexadecimal (printable) form. The string may contain binary zeroes. Use string.lower to make a lower-case version if that is what you prefer.
---
---@param s string
---@return string s
function utils.tohex(s) end

---This decodes a UTF-8 string into a table of Unicode numbers.
---
---The function returns a (numerically-keyed) table of numbers, one for each Unicode code point found in the UTF-8 string argument. If the string is found to not be valid UTF-8 it returns nil.
---@param s string
---@return integer[] codepoints
function utils.utf8decode(s) end

---This encodes a series of Unicode codes into a UTF-8 string. You can supply any number of arguments, which must be numbers in the range 0 to 2147483647. Any argument can also be a numerically-keyed table, in which case each sequential item (in the range 1 .. n) is also processed. If you supply a table the keys must not have gaps, or any item after the gap will be ignored. By supplying a table you can conveniently supply a large number of codes to be converted.
---
---The function returns a string which is the appropriate UTF-8 encoded string. It raises an error if the arguments cannot be converted to numbers, are out of range, or have decimal places (eg. 5.5).
---@param ... integer
---@return string utf8
function utils.utf8encode(...) end

---Returns a substring of the UTF-8 string, starting at index 'start' and ending at index 'end'. Both may be negative to indicate counting from the right. The end point is optional and defaults to -1, which is the entire rest of the string.
---
---For strings where each character has a character code of < 128, the behaviour is identical to string.sub:
---
---```lua
---utils.utf8sub("ABCDEF", 2, 3)  --> BC
---utils.utf8sub("ABCDEF", 3)     --> CDEF
---utils.utf8sub("ABCDEF", -1)    --> F
---```lua
---
---The above examples are straight "ASCII" strings (ie. each byte is < 128), however the behaviour is different to string.sub with UTF-8 strings. For example:
---
---```lua
---utils.utf8sub (utils.fromhex ("C686C689C69C"), 1, 2)  --> (hex) C686C689
---utils.utf8sub (utils.fromhex ("C686C689C69C"), -1, -1)  --> (hex) C69C
---```
---
---For Unicode characters (ie. whose character code is >= 128 and <= 255) then the function calculates the number of bytes that each "Unicode code point" will require, and count each one as a "column".
---
---The supplied string must be valid UTF-8. If not, the function returns nil followed by the byte position in error.
---@param str string
---@param start integer Start index. May be negative to indicate counting from the right.
---@param ending? integer End point. May be negative to indicate counting from the right. Default: -1, the entire rest of the string.
function utils.utf8(str, start, ending) end

---This checks if a UTF-8 string is valid.
---
---This returns the length of the string (in Unicode characters) if the supplied string is valid UTF-8.
---
---Otherwise it returns nil, and the column of the character in error (1-relative).
---
---eg.
---
---```lua
---print(utils.utf8valid(utils.fromhex("F5838894FCB5898588B4"))) --> 2
---print(utils.utf8valid(utils.fromhex("f0f0f0"))) --> nil 1
---```
---@param s string
---@return integer|nil n Length of the string (in Unicode characters) if the supplied string is valid UTF-8, or nil if it is not.
---@return integer? column If the supplied string is not valid, the column of the character in error (1-relative).
function utils.utf8valid(s) end

---@type string|nil
SMUSHCLIENT_VERSION = SMUSHCLIENT_VERSION
