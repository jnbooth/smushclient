---@meta

---This closes the currently open log file.
---
---If you have a log file postamble declared this is written to the log file (note that this is inconsistent with what OpenLog does).
---@return error_code code #
---`error_code.eOK`: Closed OK.\
---`error_code.eLogFileNotOpen`: Log file was not open.
---
---@see OpenLog - inverse.
function CloseLog() end

---This flushes (writes out) any cached data for the currently open log file.
---@return error_code code #
---`error_code.eOK`: Closed OK.\
---`error_code.eLogFileNotOpen`: Log file was not open.
function FlushLog() end

---Whether to log commands to the log file. Equivalent to [`GetOption("log_input")`](lua://GetOption).
---@return boolean log If `true`, commands are logged to the log file.
---
---@see SetLogInput - setter.
---@see GetLogNotes
---@see GetLogOutput
function GetLogInput() end

---Whether to log notes to the log file. Equivalent to [`GetOption("log_notes")`](lua://GetOption).
---@return boolean log If `true`, notes are logged to the log file.
---
---@see SetLogNotes - setter.
---@see GetLogInput
---@see GetLogOutput
function GetLogNotes() end

---Whether to log output to the log file. Equivalent to [`GetOption("log_output")`](lua://GetOption).
---@return boolean log If `true`, output is logged to the log file.
---
---@see SetLogOutput - setter.
---@see GetLogInput
---@see GetLogNotes
function GetLogOutput() end

---Gets whether the log file is currently open.
---@return boolean open If `true`, the log file is currently open.
function IsLogOpen() end

---This opens a log file of the name *logFileName*, with optional appending to an existing file of the same name. No previous output lines are written (as may happen when you use the ‘open log’ dialog box), nor is a log-file preamble written. If you want to write those things to the script file, you must do them yourself (see [`WriteLog`](lua://WriteLog)).
---@param logFileName? string If nil or empty, the filename specified in the Logging configuration will be used, which lets you use substitutions in the name of the file (eg. the date).
---@param append? boolean If `false`, the contents of an existing file will be erased. Default: `false`.
---@return error_code code #
---`error_code.eOK`: Opened OK.\
---`error_code.eCouldNotOpenLogFile`: Unable to open the log file.\
---`error_code.eLogFileAlreadyOpen`: Log file was already open.
---
---@see CloseLog - inverse.
function OpenLog(logFileName, append) end

---Set whether to log commands to the log file. Equivalent to [`SetOption("log_input", log)`](lua://SetOption).
---@param log? boolean If `true`, commands will be logged to the log file. Default: `true`.
---
---@see GetLogInput - getter.
---@see SetLogNotes
---@see SetLogOutput
function SetLogInput(log) end

---Set whether to log notes to the log file. Equivalent to [`SetOption("log_notes", log)`](lua://SetOption).
---@param log? boolean If `true`, notes will be logged to the log file. Default: `true`.
---
---@see GetLogNotes - getter.
---@see SetLogInput
---@see SetLogOutput
function SetLogNotes(log) end

---Set whether to log output to the log file. Equivalent to [`SetOption("log_output", log)`](lua://SetOption).
---@param log? boolean If *true*, output will be logged to the log file. Default: *true*.
---
---@see GetLogOutput - getter.
---@see SetLogInput
---@see SetLogNotes
function SetLogOutput(log) end

---This writes a message to the log file.
---
---A "newline" is appended to the line if there is not already one there.
---
---The data in the log file may not appear immediately if you view it in another application. To force the data to disk use the [`FlushLog`](lua://FlushLog) function.
---@param ... any Values to concatenate into text.
---@return error_code code #
---`error_code.eOK`: Written OK.\
---`error_code.eLogFileBadWrite`: Unable to write to the log file.\
---`error_code.eLogFileNotOpen`: Log file was not open.
---
---@see FlushLog - flush write buffer to log file.
---@see LogSend - send a message to the MUD and log it in a single function.
function WriteLog(...) end
