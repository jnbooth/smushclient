---@meta

---LuaSQLite3: SQLite3 bindings for Lua.
---
---From https://lua.sqlite.org/home/doc/tip/doc/lsqlite3.wiki.
sqlite3 = {
  OK = 0,          ---Successful result
  ERROR = 1,       ---Generic error
  INTERNAL = 2,    ---Internal logic error in SQLite
  PERM = 3,        ---Access permission denied
  ABORT = 4,       ---Callback routine requested an abort
  BUSY = 5,        ---The database file is locked
  LOCKED = 6,      ---A table in the database is locked
  NOMEM = 7,       ---A `malloc()` failed
  READONLY = 8,    ---Attempt to write a readonly database
  INTERRUPT = 9,   ---Operation terminated by `sqlite3_interrupt()`
  IOERR = 10,      ---Some kind of disk I/O error occurred
  CORRUPT = 11,    ---The database disk image is malformed
  NOTFOUND = 12,   ---Unknown opcode in `sqlite_file_control()`
  FULL = 13,       ---Insertion failed because database is full
  CANTOPEN = 14,   ---Unable to open the database file
  PROTOCOL = 15,   ---Database lock protocol error
  EMPTY = 16,      ---Internal use only
  SCHEMA = 17,     ---The database schema changed
  TOOBIG = 18,     ---String or BLOB exceeds size limit
  CONSTRAINT = 19, ---Abort due to constraint violation
  MISMATCH = 20,   ---Data type mismatch
  MISUSE = 21,     ---Library used incorrectly
  NOLFS = 22,      ---Uses OS features not supported on host
  FORMAT = 24,     ---Not used
  RANGE = 25,      ---2nd parameter to sqlite3_bind out of range
  NOTADB = 26,     ---File opened that is not a database file
  ROW = 100,       ---`sqlite3_step()` has another row ready
  DONE = 101,      ---`sqlite3_step()` has finished executing
  CREATE_INDEX = 1,
  CREATE_TABLE = 2,
  CREATE_TEMP_INDEX = 3,
  CREATE_TEMP_TABLE = 4,
  CREATE_TEMP_TRIGGER = 5,
  CREATE_TEMP_VIEW = 6,
  CREATE_TRIGGER = 7,
  CREATE_VIEW = 8,
  DELETE = 9,
  DROP_INDEX = 10,
  DROP_TABLE = 11,
  DROP_TEMP_INDEX = 12,
  DROP_TEMP_TABLE = 13,
  DROP_TEMP_TRIGGER = 14,
  DROP_TEMP_VIEW = 15,
  DROP_TRIGGER = 16,
  DROP_VIEW = 17,
  INSERT = 18,
  PRAGMA = 19,
  READ = 20,
  SELECT = 21,
  TRANSACTION = 22,
  UPDATE = 23,
  ATTACH = 24,
  DETACH = 25,
  ALTER_TABLE = 26,
  REINDEX = 27,
  ANALYZE = 28,
  CREATE_VTABLE = 29,
  DROP_VTABLE = 30,
  FUNCTION = 31,
  SAVEPOINT = 32,
  OPEN_READONLY = 0x00000001,
  OPEN_READWRITE = 0x00000002,
  OPEN_CREATE = 0x00000004,
  OPEN_URI = 0x00000040,
  OPEN_MEMORY = 0x00000080,
  OPEN_NOMUTEX = 0x00008000,
  OPEN_FULLMUTEX = 0x00010000,
  OPEN_SHAREDCACHE = 0x00020000,
  OPEN_PRIVATECACHE = 0x00040000
}

---Value type of a database column.
---@alias sqlite3.value string|integer|number|boolean|nil

