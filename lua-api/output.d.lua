---@meta

---Activates the main (first) window for this world.
---
---@see ActivateNotepad - activate a notepad window.
function Activate() end

---This adds a font (specified by pathname) into the application space. It can then be used by world windows for output or command window fonts.
---
---Currently only TrueType fonts, TrueType font collections, and OpenType fonts are supported.
---@param pathName string Path to font file.
---
---@see SetInputFont - set font of command input.
---@see SetOutputFont - set font of MUD output.
---@see WindowFont - set font of miniwindow.
function AddFont(pathName) end

---Clears 'Count' lines from the output window, starting at the end (most recently-displayed).
---
---This allows you to script the omission of recently-received lines (for example, in a multi-line trigger).
---
---All line types are omitted (whether received from the MUD, command input, or Note lines). A line is counted as a physical line on the screen, it does not necessarily end in a newline character.
---@param count integer
---
---@see GetLineCount
---@see GetLinesInBufferCount
function DeleteLines(count) end

---Clears all output from the output window. There is no confirmation dialog.
function DeleteOutput() end

---This encodes text so that it can be logged as HTML.
---
---It will encode `"<"`, `">"` and `"&"` to the HTML equivalents (eg. `&lt;` ), otherwise a line like this: `"go <north>"` would not log correctly as HTML.
---@param stringToConvert string Unencoded text.
---@return string encoded Encoded text with the following characters replaced with the HTML "escaped" equivalents:
---`<` -> `&lt;`\
---`>` -> `&gt;`\
---`&` -> `&amp;`\
---`"` -> `&quot;`
function FixupHTML(stringToConvert) end

---Whether command input is echoed to the output window.
---@return boolean echo If `true`, command input is echoed to the output window.
---
---@see SetEchoInput - setter.
function GetEchoInput() end

