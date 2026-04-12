---@meta

---Gets the clipboard contents.
---@return string clipboard Clipboard contents, or an empty string if there is no text on the clipboard.
function GetClipboard() end

---Gets the text currently selected in the output window.
---@return string selection Selection text, or an empty string if there is no selection.
function GetSelection() end

---Gets the last column of the selected text in the output window (starting at 1).
---@return integer column Column number, or zero if there is no selection.
---
---@see SetSelection - setter.
---@see GetSelectionEndLine
---@see GetSelectionStartColumn
---@see GetSelectionStartLine
function GetSelectionEndColumn() end

---Returns a number in the range 1 to the number of lines in the output window, which is the last "selected" line.
---@return integer line Line number, or zero if there is no selection.
---
---@see SetSelection - setter.
---@see GetSelectionEndColumn
---@see GetSelectionStartColumn
---@see GetSelectionStartLine
function GetSelectionEndLine() end

---Gets the first column of the selected text in the output window (starting at 1).
---@return integer column Column number, or zero if there is no selection.
---
---@see SetSelection - setter.
---@see GetSelectionEndColumn
---@see GetSelectionEndLine
---@see GetSelectionStartLine
function GetSelectionStartColumn() end

---Returns a number in the range 1 to the number of lines in the output window, which is the first "selected" line.
---@return integer column Line number, or zero if there is no selection.
---
---@see SetSelection - setter.
---@see GetSelectionEndColumn
---@see GetSelectionEndLine
---@see GetSelectionStartColumn
function GetSelectionStartLine() end

---Sets the clipboard contents.
---@param ... any Values to concatenate into text.
function SetClipboard(...) end

---This sets a selection in the output window, as if you had clicked and dragged with the mouse. To clear an existing selection set the selection to be 1,1,1,1.
---
---The selection column starts at 1. That is, column 1 is the left-most column.
---
---The selection column range is half-open. That is, to select column 1 only you need to select 1,2 as the columns. This makes it possible to have no columns by using 1,1 as the column range.
---
---Similarly, to select a single line, such as line 5, you can select like this:
---
---```lua
---SetSelection(5, 6, 1, 1)
---```
---
---That selects line 5 from column 1, up to line 6 column 1. As the end column is not included, that is therefore all of line 5.
---@param startLine integer
---@param endLine integer
---@param startColumn integer
---@param endColumn integer
---
---@see GetSelectionEndLine
---@see GetSelectionEndColumn
---@see GetSelectionEndLine
---@see GetSelectionStartColumn
function SetSelection(startLine, endLine, startColumn, endColumn) end
