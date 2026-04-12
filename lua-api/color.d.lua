---@meta

---@alias AnsiColourIndex
---| 1 # Black
---| 2 # Red
---| 3 # Green
---| 4 # Yellow
---| 5 # Blue
---| 6 # Magenta
---| 7 # Cyan
---| 8 # White

---This lets you play with colours in a script. You pass to it a colour and an adjustment method.
---@param colour integer|string Integer BBGGRR colour code, string hex code, or string colour name.
---@param method
---| 1 # Invert
---| 2 # Lighter
---| 3 # Darker
---| 4 # Less colour
---| 5 # More colour
---@return integer rgb The BBGGRR colour code of the adjusted colour. If the method is out of range the original colour is returned.
function AdjustColour(colour, method) end

---Converts a named colour (eg. "brown") to an RGB colour code (eg. 2763429).
---
---The colours can either be named (eg. "red", "blue", "green", "brown") or supplied as HTML colour codes (eg. "#010203"). For HTML colours, the leading "#" is required.
---@param name string
---@return integer rgb The BBGGRR colour code of the converted colour, or -1 if the colour name is not recognised. Note that -1 cannot be an RGB colour (-1 is the hex sequence 0xFFFFFFFF, whereas the "highest" colour (white) is hex 0x00FFFFFF).
---
---@see RGBColourToName - inverse.
function ColourNameToRGB(name) end

---Gets one of the colours used for the 8 ANSI bold colours.
---@param whichColour AnsiColourIndex
---@return integer rgb BBGGRR colour code.
---
---@see SetBoldColour - setter.
---@see GetNormalColour
---@see SetNormalColour
function GetBoldColour(whichColour) end

---Returns the RGB code for how a particular colour is going to be displayed.
---@param which integer|string Integer BBGGRR colour code, string hex code, or string colour name.
---@return integer rgb The BBGGRR colour code corresponding to the colour in the colour mapping table. If the colour is not present, the supplied argument will be returned.
---
---@see MapColour - setter.
---@see MapColourList - get all mapped colours.
function GetMapColour(which) end

---Gets one of the colours used for the 8 ANSI normal colours.
---@param whichColour AnsiColourIndex
---@return integer rgb BBGGRR colour code.
---
---@see SetNormalColour - setter.
---@see GetBoldColour
---@see SetBoldColour
function GetNormalColour(whichColour) end

---Get the background colour used in Note and Tell.
---@return integer rgb BBGGRR colour code.
---
---@see SetNoteColourBack - setter.
---@see NoteColourRGB - sets both foreground and background.
---@see GetNoteColourFore
---@see SetNoteColourFore
function GetNoteColourBack() end

---Get the foreground colour used in Note and Tell.
---@return integer rgb BBGGRR colour code.
---
---@see SetNoteColourFore - setter.
---@see NoteColourRGB - sets both foreground and background.
---@see GetNoteColourBack
---@see SetNoteColourBack
function GetNoteColourFore() end

---Gets the colour used by various items as currently configured in the system color theme, based on the selector (*index*).
---@param index
---| 0 # Scrollbar
---| 1 # Background
---| 2 # Active caption
---| 3 # Inactive caption
---| 4 # Menu
---| 5 # Window
---| 6 # Window frame
---| 7 # Menu text
---| 8 # Window text
---| 9 # Caption text
---| 10 # Active border
---| 11 # Inactive border
---| 12 # Application workspace
---| 13 # Highlight
---| 14 # Highlight text
---| 15 # Button face
---| 16 # Button shadow
---| 17 # Gray text
---| 18 # Button text
---| 19 # Inactive caption text
---| 20 # Button highlight
---| 21 # 3D dark shadow
---| 22 # 3D light
---| 23 # Info text
---| 24 # Info bk
---@return integer rgb The BBGGRR code corresponding to the selected colour index. If the index is out of range, 0 (black) is returned.
function GetSysColor(index) end

