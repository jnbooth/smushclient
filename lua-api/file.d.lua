---@meta

---This lets you change the current working directory used by SmushClient. This is useful for situations (like plugins) where you want to load files relative to the plugin location but are unsure if the working directory is at the plugin location. eg.
---
---```lua
---ChangeDir(GetInfo(66)) -- ensure directory is SmushClient.exe directory
---```
---
---Note that some actions (like opening world files) may change the directory (eg. to the world file directory) as a side-effect.
---@param path string
---@return boolean succeeded
function ChangeDir(path) end

---Closes an SQLite database.
---@param databaseId string Unique database ID assigned during `DatabaseOpen`.
---@return integer code #
---0: Closed OK\
---1: Database ID not found\
---2: Database not open
---
---@see DatabaseOpen - inverse.
---@see sqlite3.db.close - recommended alternative.
function DatabaseClose(databaseId) end

---Opens or creates a database. Databases can be held in memory or on disk. You need read or write permissions to access the database, as it is stored on disk as a normal disk file. **Use [`sqlite3.open`](lua://sqlite3.open) or [`sqlite3.open_memory`](lua://sqlite3.open_memory) instead.**
---@param databaseId string A unique ID you choose to identify this database for all other operations on it.
---@param filename string The file name on disk. The special filename ":memory:" opens an in-memory database. This can be used to manipulate data (or play with SQL) without actually writing to disk.
---@param flags integer SQLite opening flags.
---
---Use [`DatabaseClose`](lua://DatabaseClose) to close the database when you are finished with it.
---
---If the database is still open when the world file is closed, the database is automatically closed.
---
---It is not an error to re-open the same database ID of an existing, open, database, provided the disk file name is the same. This will be treated as a no-operation, so that triggers and aliases can open the database without having to check first if it was already open.
---@return integer code The return codes are available in the [`sqlite3`](lua://sqlite3) table in Lua.
---
---@see DatabaseClose - inverse.
---@see sqlite3.open - recommended alternative.
---@see sqlite3.open_memory - recommended alternative.
function DatabaseOpen(databaseId, filename, flags) end

---This lets you export one trigger, timer, alias, macro, keypad item or variable in XML format. This is symmetrical with the [`ImportXML`](lua://ImportXML) function - a trigger exported with `ExportXML` will be syntactically correct for importing with `ImportXML`. However, it may not actually import if, say, there is a clash where two triggers both have the same label.
---
---This would let you send triggers etc. from one world to another in an automated way, or from one client to another if you built some chat scripting around these functions.
---@param type
---| 0 # Trigger
---| 1 # Alias
---| 2 # Timer
---| 3 # Macro
---| 4 # Variable
---| 5 # Keypad
---@param name string
---@return string xml The XML text of the nominated item, or an empty string if *type* is not valid or the item cannot be found.
function ExportXML(type, name) end

---This duplicates the functionality in the Edit menu -> Paste XML Settings.
---
---It allows you to import one or more triggers, aliases, macros, timers, variables, colours, numeric keypad configuration items.
---
---The two things you cannot import here are general world configuration (use SetOption and SetAlphaOption for that), and plugins.
---
---The XML text should start with one of the following, or -1 will be returned:
---
---```xml
---<?xml
---<!--
---<!DOCTYPE
---<muclient
---<world
---<triggers
---<aliases
---<timers
---<macros
---<variables
---<colours
---<keypad
---<printing
---<comment
---<include
---<plugin
---<script
---```
---
---In other words, to import a trigger you should start with <triggers>. You can import more than one type of thing, eg.
---
---```xml
---<triggers> ... </triggers> <aliases> ... </aliases>
---```
---
---The syntax is exactly the same as used in the world file, or if you copy a trigger/alias/timers/variable to the clipboard from the world configuration screens.
---
---If there is a parsing error (eg. bad XML format, like unterminated element, or unknown option for an item) then a text window will open with the error description shown.
---
---Note that duplicate items are replaced by `ImportXML` - so if you already have a trigger / alias / timer of the same name, it will be replaced.
---@param xml string
---@return integer imported If sucessful, the number of items imported, which might be zero if the syntax was OK, but nothing was actually added, eg. "<triggers></triggers>" would parse OK but not add anything.
---
---If unsuccessful, returns a negative number as follows:\
----1: Invalid text.
----2: Parsing error (eg. bad XML format, like unterminated element, or unknown option for an item).
function ImportXML(xml) end

---Saves the current world, under the specified filename.
---@param name? string Savefile name. Supply a nil (or blank) file name to save under the current name.
---@param saveAs? boolean Save under a new name, but preserve the original name.
---@return boolean failed `true` if failed, `false` if succeeded.
---
---@see SaveState - save plugin state.
---@see SetChanged - mark world as needing a save.
function Save(name, saveAs) end

---Use this function to set or clear the "world has changed" flag. If this flag is set, you will be asked to save it. If clear, you will not be asked to save it.
---@param changedFlag boolean
---
---@see Save - save world.
function SetChanged(changedFlag) end
