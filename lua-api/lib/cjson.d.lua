---@meta

---cjson: JSON support for Lua.
---
---From https://kyne.au/%7Emark/software/lua-cjson-manual.html.
---@class cjson
cjson = {}

---The name of the Lua CJSON module.
---@type string
cjson._NAME = "cjson"

---The version number of the Lua CJSON module.
---@type string
cjson._VERSION = cjson._VERSION

---Lua CJSON decodes JSON null as a Lua lightuserdata NULL pointer. `cjson.null` is provided for comparison.
---@type lightuserdata
cjson.null = cjson.null

---`cjson.decode` will deserialise any UTF-8 JSON string into a Lua value or table.
---
---UTF-16 and UTF-32 JSON strings are not supported.
---
---`cjson.decode` requires that any NULL (ASCII 0) and double quote (ASCII 34) characters are escaped within strings. All escape codes will be decoded and other bytes will be passed transparently. UTF-8 characters are not validated during decoding and should be checked elsewhere if required.
---
---JSON null will be converted to a NULL lightuserdata value. This can be compared with `cjson.null` for convenience.
---
---By default, numbers incompatible with the JSON specification (infinity, NaN, hexadecimal) can be decoded. This default can be changed with [`cjson.decode_invalid_numbers`](lua://cjson.decode_invalid_numbers).
---
---Caution: care must be taken after decoding JSON objects with numeric keys. Each numeric key will be stored as a Lua string. Any subsequent code assuming type number may break.
---@param json_text string
---@return unknown value
---
---@see cjson.decode_invalid_numbers
---@see cjson.decode_max_depth
function cjson.decode(json_text) end

---Lua CJSON may generate an error when trying to decode numbers not supported by the JSON specification. Invalid numbers are defined as:
---
---* infinity
---* not-a-number (NaN)
---* hexadecimal
---
---The current setting is always returned, and is only updated when an argument is provided.
---@param setting?
---| true # Accept and decode invalid numbers. This is the default.
---| false # Throw an error when invalid numbers are encountered.
---@return boolean setting
---
---@see cjson.decode
---@see cjson.decode_max_depth
function cjson.decode_invalid_numbers(setting) end

---Lua CJSON will generate an error when parsing deeply nested JSON once the maximum array/object depth has been exceeded. This check prevents unnecessarily complicated JSON from slowing down the application, or crashing the application due to lack of process stack space.
---
---An error may be generated before the depth limit is hit if Lua is unable to allocate more objects on the Lua stack.
---
---By default, Lua CJSON will reject JSON with arrays and/or objects nested more than 1000 levels deep.
---
---The current setting is always returned, and is only updated when an argument is provided.
---@param depth? integer Must be positive. Default: 1000.
---@return integer depth
---
---@see cjson.decode
---@see cjson.decode_invalid_numbers
function cjson.decode_max_depth(depth) end

---`cjson.encode` will serialise a Lua value into a string containing the JSON representation.
---
---By default, numbers are encoded with 14 significant digits. Refer to [`cjson.encode_number_precision`](lua://cjson.encode_number_precision) for details.
---
---Lua CJSON will escape the following characters within each UTF-8 string:
---
---* Control characters (ASCII 0 - 31)
---* Double quote (ASCII 34)
---* Forward slash (ASCII 47)
---* Blackslash (ASCII 92)
---* Delete (ASCII 127)
---
---All other bytes are passed transparently.
---
---Caution: Lua CJSON will successfully encode/decode binary strings, but this is technically not supported by JSON and may not be compatible with other JSON libraries. To ensure the output is valid JSON, applications should ensure all Lua strings passed to `cjson.encode` are UTF-8.
---
---Base64 is commonly used to encode binary data as the most efficient encoding under UTF-8 can only reduce the encoded size by a further ~8%. Lua Base64 routines can be found in the LuaSocket and lbase64 packages.
---
---Lua CJSON uses a heuristic to determine whether to encode a Lua table as a JSON array or an object. A Lua table with only positive integer keys of type number will be encoded as a JSON array. All other tables will be encoded as a JSON object.
---
---Lua CJSON does not use metamethods when serialising tables.
---
---* [`rawget`](lua://rawget) is used to iterate over Lua arrays
---* [`next`](lua://next) is used to iterate over Lua objects
---
---Lua arrays with missing entries (sparse arrays) may optionally be encoded in several different ways. Refer to [`cjson.encode_sparse_array`](lua://cjson.encode_sparse_array) for details.
---
---JSON object keys are always strings. Hence `cjson.encode` only supports table keys which are type number or string. All other types will generate an error.
---
---Note: standards compliant JSON must be encapsulated in either an object (`{}`) or an array (`[]`). If strictly standards compliant JSON is desired, a table must be passed to cjson.encode.
---@param value boolean|lightuserdata|nil|number|string|table For lightuserdata, only a null pointer is permitted.
---@return string json_text
---
---@see cjson.encode_invalid_numbers
---@see cjson.encode_keep_buffer
---@see cjson.encode_max_depth
---@see cjson.encode_number_precision
---@see cjson.encode_sparse_array
function cjson.encode(value) end

---Lua CJSON may generate an error when encoding floating point numbers not supported by the JSON specification (invalid numbers):
---
---* infinity
---* not-a-number (NaN)
---
---The current setting is always returned, and is only updated when an argument is provided.
---@param setting?
---| false # Throw an error when attempting to encode invalid numbers. This is the default setting.
---| true # Allow invalid numbers to be encoded. This will generate non-standard JSON, but this output is supported by some libraries.
---| "null" # Encode invalid numbers as a JSON null value. This allows infinity and NaN to be encoded into valid JSON.
---@return boolean|"null" setting
---
---@see cjson.encode
---@see cjson.encode_keep_buffer
---@see cjson.encode_max_depth
---@see cjson.encode_number_precision
---@see cjson.encode_sparse_array
function cjson.encode_invalid_numbers(setting) end

---Lua CJSON can reuse the JSON encoding buffer to improve performance.
---
---The current setting is always returned, and is only updated when an argument is provided.
---@param setting?
---| true # The buffer will grow to the largest size required and is not freed until the Lua CJSON module is garbage collected. This is the default setting.
---| false # Free the encode buffer after each call to [`cjson.encode`](lua://cjson.encode).
---@return boolean setting
---
---@see cjson.encode
---@see cjson.encode_invalid_numbers
---@see cjson.encode_max_depth
---@see cjson.encode_number_precision
---@see cjson.encode_sparse_array
function cjson.encode_keep_buffer(setting) end

---Once the maximum table depth has been exceeded Lua CJSON will generate an error. This prevents a deeply nested or recursive data structure from crashing the application.
---
---By default, Lua CJSON will generate an error when trying to encode data structures with more than 1000 nested tables.
---
---The current setting is always returned, and is only updated when an argument is provided.
---@param depth integer? Must be a positive integer. Default: 1000.
---@return integer depth
---
---@see cjson.encode
---@see cjson.encode_invalid_numbers
---@see cjson.encode_keep_buffer
---@see cjson.encode_number_precision
---@see cjson.encode_sparse_array
function cjson.encode_max_depth(depth) end

---The amount of significant digits returned by Lua CJSON when encoding numbers can be changed to balance accuracy versus performance. For data structures containing many numbers, setting `cjson.encode_number_precision` to a smaller integer, for example 3, can improve encoding performance by up to 50%.
---
---By default, Lua CJSON will output 14 significant digits when converting a number to text.
---
---The current setting is always returned, and is only updated when an argument is provided.
---@param precision? integer Must be between 1 and 14. Default: 14.
---@return integer precision
---
---@see cjson.encode
---@see cjson.encode_invalid_numbers
---@see cjson.encode_keep_buffer
---@see cjson.encode_max_depth
---@see cjson.encode_sparse_array
function cjson.encode_number_precision(precision) end

---Lua CJSON classifies a Lua table into one of three kinds when encoding a JSON array. This is determined by the number of values missing from the Lua array as follows:
---
---* Normal: All values are available.
---* Sparse: At least 1 value is missing.
---* Excessively sparse: The number of values missing exceeds the configured ratio.
---
---Lua CJSON encodes sparse Lua arrays as JSON arrays using JSON null for the missing entries.
---
---An array is excessively sparse when all the following conditions are met:
---
---* `ratio > 0`
---* `maximum_index > safe`
---* `maximum_index > item_count * ratio`
---
---Lua CJSON will never consider an array to be excessively sparse when `ratio = 0`. The safe limit ensures that small Lua arrays are always encoded as sparse arrays.
---
---By default, attempting to encode an excessively sparse array will generate an error. If convert is set to true, excessively sparse arrays will be converted to a JSON object.
---
---The current settings are always returned. A particular setting is only changed when the argument is provided (non-nil).
---@param convert? boolean Default: false.
---@param ratio? integer Must be a positive integer. Default: 2.
---@param safe? integer Must be a positive integer. Default: 10.
---@return boolean convert
---@return integer ratio
---@return integer safe
---
---@see cjson.encode
---@see cjson.encode_invalid_numbers
---@see cjson.encode_keep_buffer
---@see cjson.encode_max_depth
---@see cjson.encode_number_precision
function cjson.encode_sparse_array(convert, ratio, safe) end

---Instantiates an independent copy of the Lua CJSON module. The new module has a separate persistent encoding buffer, and default settings.
---@return cjson clone
function cjson.new() end