---You can specify a "map" of an original colour to the colour you want displayed.
---
---The intention here is to let you override the colours sent down by MUDs, particularly using MXP colours, which are not subject to the normal ANSI colour palette.
---
---For example, if a MUD annoys you by using a lot of red letters, you could do this:
---
---```lua
---MapColour(ColourNameToRGB ("red"), ColourNameToRGB ("green"))
---```
---
---You can map any number of colours, within reason. Potentially there are 16,777,216 different colours (2 to the power 24) however in practice a particular MUD is likely to only use a handful.
---
---The colour must match *exactly* or the translation will not occur.
---
---The arguments to the function are RGB colours (that is, numbers in the range 0 to 16,777,215) however you can use the function [`ColourNameToRGB`](lua://ColourNameToRGB) as shown above if you want to use colour names.
---
---The colour number is really made up of three bytes:
---
---1. Red (low-order byte), ie. 0 to 255
---2. Green (middle byte), ie. 0 to 255 multiplied by 256
---3. Blue (higher-order byte), ie. 0 to 255 multiplied by 65536
---
---To find the colour of a particular piece of text in the output window (so you can know the "from" colour) you can select it (eg. double-click) and then use the context menu -> Text Attributes.
---
---This will show you the Text and Background RGB colours (in HTML format). You can use those colours in the function [`ColourNameToRGB`](lua://ColourNameToRGB) to translate them to numbers. eg.
---
---```lua
---local x = ColourNameToRGB ("#FF00FF")
---```
---@param original integer|string Integer BBGGRR colour code, string hex code, or string colour name.
---@param replacement integer|string Integer BBGGRR colour code, string hex code, or string colour name.
---
---@see GetMapColour - getter.
---@see MapColourList - get all mapped colours.
function MapColour(original, replacement) end

---Gets a table of all the colours that have been mapped for this world.
---
---Note that this will return only colours in the table.
---@return { [integer]: integer } colours Table where the key is the original colour BBGGRR and the value is the replacement colour BBGGRR.
---
---@see MapColour - setter.
---@see GetMapColour - get a single mapped colour.
function MapColourList() end

---This is an alias for [`NoteColourRGB`](lua://NoteColourRGB), for compatibility with MUSHclient.
---@param foreground? integer|string Integer BBGGRR colour code, string hex code, or string colour name.
---@param background? integer|string Integer BBGGRR colour code, string hex code, or string colour name.
function NoteColourName(foreground, background) end

---You can use this to set the foreground (text) and background colours used in [`Note`](lua://Note) and [`Tell`](lua://Tell).
---
---If a particular colour is nil, then that colour will be unchanged. Thus you can use this function to change the foreground colour only, the background colour only, or both.
---@param foreground? integer|string Integer BBGGRR colour code, string hex code, or string colour name. If nil, foreground is unchanged.
---@param background? integer|string Integer BBGGRR colour code, string hex code, or string colour name. If nil, background is unchanged.
---
---@see GetNoteColourFore
---@see SetNoteColourFore
---@see GetNoteColourBack
---@see SetNoteColourBack
function NoteColourRGB(foreground, background) end

---This lets you call the colour picker from within a script. Note that doing this will pause script execution while the dialog is active, and is not recommended for scripts that should run to completion quickly.
---@param suggested? integer|string Integer BBGGRR colour code, string hex code, or string colour name for starting colour code suggestion.
---@return integer rgb Chosen BBGGRR colour code, or -1 if the dialog was cancelled.
function PickColour(suggested) end

---This changes the default background colour of the output window. This is intended to be used to make "mood" changes to the output window. For example, if you are currently playing in a watery zone, the background colour could change from black to a deep blue.
---
---You simply specify the RGB code for the desired colour, or the constant 0xFFFFFFFF to indicate to use the default behaviour. If you have set a colour, then any time the background colour for text would have been "normal black", then the background is not drawn and thus the background colour you chose is allowed to stay.
---@param colour integer|string Integer BBGGRR colour code, string hex code, or string colour name.
---@return integer rgb Previous background BBGGRR colour code.
function SetBackgroundColour(colour) end

---Sets one of the colours used for the 8 ANSI bold colours.
---@param whichColour AnsiColourIndex
---@param newValue integer|string Integer BBGGRR colour code, string hex code, or string colour name.
---
---@see GetBoldColour - getter.
---@see GetNormalColour
---@see SetNormalColour
function SetBoldColour(whichColour, newValue) end

---Gets one of the colours used for the 8 ANSI normal colours.
---@param whichColour AnsiColourIndex
---@param newValue integer|string Integer BBGGRR colour code, string hex code, or string colour name.
---
---@see GetNormalColour -- getter.
---@see GetBoldColour
---@see SetBoldColour
function SetNormalColour(whichColour, newValue) end

---Get the background colour used in [`Note`](lua://Note) and [`Tell`](lua://Tell).
---@param colour integer|string Integer BBGGRR colour code, string hex code, or string colour name.
---
---@see GetNoteColourBack - getter.
---@see GetNoteColourFore
---@see SetNoteColourFore
function SetNoteColourBack(colour) end

---Set the foreground colour used in [`Note`](lua://Note) and [`Tell`](lua://Tell).
---@param colour integer|string Integer BBGGRR colour code, string hex code, or string colour name.
---
---@see GetNoteColourFore - getter.
---@see GetNoteColourBack
---@see SetNoteColourBack
function SetNoteColourFore(colour) end

---Converts an RGB colour code (eg. 255) to its HTML name (eg. "red").
---
---If the name is not in the list of recognised names, the colour is represented as a hex HTML colour (eg. "#010203").
---@param colour integer|string Integer BBGGRR colour code, string hex code, or string colour name.
---@return string name Colour name or hex code.
---
---@see ColourNameToRGB - inverse.
function RGBColourToName(colour) end