---A callback context is available as a parameter inside the callback functions [`sqlite3.db:create_aggregate()`](lua://sqlite3.db.create_aggregate) and [`sqlite3.db:create_function()`](lua://sqlite3.db.create_function). It can be used to get further information about the state of a query.
---@class sqlite3.context<T>
local sqlite3_context = {}

---Returns the number of calls to the aggregate step function.
---@return integer calls
function sqlite3_context:aggregate_count() end

---Returns the user-definable data field for callback functions.
---@return unknown udata
---
---@see sqlite3.context.set_aggregate_data - setter.
function sqlite3_context:get_aggregate_data() end

---Sets the user-definable data field for callback functions.
---@param udata any
---
---@see sqlite3.context.get_aggregate_data - getter.
function sqlite3_context:set_aggregate_data(udata) end

---This function sets the result of a callback function to *res*. The type of the result depends on the type of *res* and is either a number or a string or nil. All other values will raise an error message.
---@param res number|string|nil
---
---@see sqlite3.context.result_blob - for setting the result to a binary blob rather than text.
---@see sqlite3.context.result_error - for setting the result to an error code rather than an integer.
function sqlite3_context:result(res) end

---This function sets the result of a callback function to nil.
---
---@see sqlite3.context.result - generic version.
function sqlite3_context:result_null() end

---This function sets the result of a callback function to the value *number*.
---@param number number
---
---@see sqlite3.context.result - generic version.
function sqlite3_context:result_number(number) end

---Alias for [`sqlite3.context:result_number()`](lua://sqlite3.context.result_number).
---@param number number
function sqlite3_context:result_double(number) end

---This function sets the result of a callback function to the integer value in *number*.
---@param number integer
---
---@see sqlite3.context.result - generic version.
---@see sqlite3.context.result_error - for setting the result to an error code rather than an integer.
function sqlite3_context:result_int(number) end

---This function sets the result of a callback function to the string in *str*.
---@param str string
---
---@see sqlite3.context.result - generic version.
---@see sqlite3.context.result_blob - for setting the result to a binary blob rather than text.
function sqlite3_context:result_text(str) end

---This function sets the result of a callback function to the binary blob in *blob*.
---@param blob string
---
---@see sqlite3.context.result - generic version.
function sqlite3_context:result_blob(blob) end

---This function sets the result of a callback function to the error value in *err*.
---@param err integer
---
---@see sqlite3.context.result - generic version.
function sqlite3_context:result_error(err) end

---Returns the userdata parameter given in the call to install the callback function (see [`sqlite3.db:create_aggregate()`](lua://sqlite3.db.create_aggregate) and [`sqlite3.db:create_function()`](lua://sqlite3.db.create_function) for details).
---@return T udata
function sqlite3_context:user_data() end

---After creating a prepared statement with [`sqlite3.db:prepare()`](lua://sqlite3.db.prepare) the returned statement object should be used for all further calls in connection with that statement.
---@class sqlite3.stmt
local sqlite3_stmt = {}

---Binds value to statement parameter *n*. If the type of value is string it is bound as text. If the type of value is number, it is bound as an integer or double depending on its subtype using `lua_isinteger`. If value is a boolean then it is bound as 0 for `false` or 1 for `true`. If value is nil or missing, any previous binding is removed.
---@param n integer Parameter index.
---@param value? sqlite3.value Value to bind.
---@return integer code `sqlite3.OK` on success or else a numerical error code.
---
---@see sqlite3.stmt.bind_blob - for binding a string value as a binary blob rather than text.
---@see sqlite3.stmt.bind_names - for binding named parameters.
---@see sqlite3.stmt.bind_values - for binding multiple positional parameters.
function sqlite3_stmt:bind(n, value) end

---Binds string *blob* (which can be a binary string) as a blob to statement parameter *n*.
---@param n integer Parameter index.
---@param blob string Binary string.
---@return integer code `sqlite3.OK` on success or else a numerical error code.
---
---@see sqlite3.stmt.bind - generic version.
function sqlite3_stmt:bind_blob(n, blob) end

---Binds the values in *nametable* to statement parameters. If the statement parameters are named (i.e., of the form ":AAA" or "$AAA") then this function looks for appropriately named fields in *nametable*; if the statement parameters are not named, it looks for numerical fields 1 to the number of statement parameters.
---@param nametable { [string]: sqlite3.value } Keys are parameter names; values are values to bind.
---@return integer code `sqlite3.OK` on success or else a numerical error code.
---
---@see sqlite3.stmt.bind - for binding a positional parameter.
---@see sqlite3.stmt.bind_values - for binding multiple positional parameters.
function sqlite3_stmt:bind_names(nametable) end

---Returns the largest statement parameter index in the prepared statement. When the statement parameters are of the forms ":AAA" or "?", then they are assigned sequentially increasing numbers beginning with one, so the value returned is the number of parameters. However if the same statement parameter name is used multiple times, each occurrence is given the same number, so the value returned is the number of unique statement parameter names.
---
---If statement parameters of the form "?NNN" are used (where NNN is an integer) then there might be gaps in the numbering and the value returned by this interface is the index of the statement parameter with the largest index value.
---@return integer index
function sqlite3_stmt:bind_parameter_count() end

---Returns the name of the n<sup>th</sup> parameter in the prepared statement. Statement parameters of the form ":AAA" or "@AAA" or "$VVV" have a name which is the string ":AAA" or "@AAA" or "$VVV". In other words, the initial ":" or "$" or "@" is included as part of the name. Parameters of the form "?" or "?NNN" have no name. The first bound parameter has an index of 1. If the value *n* is out of range or if the n-th parameter is nameless, then nil is returned.
---@param n integer Parameter index.
---@return string|nil name Parameter name.
function sqlite3_stmt:bind_parameter_name(n) end

---Binds the given values to statement parameters.
---@param ... sqlite3.value
---@return integer code `sqlite3.OK` on success or else a numerical error code.
---
---@see sqlite3.stmt.bind - for binding a single positional parameter.
---@see sqlite3.stmt.bind_values - for binding positional parameters.
function sqlite3_stmt:bind_values(...) end

---Returns the number of columns in the result set returned by statement *stmt* or 0 if the statement does not return data (for example an `UPDATE`).
---@return integer columns
function sqlite3_stmt:columns() end

---This function frees prepared statement *stmt*.
---@return integer code If the statement was executed successfully, or not executed at all, then `sqlite3.OK` is returned. If execution of the statement failed then an error code is returned.
---
---@see sqlite3.db.close_vm - finalize all statements.
function sqlite3_stmt:finalize() end

---Returns the name of column *n* in the result set of statement *stmt*.
---@param n integer Column number. The left-most column is number 0.
---@return string name
---
---@see sqlite3.stmt.get_type
function sqlite3_stmt:get_name(n) end

---Returns a table with the names and types of all columns in the result set of statement *stmt*.
---@return { [string]: string } types
---
---@see sqlite3.stmt.get_named_values
function sqlite3_stmt:get_named_types() end

---Returns a table with names and values of all columns in the current result row of a query.
---@return { [string]: sqlite3.value } values
---
---@see sqlite3.stmt.get_named_types
---@see sqlite3.stmt.nrows - iterator.
function sqlite3_stmt:get_named_values() end

---This function returns an array with the names of all columns in the result set returned by statement *stmt*.
---@return string[] names
---
---@see sqlite3.stmt.get_unames -- unpacked version.
function sqlite3_stmt:get_names() end

---Returns the type of column *n* in the result set of statement *stmt*.
---@param n integer Column number. The left-most column is number 0.
---@return string type
---
---@see sqlite3.stmt.get_name
function sqlite3_stmt:get_type(n) end

---This function returns an array with the types of all columns in the result set returned by statement *stmt*.
---@return string[] types
---
---@see sqlite3.stmt.get_utypes - unpacked version.
function sqlite3_stmt:get_types() end

---This function returns a list with the names of all columns in the result set returned by statement *stmt*.
---@return string ...
---
---@see sqlite3.stmt.get_names - packed version.
function sqlite3_stmt:get_unames() end

---This function returns a list with the types of all columns in the result set returned by statement *stmt*.
---@return string ...
---
---@see sqlite3.stmt.get_types - packed version.
function sqlite3_stmt:get_utypes() end

---This function returns a list with the values of all columns in the current result row of a query.
---@return sqlite3.value ...
---
---@see sqlite3.stmt.get_values - packed version.
---@see sqlite3.stmt.urows - iterator.
function sqlite3_stmt:get_uvalues() end

---Returns the value of column *n* in the result set of statement *stmt*.
---@param n integer Column number. The left-most column is number 0.
---@return sqlite3.value value
function sqlite3_stmt:get_value(n) end

---This function returns an array with the values of all columns in the result set returned by statement *stmt*.
---@return sqlite3.value[] values
---
---@see sqlite3.stmt.get_uvalues - unpacked version.
---@see sqlite3.stmt.rows - iterator.
function sqlite3_stmt:get_values() end

---Returns `true` if *stmt* has not yet been finalized, `false` otherwise.
---@return boolean
function sqlite3_stmt:isopen() end

---Returns a function that iterates over the names and values of the result set of statement *stmt*. Each iteration returns a table with the names and values for the current row.
---@return fun(): { [string]: sqlite3.value } iter
---
---@see sqlite3.db.nrows - non-prepared equivalent.
---@see sqlite3.stmt.get_named_values - single row.
function sqlite3_stmt:nrows() end

---This function resets SQL statement *stmt*, so that it is ready to be re-executed. Any statement variables that had values bound to them using the `stmt:bind*()` functions retain their values.
function sqlite3_stmt:reset() end

---Returns an function that iterates over the values of the result set of statement *stmt*. Each iteration returns an array with the values for the current row.
---@return fun(): sqlite3.value[] iter
---
---@see sqlite3.db.rows - non-prepared equivalent.
---@see sqlite3.stmt.get_values - single row.
function sqlite3_stmt:rows() end

---This function must be called to evaluate the (next iteration of the) prepared statement *stmt*.
---@return integer code #
---`sqlite3.BUSY`: the engine was unable to acquire the locks needed. If the statement is a COMMIT or occurs outside of an explicit transaction, then you can retry the statement. If the statement is not a COMMIT and occurs within a explicit transaction then you should rollback the transaction before continuing.
---
---`sqlite3.DONE`: the statement has finished executing successfully. `stmt:step()` should not be called again on this statement without first calling `stmt:reset()` to reset the virtual machine back to the initial state.
---
---`sqlite3.ROW`: this is returned each time a new row of data is ready for processing by the caller. The values may be accessed using the column access functions. `stmt:step()` can be called again to retrieve the next row of data.
---
---`sqlite3.ERROR`: a run-time error (such as a constraint violation) has occurred. `stmt:step()` should not be called again. More information may be found by calling `db:errmsg()`. A more specific error code (can be obtained by calling `stmt:reset()`.
---
---sqlite3.MISUSE: the function was called inappropriately, perhaps because the statement has already been finalized or a previous call to stmt:step() has returned sqlite3.ERROR or sqlite3.DONE.
---
---@see sqlite3.db.progress_handler
function sqlite3_stmt:step() end

---Returns an function that iterates over the values of the result set of statement *stmt*. Each iteration returns the values for the current row.
---@return fun(): ...: sqlite3.value iter
---
---@see sqlite3.db.urows - non-prepared equivalent.
---@see sqlite3.stmt.get_uvalues - single row.
function sqlite3_stmt:urows() end

---This function returns the rowid of the most recent INSERT into the database corresponding to this statement.
---@return integer rowid
---
---@see sqlite3.db.last_insert_rowid
function sqlite3_stmt:last_insert_rowid() end

---After opening a database with [`sqlite3.open()`](lua://sqlite3.open) or [`sqlite3.open_memory()`](lua://sqlite3.open_memory) the returned database object should be used for all further method calls in connection with that database.
---
---@class sqlite3.db
local sqlite3_db = {}

---Sets a busy handler for a database. To remove the handler, pass nil as the first argument.
---@generic T
---@param func fun(udata: T, retries: integer): any Function that implements the busy handler. Called with two parameters: *udata* and the number of (re-)tries for a pending transaction. It should return nil, `false` or 0 if the transaction is to be aborted. All other values will result in another attempt to perform the transaction.
---@param udata? T Value passed to `func`.
---
---@see sqlite3.db.busy_timeout
---@see sqlite3.db.progress_handler
function sqlite3_db:busy_handler(func, udata) end

---Removes a handler previously set by [`sqlite3.db:busy_handler`](lua://sqlite3.db.busy_handler).
---@param func? nil
function sqlite3_db:busy_handler(func) end

---Sets a busy handler that waits for *t* milliseconds if a transaction cannot proceed. Calling this function will remove any busy handler set by [`sqlite3.db:busy_handler()`](lua://sqlite3.db.busy_handler); calling it with an argument less than or equal to 0 will turn off all busy handlers.
---@param t integer Milliseconds.
---
---@see sqlite3.db.busy_handler
function sqlite3_db:busy_timeout(t) end

---This function returns the number of database rows that were changed (or inserted or deleted) by the most recent SQL statement. Only changes that are directly specified by INSERT, UPDATE, or DELETE statements are counted. Auxiliary changes caused by triggers are not counted.
---@return integer rows
---
---@see sqlite3.db.total_changes - total number of changes.
function sqlite3_db:changes() end

---Closes a database. All SQL statements prepared using [`sqlite3.db:prepare()`](lua://sqlite3.db.prepare) should have been finalized before this function is called.
---@return integer code `sqlite3.OK` on success or else a numerical error code.
---
---@see sqlite3.db.close_vm - finalize all statements.
---@see sqlite3.stmt.finalize - finalize one statement.
function sqlite3_db:close() end

---Finalizes all statements that have not been explicitly finalized. If temponly is `true`, only internal, temporary statements are finalized.
---@param temponly boolean
---
---@see sqlite3.stmt.finalize - finalize one statement.
function sqlite3_db:close_vm(temponly) end

---Returns a lightuserdata corresponding to the open database. Use with [`sqlite3.open_ptr`](lua://sqlite3.open_ptr) to pass a database connection between threads. (When using lsqlite3 in a multithreaded environment, each thread has a separate Lua environment; full userdata structures can't be passed from one thread to another, but this is possible with lightuserdata.)
---@return lightuserdata ptr
---
---@see sqlite3.open_ptr
function sqlite3_db:get_ptr() end

---This function installs a commit_hook callback handler.
---@generic T
---@param func fun(udata: T): any Function that is invoked by SQLite3 whenever a transaction is commited. Receives one argument: the *udata* argument used when the callback was installed. If *func* returns `false` or nil the COMMIT is allowed to prodeed, otherwise the COMMIT is converted to a ROLLBACK.
---@param udata T Value passed to `func`.
---
---@see sqlite3.db.rollback_hook
---@see sqlite3.db.update_hook
function sqlite3_db:commit_hook(func, udata) end

---This function creates an aggregate callback function. Aggregates perform an operation over all rows in a query.
---@generic T
---@param name string Name of the aggregate function as given in an SQL statement.
---@param nargs integer Number of arguments this call will provide.
---@param step fun(context: sqlite3.context<T>, ...: sqlite3.value) Function that gets called once for every row; it should accept a function context plus the same number of parameters as given in *nargs*.
---@param final fun(context: sqlite3.context<T>) Function that is called once after all rows have been processed; it receives one argument, the function context.
---@param userdata? T Returned by the [`sqlite3.context:user_data()`](lua://sqlite3.context.user_data) method.
---
---@see sqlite3.context
---@see sqlite3.db.create_function
function sqlite3_db:create_aggregate(name, nargs, step, final, userdata) end

---This creates a collation callback. A collation callback is used to establish a collation order, mostly for string comparisons and sorting purposes.
---@param name string Name of the collation to be created.
---@param func fun(a: string, b: string): -1|0|1 Function that accepts two string arguments, compares them and returns 0 if both strings are identical, -1 if the first argument is lower in the collation order than the second and 1 if the first argument is higher in the collation order than the second.
function sqlite3_db:create_collation(name, func) end

---This function creates a callback function. Callback function are called by SQLite3 once for every row in a query.
---@generic T
---@param name string Name of the callback function as given in an SQL statement.
---@param nargs integer Number of arguments this call will provide.
---@param func fun(context: sqlite3.context<T>, ...: sqlite3.value) The actual Lua function that gets called once for every row; it should accept a function context (see Methods for callback contexts) plus the same number of parameters as given in nargs.
---@param userdata? T Returned by the [`sqlite3.context:user_data()`](lua://sqlite3.context.user_data) method.
---
---@see sqlite3.context
---@see sqlite3.db.create_aggregate
function sqlite3_db:create_function(name, nargs, func, userdata) end

---Loads an SQLite extension library from the named file into this database connection. Returns `true` when successful, or `false` and an error string otherwise.
---@param name string Filename.
---@param entrypoint? string Library initialization function name; if not supplied, SQLite tries various default entrypoint names.
---@return boolean success
---@return string? error
function sqlite3_db:load_extension(name, entrypoint) end

---Disables the `load_extension()` SQL function, which is enabled as a side effect of calling [`sqlite3.db:load_extension`](lua://sqlite3.db.load_extension) with a name.
function sqlite3_db:load_extension() end

---Returns the numerical result code (or extended result code) for the most recent failed call associated with database *db*.
---@return integer code
function sqlite3_db:errcode() end

---Alias for [`sqlite3.db:errcode()`](lua://sqlite3.db.errcode).
---@return integer code
function sqlite3_db:error_code() end

---Returns a string that contains an error message for the most recent failed call associated with database *db*.
---@return string msg
function sqlite3_db:errmsg() end

---Alias for [`sqlite3.db:errmsg()`](lua://sqlite3.db.errmsg).
---@return string msg
function sqlite3_db:error_message() end

---Compiles and executes the SQL statement(s) given in string *sql*. The statements are simply executed one after the other and not stored.
---
---If one or more of the SQL statements are queries, then the callback function specified in *func* is invoked once for each row of the query result (if *func* is nil, no callback is invoked).
---@generic T
---@param func? fun(udata: T, columns: integer, values: string[], names: string[]): integer -- returns `sqlite3.OK` on success or else a numerical error code. The callback receives four arguments: *udata* (the third parameter of the `db:exec()` call), the number of columns in the row, a table with the column values and another table with the column names. The callback function should return 0. If the callback returns a non-zero value then the query is aborted, all subsequent SQL statements are skipped and `db:exec()` returns `sqlite3.ABORT`.
---@param udata? T Value passed to `func`.
function sqlite3_db:exec(sql, func, udata) end

---Alias for [`sqlite3.db:exec()`](lua://sqlite3.db.exec).
---@generic T
---@param func? fun(udata: T, columns: integer, values: string[], names: string[]): integer
---@param udata? T
---
---@see sqlite3.db.progress_handler
function sqlite3_db:execute(sql, func, udata) end

---This function causes any pending database operation to abort and return at the next opportunity.
---
---@see sqlite3.db.progress_handler - for automated interrupting.
function sqlite3_db:interrupt() end

---This function returns the filename associated with database name of connection *db*.
---@return string? filename `"main"` for the main database file, or the name specified after the AS keyword in an ATTACH statement for an attached database. If there is no attached database name on the database connection *db*, then no value is returned; if database name is a temporary or in-memory database, then an empty string is returned.
function sqlite3_db:db_filename() end

---Returns `true` if database *db* is open, `false` otherwise.
---@return boolean open
function sqlite3_db:isopen() end

---This function returns the rowid of the most recent INSERT into the database. If no inserts have ever occurred, 0 is returned. (Each row in an SQLite table has a unique 64-bit signed integer key called the 'rowid'. This id is always available as an undeclared column named ROWID, OID, or _ROWID_. If the table has a column of type INTEGER PRIMARY KEY then that column is another alias for the rowid.)
---
---If an INSERT occurs within a trigger, then the rowid of the inserted row is returned as long as the trigger is running. Once the trigger terminates, the value returned reverts to the last value inserted before the trigger fired.
---@return integer rowid
---
---@see sqlite3.stmt.last_insert_rowid
function sqlite3_db:last_insert_rowid() end

---Creates an iterator that returns the successive rows selected by the SQL statement given in string *sql*. Each call to the iterator returns a table in which the named fields correspond to the columns in the database.
---@param sql string
---@return fun(): { [string]: sqlite3.value } iter
---
---@see sqlite3.stmt.nrows - prepared equivalent.
function sqlite3_db:nrows(sql) end

---This function compiles the SQL statement in string sql into an internal representation and returns this as userdata. The returned object should be used for all further method calls in connection with this specific SQL statement.
---
---@param sql string
---@return sqlite3.stmt stmt See [`sqlite3.stmt`](lua://sqlite3.stmt).
function sqlite3_db:prepare(sql) end

---This function installs a callback function *func* that is invoked periodically during long-running calls to [`sqlite3.db:exec()`](lua://sqlite3.db.exec) or [`sqlite3.stmt:step()`](lua://sqlite3.stmt.step). The progress callback is invoked once for every *n* internal operations, where *n* is the first argument to this function. udata is passed to the progress callback function each time it is invoked. If a call to `sqlite3.db:exec()` or `sqlite3.stmt:step()` results in fewer than *n* operations being executed, then the progress callback is never invoked. Only a single progress callback function may be registered for each opened database and a call to this function will overwrite any previously set callback function. To remove the progress callback altogether, pass nil as the second argument.
---
---If the progress callback returns a result other than 0, then the current query is immediately terminated, any database changes are rolled back and the containing `sqlite.db:exec()` or `sqlite3.stmt:step()` call returns `sqlite3.INTERRUPT`. This feature can be used to cancel long-running queries.
---@generic T
---@param n integer Number of operations in between each call of `func`.
---@param func fun(udata: T): any Progress callback. This callback receives one argument: the *udata* argument used when the callback was installed.
---@param udata? T Value passed to `func`.
---
---@see sqlite3.db.busy_handler
---@see sqlite3.db.interrupt - for manual interrupting.
function sqlite3_db:progress_handler(n, func, udata) end

---Removes the progress callback set by [`sqlite3.db:progress_handler()`](lua://sqlite3.db.progress_handler) altogether.
---
---@param n? integer
---@param func? nil
function sqlite3_db:progress_handler(n, func, udata) end

---This function installs a rollback_hook callback handler.
---@generic T
---@param func fun(udata: T) Function that is invoked by SQLite3 whenever a transaction is rolled back. This callback receives one argument: the *udata* argument used when the callback was installed.
---@param udata? T Value passed to `func`.
---
---@see sqlite3.db.commit_hook
---@see sqlite3.db.update_hook
function sqlite3_db:rollback_hook(func, udata) end

---Creates an iterator that returns the successive rows selected by the SQL statement given in string *sql*. Each call to the iterator returns a table in which the numerical indices 1 to n correspond to the selected columns 1 to n in the database.
---@param sql string
---@return fun(): { [integer]: sqlite3.value } iter
---
---@see sqlite3.stmt.rows - prepared equivalent.
function sqlite3_db:rows(sql) end

---This function returns the number of database rows that have been modified by INSERT, UPDATE or DELETE statements since the database was opened. This includes UPDATE, INSERT and DELETE statements executed as part of trigger programs. All changes are counted as soon as the statement that produces them is completed by calling either stmt:reset() or stmt:finalize().
---@return integer rows
function sqlite3_db:total_changes() end

---This function installs a trace callback handler.
---@generic T
---@param func fun(udata: T, sql: string)  Function that is called by SQLite3 just before the evaluation of an SQL statement. This callback receives two arguments: the first is the udata argument used when the callback was installed; the second is a string with the SQL statement about to be executed.
---@param udata? T Value passed to `func`.
function sqlite3_db:trace(func, udata) end

---This function installs an update_hook Data Change Notification Callback handler.
---@generic T
---@param func fun(udata: T, operation: integer, database: string, table: string, rowid: integer) Function that is invoked by SQLite3 whenever a row is updated, inserted or deleted. This callback receives five arguments: the first is the *udata* argument used when the callback was installed; the second is an integer indicating the operation that caused the callback to be invoked (one of `sqlite3.UPDATE`, `sqlite3.INSERT`, or `sqlite3.DELETE`). The third and fourth arguments are the database and table name containing the affected row. The final callback parameter is the rowid of the row. In the case of an update, this is the rowid after the update takes place.
---@param udata? T Value passed to `func`.
---
---@see sqlite3.db.commit_hook
---@see sqlite3.db.rollback_hook
function sqlite3_db:update_hook(func, udata) end

---Creates an iterator that returns the successive rows selected by the SQL statement given in string *sql*. Each call to the iterator returns the values that correspond to the columns in the currently selected row.
---@param sql string
---@return fun(): ...: sqlite3.value iter
---
---@see sqlite3.stmt.urows - prepared equivalent.
function sqlite3_db:urows(sql) end

---Returns `true` if the string *sql* comprises one or more complete SQL statements and `false` otherwise.
---@param sql string
---@return boolean complete
function sqlite3.complete(sql) end

---Opens (or creates if it does not exist) an SQLite database with name *filename* and returns its handle as userdata (the returned object should be used for all further method calls in connection with this specific database).
---
---In case of an error, the function returns nil, an error code and an error message.
---@param filename string Database filename.
---@param flags? integer Default: `sqlite3.OPEN_READWRITE + sqlite3.OPEN_CREATE`.
---@return sqlite3.db|nil db See [`sqlite3.db`](lua://sqlite3.db).
---@return integer? errorCode SQL error code.
---@return string? errorMessage Description of the `errorCode` error.
function sqlite3.open(filename, flags) end

---Opens an SQLite database in memory and returns its handle as userdata. In case of an error, the function returns nil, an error code and an error message. (In-memory databases are volatile as they are never stored on disk.)
---@return sqlite3.db|nil db See [`sqlite3.db`](lua://sqlite3.db).
---@return integer? errorCode SQL error code.
---@return string? errorMessage Description of the `errorCode` error.
function sqlite3.open_memory() end

---Opens the SQLite database corresponding to the lightuserdata *db_ptr* and returns its handle as userdata. Use [`sqlite3.db:get_ptr()`](lua://sqlite3.db.get_ptr) to get a `db_ptr` for an open database.
---@param db_ptr lightuserdata
---@return sqlite3.db db
---
---@see sqlite3.db.get_ptr
function sqlite3.open_ptr(db_ptr) end

---Starts an SQLite Online Backup from *source_db* to *target_db* and returns its handle as userdata. The *source_db* and *target_db* are open databases; they may be in-memory or file-based databases. The *target_name* and *source_name* are `"main"` for the main database, `"temp"` for the temporary database, or the name specified after the AS keyword in an ATTACH statement for an attached database.
---
---The source and target databases must be different, or else the init call will fail with an error. A call to `sqlite3.backup_init` will fail, returning NULL, if there is already a read or read-write transaction open on the target database.
---
---If an error occurs within `sqlite3.backup_init`, then NULL is returned, and an error code and error message are stored in `target_db`. The error code and message for the failed call can be retrieved using [`sqlite3.db:errcode()`](lua://sqlite3.db.errcode), or [`sqlite3.db:errmsg()`](lua://sqlite3.db.errmsg).
---@param target_db sqlite3.db
---@param target_name string
---@param source_db sqlite3.db
---@param source_name string
---@return nil? # If the backup is successful, this function does not return anything.
function sqlite3.backup_init(target_db, target_name, source_db, source_name) end

---Sets or queries the directory used by SQLite for temporary files. If string *temp* is a directory name or nil, the temporary directory is set accordingly and the old value is returned. If temp is missing, the function simply returns the current temporary directory.
---@param temp? string|nil
---@return string temp
function sqlite3.temp_directory(temp) end

---Returns a string with SQLite version information, in the form 'x.y\[.z\[.p\]\]'.
---@return string version
---
---@see sqlite3.lversion - in the form 'x.y\[.z\[.p\]\]'.
function sqlite3.version() end

---Returns a string with lsqlite3 library version information, in the form 'x.y\[.z\]'.
---@return string version - in the form 'x.y\[.z\[.p\]\]'.
---
---@see sqlite3.version
function sqlite3.lversion() end
