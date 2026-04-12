---@meta

---lrexlib: PCRE bindings.
---
---From https://github.com/rrthomas/lrexlib.
rex = {}

---Userdata of character tables. Created by [`rex.maketables()`](lua://rex.maketables).
---@class rex.locale: userdata

---Regular expression pattern compiled to userdata. Created by [`rex.new()`](lua://rex.new).
---@class rex.pattern: userdata
local rex_pattern = {}

---The function searches for the first match of the compiled regexp in the string *subj*, starting from offset *init*, subject to flags *ef*.
---
---Returns `nil` on failure.
---@param subj string Subject.
---@param init? integer Start offset in the subject (can be negative). Default: 1.
---@param ef? integer Execution flags (bitwise OR). Default value: 0.
---@return nil|string|false ... All substring matches ("captures") in the order they appear in the pattern. `false` is returned for sub-patterns that did not participate in the match. If the pattern specified no captures then the whole matched substring is returned.
---
---@see rex.match - non-compiled equivalent.
---@see rex.pattern.find - returns `integer, integer, ...string|false`.
---@see rex.pattern.tfind - returns `integer, integer, { [integer|string]: string|false }`.
---@see rex.pattern.exec - returns `integer, integer, { [integer|string]: integer|false }`.
function rex_pattern:match(subj, init, ef) end

---The method searches for the first match of the compiled regexp in the string *subj*, starting from offset *init*, subject to flags *cf* and *ef*.
---
---Returns `nil` on failure.
---@param subj string Subject.
---@param init? integer Start offset in the subject (can be negative). Default: 1.
---@param ef? integer Execution flags (bitwise OR). Default value: 0.
---@return integer|nil start The start point of the match.
---@return integer? end The end point of the match.
---@return string|false|nil ... All substring matches ("captures"), in the order they appear in the pattern. `false` is returned for sub-patterns that did not participate in the match.
---
---@see rex.find
---@see rex.pattern.match - returns `...string|false`.
---@see rex.pattern.tfind - returns `integer, integer, { [integer|string]: string|false }`.
---@see rex.pattern.exec - returns `integer, integer, { [integer|string]: integer|false }`.
function rex_pattern:find(subj, patt, init, cf, ef, locale) end

---The method searches for the first match of the compiled regexp in the string *subj*, starting from offset *init*, subject to execution flags *ef*.
---
---Returns `nil` on failure.
---@param subj string Subject.
---@param init? integer Start offset in the subject (can be negative). Default: 1.
---@param ef? integer Execution flags (bitwise OR). Default value: 0.
---@return integer|nil start The start point of the match.
---@return integer? end The end point of the match.
---@return { [integer|string]: string|false }? matches Substring matches in a table. This table contains `false` in the positions where the corresponding sub-pattern did not participate in the match. The table also contains substring matches keyed by their correspondent subpattern names (strings).
---
---@see rex.pattern.match - returns `...string|false`.
---@see rex.pattern.find - returns `integer, integer, ...string|false`.
---@see rex.pattern.exec - returns `integer, integer, { [integer|string]: integer|false }`.
function rex_pattern:tfind(subj, init, ef) end

---The method searches for the first match of the compiled regexp in the string *subj*, starting from offset *init*, subject to execution flags *ef*.
---
---Returns `nil` on failure.
---@param subj string Subject.
---@param init? integer Start offset in the subject (can be negative). Default: 1.
---@param ef? integer Execution flags (bitwise OR). Default value: 0.
---@return integer|nil start The start point of the match.
---@return integer? end The end point of the match.
---@return { [integer|string]: integer|false }? matches Substring match offsets in a table. This table contains `false` in the positions where the corresponding sub-pattern did not participate in the match. The table also contains substring matches keyed by their correspondent subpattern names (strings).
---
---@see rex.pattern.match - returns `...string|false`.
---@see rex.pattern.find - returns `integer, integer, ...string|false`.
---@see rex.pattern.tfind - returns `integer, integer, { [integer|string]: string|false }`.
function rex_pattern:exec(subj, init, ef) end

---This function returns a table containing information about the compiled pattern. The keys are strings formed in the following way: `PCRE2_INFO_CAPTURECOUNT` -> `"CAPTURECOUNT"`.
---@return { [string]: integer } info
function rex_pattern:patterninfo() end

---The method matches a compiled regular expression *r* against a given subject string *subj*, using a DFA matching algorithm.
---
---Returns `nil` on failure.
---@param subj string Subject.
---@param init? integer Start offset in the subject (can be negative). Default: 1.
---@param ef? integer Execution flags (bitwise OR). Default value: 0.
---@param ovecsize? integer Size of the array for result offsets. Default: 100.
---@param wscount? integer Number of elements in the working space array. Default: 50.
---@return integer|nil start Start point of the matches found.
---@return integer[]? endpoints A table containing the end points of the matches found, the longer matches first.
---@return integer? code The return value of the underlying `pcre_dfa_exec` call.
function rex_pattern:dfa_exec(subj, init, ef, ovecsize, wscount) end

---Requests JIT compilation, which, if the just-in-time compiler is available, further processes a compiled pattern into machine code that executes much faster than the interpretive matching function.
---
---The method returns `true` on success or `false`, *error message string* on failure.
---@param options? integer Bitwise OR of separate options. Default: `PCRE2_JIT_COMPLETE`.
---@return boolean success
---@return string? error
function rex_pattern:jit_compile(options) end

---The function compiles regular expression *patt* into a regular expression object as userdata. The returned result then can be used by the methods, e.g. [`rex.pattern:tfind`](lua://rex.pattern.tfind), [`rex.pattern:exec`](lua://rex.pattern.exec), etc. Regular expression objects are automatically garbage collected.
---@param patt string Regular expression pattern.
---@param cf? integer Compilation flags (bitwise OR). Default value: 0.
---@param locale? string|rex.locale String (e.g. "French_France.1252") or userdata obtained from [`rex.maketables`](lua://rex.maketables). Default: built-in set.
---@return rex.pattern pattern
function rex.new(patt, cf, locale) end

---The function searches for the first match of the regexp patt in the string *subj*, starting from offset *init*, subject to flags *cf* and *ef*.
---
---Returns `nil` on failure.
---@param subj string Subject.
---@param patt rex.pattern|string Regular expression pattern.
---@param init? integer Start offset in the subject (can be negative). Default: 1.
---@param cf? integer Compilation flags (bitwise OR). Default value: 0.
---@param ef? integer Execution flags (bitwise OR). Default value: 0.
---@param locale? string|rex.locale String (e.g. "French_France.1252") or userdata obtained from [`rex.maketables`](lua://rex.maketables). Default: built-in set.
---@return nil|string|false ... All substring matches ("captures") in the order they appear in the pattern. `false` is returned for sub-patterns that did not participate in the match. If the pattern specified no captures then the whole matched substring is returned.
---
---@see rex.find - returns `integer, integer, ...string|false`.
---@see rex.gmatch - returns an iterator over all matches.
---@see rex.pattern.match - compiled equivalent.
function rex.match(subj, patt, init, cf, ef, locale) end

---The function searches for the first match of the regexp *patt* in the string *subj*, starting from offset *init*, subject to flags *cf* and *ef*.
---
---Returns `nil` on failure.
---@param subj string Subject.
---@param patt rex.pattern|string Regular expression pattern.
---@param init? integer Start offset in the subject (can be negative). Default: 1.
---@param cf? integer Compilation flags (bitwise OR). Default value: 0.
---@param ef? integer Execution flags (bitwise OR). Default value: 0.
---@param locale? string|rex.locale String (e.g. "French_France.1252") or userdata obtained from [`rex.maketables`](lua://rex.maketables). Default: built-in set.
---@return integer|nil start The start point of the match.
---@return integer? end The end point of the match.
---@return string|false ... All substring matches ("captures") in the order they appear in the pattern. `false` is returned for sub-patterns that did not participate in the match.
---
---@see rex.pattern.find - compiled equivalent.
function rex.find(subj, patt, init, cf, ef, locale) end

---The function is intended for use in the generic for Lua construct. It returns an iterator for repeated matching of the pattern patt in the string *subj*, subject to flags *cf* and *ef*.
---
---The iterator function is called by Lua. On every iteration (that is, on every match), it returns all captures in the order they appear in the pattern (or the entire match if the pattern specified no captures). The iteration will continue till the subject fails to match.
---@param subj string Subject.
---@param patt rex.pattern|string Regular expression pattern.
---@param cf? integer Compilation flags (bitwise OR). Default value: 0.
---@param ef? integer Execution flags (bitwise OR). Default value: 0.
---@param locale? string|rex.locale String (e.g. "French_France.1252") or userdata obtained from [`rex.maketables`](lua://rex.maketables). Default: built-in set.
---@return fun(): ...: string|false iter
function rex.gmatch(subj, patt, cf, ef, locale) end

---This function searches for all matches of the pattern *patt* in the string *subj* and replaces them according to the parameters *repl* and *n*.
---
---The parameter *repl* can be either a string, a function or a table. On each match made, it is converted into a value *repl_out* that may be used for the replacement.
---
---*repl_out* is generated differently depending on the type of *repl*:
---
---1. If *repl* is a string then it is treated as a template for substitution, where the %X occurences in *repl* are handled in a special way, depending on the value of the character X:
---
---   * if X represents a digit, then each %X occurence is substituted by the value of the X<sup>th</sup> submatch (capture), with the following cases handled specially:
---     * each %0 is substituted by the entire match
---     * if the pattern contains no captures, then each %1 is substituted by the entire match
---     * any other %X where X is greater than the number of captures in the pattern will generate an error ("invalid capture index")
---     * if the pattern does contain a capture with number X but that capture didn't participate in the match, then %X is substituted by an empty string
---   * if X is any non-digit character then %X is substituted by X
---
---  All parts of *repl* other than %X are copied to *repl_out* verbatim.
---
---2. If *repl* is a function then it is called on each match with the submatches passed as parameters (if there are no submatches then the entire match is passed as the only parameter). *repl_out* is the return value of the *repl* call, and is interpreted as follows:
---
---   * if it is a string or a number (coerced to a string), then the replacement value is that string;
---   * if it is a nil or a false, then no replacement is to be done;
---
---3. If *repl* is a table then *repl_out* is `repl[m1]`, where *m1* is the first submatch (or the entire match if there are no submatches), following the same rules as for the return value of *repl* call, described in the above paragraph.
---
---gsub behaves differently depending on the type of *n*:
---
---1. If *n* is a number then it is treated as the maximum number of matches to search for (an omitted or nil value means an unlimited number of matches). On each match, the replacement value is the *repl_out* string (see above).
---
---2. If *n* is a function, then it is called on each match, after *repl_out* is produced (so if *repl* is a function, it will be called prior to the *n* call).
---   *n* receives 3 arguments and returns 2 values. Its arguments are:
---
---     1. The start offset of the match (a number)
---     2. The end offset of the match (a number)
---     3. *repl_out*
---
---   The type of its first return controls the replacement produced by gsub for the current match:
---
---     * `true` -- replace/don't replace, according to *repl_out*;
---     * `nil`/`false` -- don't replace;
---     * a string (or a number coerced to a string) -- replace by that string;
---
---   The type of its second return controls gsub behavior after the current match is handled:
---
---     * `nil`/`false` -- no changes: *n* will be called on the next match;
---     * `true` -- search for an unlimited number of matches; *n* will not be called again;
---     * a number -- maximum number of matches to search for, beginning from the next match; *n* will not be called again;
---@param subj string Subject.
---@param patt rex.pattern|string Regular expression pattern.
---@param repl string|table|fun(...: string): string|number|false|nil? Replacer. See above.
---@param n? integer|fun(start: integer, end: integer, out: string): true|nil|false|string Limiter. See above.
---@param cf? integer Compilation flags (bitwise OR). Default value: 0.
---@param ef? integer Execution flags (bitwise OR). Default value: 0.
---@param locale? string|rex.locale String (e.g. "French_France.1252") or userdata obtained from [`rex.maketables`](lua://rex.maketables). Default: built-in set.
---@return string replaced The subject string with the substitutions made.
---@return integer matches Number of matches found.
---@return integer substitutions Number of substitutions made.
function rex.gsub(subj, patt, repl, n, cf, ef, locale) end

---The function is intended for use in the *generic for* Lua construct. It is used for splitting a subject string* subj* into parts (*sections*). The *sep* parameter is a regular expression pattern representing separators between the sections.
---
---The function returns an iterator for repeated matching of the pattern *sep* in the string *subj*, subject to flags *cf* and *ef*.
---
---On every iteration pass, the iterator returns:
---
---1. A subject section (can be an empty string), followed by
---2. All captures in the order they appear in the *sep* pattern (or the entire match if the *sep* pattern specified no captures). If there is no match (this can occur only in the last iteration), then nothing is returned after the subject section.
---
---The iteration will continue till the end of the subject. Unlike [`rex.gmatch`](lua://rex.gmatch), there will always be at least one iteration pass, even if there are no matches in the subject.
---@param subj string Subject.
---@param sep rex.pattern|string Separator.
---@param cf? integer Compilation flags (bitwise OR). Default value: 0.
---@param ef? integer Execution flags (bitwise OR). Default value: 0.
---@param locale? string|rex.locale String (e.g. "French_France.1252") or userdata obtained from [`rex.maketables`](lua://rex.maketables). Default: built-in set.
---@return fun(): string, ...: string|false iter
function rex.split(subj, sep, cf, ef, locale) end

---This function counts matches of the pattern *patt* in the string *subj*.
---@param subj string Subject.
---@param patt rex.pattern|string Pattern.
---@param cf? integer Compilation flags (bitwise OR). Default value: 0.
---@param ef? integer Execution flags (bitwise OR). Default value: 0.
---@param locale? string|rex.locale String (e.g. "French_France.1252") or userdata obtained from [`rex.maketables`](lua://rex.maketables). Default: built-in set.
---@return integer matches Number of matches found.
function rex.count(subj, patt, cf, ef, locale) end

---This function returns a table containing the numeric values of the constants defined by the used regex library, with the keys being the (string) names of the constants. If the table argument *tb* is supplied then it is used as the output table, otherwise a new table is created.
---
---The constants contained in the returned table can then be used in most functions and methods where *compilation flags* or *execution flags* can be specified. They can also be used for comparing with return codes of some functions and methods for determining the reason of failure.
---@param tb? table A table for placing results into.
---@return { [string]: integer } tb A table filled with the results.
---
---@see rex.config - table of PCRE2 configuration parameters.
function rex.flags(tb) end

---Creates a set of character tables corresponding to the current locale and returns it as a userdata. The returned value can be passed to any Lrexlib function accepting the *locale* parameter.
---@return rex.locale locale See [`rex.locale`](lua://rex.locale).
function rex.maketables() end

---This function returns a table containing the values of the configuration parameters used at PCRE2 library build-time. Those parameters (numbers) are keyed by their names (strings). If the table argument *tb* is supplied then it is used as the output table, else a new table is created.
---@param tb? table A table for placing results into.
---@return { [string]: integer } tb A table filled with the results.
---
---@see rex.flags - table of regex library constants.
function rex.config(tb) end

---This function returns a string containing the version of the used PCRE2 library and its release date.
---@return string version
function rex.version() end
