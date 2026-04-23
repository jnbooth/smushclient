---This adds the specified text to the info bar.
---
---The bar is not cleared first, use [`InfoClear`](lua://InfoClear) if you want to start a fresh lot of text.
---
---If you want to change the colour, font or style of the text, use [`InfoFont`](lua://InfoFont) and/or [`InfoColour`](lua://InfoColour) before displaying the text.
---
---The info bar may contain mixed colours, fonts and styles.
---
---The info bar is shared between all worlds. Switching worlds will not change its contents.
---@param ... any Values to concatenate into text.
---
---@see InfoBackground - change info bar background colour.
---@see InfoClear - reset info bar text and colour.
---@see InfoColour - change info bar foreground colour.
---@see InfoFont - change info bar font.
---@see ShowInfoBar - change info bar visibility.
function Info(...) end

---Sets the backkground colour for the info bar.
---@param colour integer|string Integer BBGGRR colour code, string hex code, or string colour name.
---
---@see Info - add text to the info bar.
---@see InfoClear - reset info bar text and colour.
---@see InfoColour - change info bar foreground colour.
---@see InfoFont - change info bar font.
---@see ShowInfoBar - change info bar visibility.
function InfoBackground(colour) end

---Clears the info bar, ready to start adding new text to it.
---
---The font and colour are restored to the defaults as well.
---
---@see Info - add text to the info bar.
---@see InfoBackground - change info bar background colour.
---@see InfoColour - change info bar foreground colour.
---@see InfoFont - change info bar font.
---@see ShowInfoBar - change info bar visibility.
function InfoClear() end

---Sets the text colour for subsequent text written to the info bar with [`Info`](lua://Info).
---@param colour integer|string Integer BBGGRR colour code, string hex code, or string colour name.
---
---@see Info - add text to the info bar.
---@see InfoBackground - change info bar background colour.
---@see InfoClear - reset info bar text and colour.
---@see InfoFont - change info bar font.
---@see ShowInfoBar - change info bar visibility.
function InfoColour(colour) end

---Changes the font, font size, and style for the info bar
---@param fontName? string Font name. If nil or an empty string, font is not changed.
---@param size? number Font size. If nil or 0, font size is not changed.
---@param style? integer OR combination of style bits. Default: 0.
---bold: 1\
---italic: 2\
---underline: 4\
---strikeout: 8
---
---@see Info - add text to the info bar.
---@see InfoBackground - change info bar background colour.
---@see InfoClear - reset info bar text and colour.
---@see InfoColour - change info bar foreground colour.
---@see ShowInfoBar - change info bar visibility.
function InfoFont(title, fontName, size, style) end

---This resets the "connect time" to now, so that the time shown on the status bar counts up from the moment you called this function.
---
---@see GetConnectDuration
---@see SetStatus
function ResetStatusTime() end

---This sets the status line (at the bottom of the screen) to the nominated text. It is useful for keeping track of things. The status line is separately maintained for each world.
---@param ... any Values to concatenate into text.
---@see Info
---@see ResetStatusTime
function SetStatus(...) end

---Use this to show or hide the "info" toolbar (the one above the status line).
---@param visible? boolean Show the toolbar. Default: true.
---
---@see Info - add text to the info bar.
---@see InfoBackground - change info bar background colour.
---@see InfoClear - reset info bar text and colour.
---@see InfoColour - change info bar foreground colour.
---@see InfoFont - change info bar font.
function ShowInfoBar(visible) end