---This returns the number of lines received by this world. However, see [`GetLinesInBufferCount`](lua://GetLinesInBufferCount) for the number of lines currently in the output buffer, which might be different.
---@return integer lines The number of lines received by this world.
function GetLineCount() end

---This returns the number of lines in the output buffer (window) of this world.
---
---It will differ from [`GetLineCount`](lua://GetLineCount) once the number of lines received exceeds the size of the buffer (as early ones will be discarded).
---@return integer lines Line count.
function GetLinesInBufferCount() end

---This function returns the geometry of the main MUSHclient window (the frame window inside which all world windows reside), in pixels.
---@return Rectangle geometry
---
---@see GetWorldWindowPosition
function GetMainWindowPosition() end

---This function returns the geometry of the current world's window, in pixels.
---@return Rectangle geometry
---
---@see GetMainWindowPosition
function GetWorldWindowPosition() end

---This returns a block of text consisting of the nominated number of recently-arrived lines from the MUD.
---
---It excludes notes or commands you type.
---
---It includes lines even if they have been omitted from output by a trigger.
---
---A "line" is a string of text from the MUD followed by a newline character. This is different from the lines returned by GetLineInfo which are physical lines which appear in the output window. Thus a line here might be shown as 5 lines on the screen if they wrapped around to fit them in.
---
---The maximum number of lines stored is configured in the Input settings.
---
---The number you nominate is counted from the bottom, so if you ask for 1 line it will be the most recent, 2 lines will be the most recent 2 lines and so on.
---@param count integer
---@return string lines A string containing the nominated number of lines of recent MUD output, separated by the newline character (hex 0A).
function GetRecentLines(count) end

---Opens the URL you supply using the default web browser.
---@param url string
---@return error_code code #
---`error_code.eBadParameter`: The URL could not be parsed.\
---`error_code.eCouldNotOpenFile`: An error code was returned when attempting to open the web browser.\
---`error_code.eOK`: Successfully opened.
function OpenBrowser(url) end

---Turns on or off the "paused" flag for all output windows in this world.
---@param flag? boolean Default: `true`.
function Pause(flag) end

---This closes any outstanding MXP and resets the output window colours to ANSI white on black. It also cancels underline, highlight, and inverse.
function Reset() end

---This sets a background image for output window. The text in the output window is drawn on top of this. If the image does not completely fill the window, the background colour is visible beneath it.
---@param fileName string|nil Disk file to load the image from. The file name can be nil or an empty string, in which case any existing image will be removed.
---@param mode miniwin.pos See [`miniwin.pos`](lua://miniwin.pos).
---@return error_code code #
---`error_code.eBadParameter`: Mode out of range above, or file name less than 5 characters, or file type is not recognized.\
---`error_code.eFileNotFound`: File not found.\
---`error_code.eCouldNotOpenFile`: File could not be loaded (perhaps corrupt).\
---`error_code.eOK`: Success.
---
---@see SetForegroundImage
function SetBackgroundImage(fileName, mode) end

---Changes the shape of the mouse cursor.
---@param cursor miniwin.cursor See [`miniwin.cursor`](lua://miniwin.cursor).
---@return error_code code #
---`error_code.eBadParameter`: Invalid cursor value.\
---`error_code.eOK`: Completed OK.
function SetCursor(cursor) end

---Sets whether command input is echoed to the output window.
---@param echo boolean If `true`, command input will be echoed to the output window.
---
---@see GetEchoInput - getter.
function SetEchoInput(echo) end

---This sets a foreground image for output window. The text in the output window is drawn under this.
---
---With "stretching" or "tiling" modes, the text from the MUD will not be visible.
---@param fileName string|nil Disk file to load the image from. The file name can be nil or an empty string, in which case any existing image will be removed.
---@param mode miniwin.pos See [`miniwin.pos`](lua://miniwin.pos).
---@return error_code code #
---`error_code.eBadParameter`: Mode out of range above, or file name less than 5 characters, or file type is not recognized.\
---`error_code.eFileNotFound`: File not found.\
---`error_code.eCouldNotOpenFile`: File could not be loaded (perhaps corrupt).\
---`error_code.eOK`: Success.
function SetForegroundImage(fileName, mode) end

---Sets text on the status bar at the bottom of the screen.
---@param message string Status text.
function SetStatus(message) end

---This sets the window title (for the whole client window) to the nominated text. You could use this to make the tile reflect the character name, world name, or whatever you want.
---
---Setting the title to an empty string (ie. "") will revert back to the default title line (the world name, plus connection status).
---@param ... any Values to concatenate into text.
function SetMainTitle(...) end

---Use this to change the output font. The *entire* window font changes, you cannot change individual lines or words.
---@param fontName string Font family name.
---@param pointSize number Font point size.
---
---@see AddFont - import a font file.
---@see SetInputFont - set font of command input.
---@see WindowFont - set font of miniwindow.
function SetOutputFont(fontName, pointSize) end

---This lets you do two things:
---
---1. Hide the existing scroll bar if desired
---2. Set the "scroll position" in the output window
---
---The first feature (hiding the scroll bar) lets you, well, hide it, so that you can draw your own wherever you want (eg. with a themed miniwindow somewhere near where the text appears).
---
---The second feature lets your code scroll the text around (so you can still use the existing output buffer) in response to the player clicking and dragging on "your" thumb, or arrows.
---
---You can use [`GetInfo(296)`](lua://GetInfo) to find the current scroll-bar position (and thus add or subtract from it, to scroll up or down small amounts (like a line or a page).
---
---You can use [`GetInfo(120)`](lua://GetInfo) to find if the scroll-bar is currently hidden or not.
---
---The existing code adds the font height ([`GetInfo(212)`](lua://GetInfo)) to the current scroll position to scroll downwards a line, and subtracts it to scroll up a line.
---
---To scroll up or down pages you need to find the size of the output window text rectangle, and add or subtract that.
---@param position integer Set scroll position. -1 means "the end of the window" (which is the number of lines multiplied by the size of the font, less the size of the output window). -2 means "no chnge".
---@param visible? boolean Show the scroll bar. Default: `true`.
---@return error_code code #
---`error_code.eOK`: Success.
function SetScroll(position, visible) end

---Changes the status of the world window.
---@param parameter
---| 1 # Activates the window and displays it as a maximized window.
---| 2 # Minimizes the window and activates the top-level window in the system’s list.
---| 3 # Activates and displays the window. If the window is minimized or maximized, restores it to its original size and position.
---| 4 # Activates and displays the window. If the window is minimized or maximized, restores it to its original size and position.
function SetWorldWindowStatus(parameter) end

---The argument to this function is supplied to MUSHclient as if it had been sent from the MUD. This is intended for debugging triggers etc, similar to the Game -> Test Trigger menu option.
---@param ... any Values to concatenate into text.
function Simulate(...) end

---This reduces the rectangle in which normal MUD output is displayed, from the entire output window, to the subset specified.
---
---This can be used to make a "desktop effect" where you have room on all sides for miniwindows to be placed.
---
---A border can be drawn at the edge of the rectangle to make it obvious where the text ends, to help in resizing the window.
---
---The text is clipped to be within the text rectangle. If the text rectangle is 0, 0, 0, 0, text is drawn "normally" to the borders of the output window.
---@param left integer Left pixel coordinate of the output rectangle.
---@param top integer Top pixel coordinate of the output rectangle.
---@param right integer Right pixel coordinate of the output rectangle.
---@param bottom integer Bottom pixel coordinate of the output rectangle.
---@param borderOffset integer The offset in pixels between the edge of the text, and the border. You can specify a few pixels here so that the text is not jammed up against the border.
---@param borderColour integer|string Integer BBGGRR colour code, string hex code, or string colour name of colour to draw the border in.
---@param borderWidth integer Width of the border (0 for no border).
---@param outsideFillColour integer|string Integer BBGGRR colour code, string hex code, or string colour name of brush colour to fill the area outside the text rectangle.
---@param outsideFillStyle miniwin.brush The brush mode to use for the outside fill. See [`miniwin.brush`](lua://miniwin.brush).
---@return error_code code #
---`error_code.eBrushStyleNotValid`: Invalid brush style specified for *outsideFillStyle*.\
---`error_code.eOK`: Done OK.
function TextRectangle(left, top, right, bottom, borderOffset, borderColour, borderWidth, outsideFillColour, outsideFillStyle) end
