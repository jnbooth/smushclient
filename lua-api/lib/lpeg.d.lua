---@meta

---LPeg: A pattern-matching library for Lua, based on Parsing Expression Grammars (PEGs).
---
---From https://www.inf.puc-rio.br/~roberto/lpeg/.
lpeg = {}

---LPeg pattern compiled to userdata.
---@class lpeg.pattern: userdata
---@operator len: lpeg.pattern
---@operator unm: lpeg.pattern
---@operator add(lpeg.patternlike): lpeg.pattern
---@operator sub(lpeg.patternlike): lpeg.pattern
---@operator mul(lpeg.patternlike): lpeg.pattern
---@operator div(string|number|table|function): lpeg.pattern
---@operator mod(function): lpeg.pattern
---@operator pow(integer): lpeg.pattern

---@alias lpeg.matcher fun(subject: string, position: integer, ...: unknown): integer|boolean|nil?

---@alias lpeg.patternlike lpeg.pattern|string|integer|boolean|table|lpeg.matcher

---Table with patterns for matching some character classes according to the current locale. Each pattern matches any single character that belongs to its class.
---
---@class lpeg.locale
---@field alnum lpeg.pattern
---@field alpha lpeg.pattern
---@field cntrl lpeg.pattern
---@field digit lpeg.pattern
---@field graph lpeg.pattern
---@field lower lpeg.pattern
---@field print lpeg.pattern
---@field punct lpeg.pattern
---@field space lpeg.pattern
---@field upper lpeg.pattern
---@field xdigit lpeg.pattern

---The running version of LPeg.
---@type string
lpeg.version = lpeg.version

---The matching function. It attempts to match the given pattern against the subject string. If the match succeeds, returns the index in the subject of the first character after the match, or the captured values (if the pattern captured any value).
---
---Unlike typical pattern-matching functions, `lpeg.match` works only in *anchored* mode; that is, it tries to match the pattern with a prefix of the given subject string (at position *init*), not with an arbitrary substring of the subject. So, if we want to find a pattern anywhere in a string, we must either write a loop in Lua or write a pattern that matches anywhere. This second approach is easy and quite efficient.
---@param pattern lpeg.patternlike
---@param subject string Subject.
---@param init? integer Start the match at the specified position in the subject string. As in the Lua standard libraries, a negative value counts from the end.
---@return integer|string ... Index or captured values.
function lpeg.match(pattern, subject, init) end

---If the given value is a pattern, returns the string `"pattern"`. Otherwise returns `nil`.
---@param value unknown
---@return "pattern"|nil type
function lpeg.type(value) end

---Sets a limit for the size of the backtrack stack used by LPeg to track calls and choices. (The default limit is 400.) Most well-written patterns need little backtrack levels and therefore you seldom need to change this limit; before changing it you should try to rewrite your pattern to avoid the need for extra space. Nevertheless, a few useful patterns may overflow. Also, with recursive grammars, subjects with deep recursion may also need larger limits.
---
---@param max integer
function lpeg.setmaxstack(max) end

---Converts the given value into a proper pattern, according to the following rules:
---
---* If the argument is a pattern, it is returned unmodified.
---* If the argument is a string, it is translated to a pattern that matches the string literally.
---* If the argument is a non-negative number *n*, the result is a pattern that matches exactly *n* characters.
---* If the argument is a negative number *-n*, the result is a pattern that succeeds only if the input string has less than *n* characters left: `lpeg.P(-n)` is equivalent to `-lpeg.P(n)`.
---* If the argument is a boolean, the result is a pattern that always succeeds or always fails (according to the boolean value), without consuming any input.
---* If the argument is a table, it is interpreted as a grammar.
---* If the argument is a function, returns a pattern equivalent to a match-time capture ([`lpeg.Cmt`](lua://lpeg.Cmt)) over the empty string.
---@param value lpeg.patternlike
---@return lpeg.pattern pattern
---
---@see lpeg.match
function lpeg.P(value) end

---Returns a pattern that matches only if the input string at the current position is preceded by *patt*. Pattern *patt* must match only strings with some fixed length, and it cannot contain captures.
---
---Like the and predicate, this pattern never consumes any input, independently of success or failure.
---@param patt lpeg.patternlike
---@return lpeg.pattern pattern
---
---@see lpeg.match
function lpeg.B(patt) end

---Returns a pattern that matches any single character belonging to one of the given ranges. Each range is a string *xy* of length 2, representing all characters with code between the codes of *x* and *y* (both inclusive).
---
---As an example, the pattern `lpeg.R("09")` matches any digit, and `lpeg.R("az", "AZ")` matches any ASCII letter.
---@param ... string
---@return lpeg.pattern pattern
---
---@see lpeg.match
---@see lpeg.utfR - Unicode codepoint range.
function lpeg.R(...) end

---Returns a pattern that matches any single character that appears in the given string. (The `S` stands for *Set*.)
---
---As an example, the pattern `lpeg.S("+-*/")` matches any arithmetic operator.
---
---Note that, if *set* is a character (that is, a string of length 1), then `lpeg.P(set)` is equivalent to `lpeg.S(set)` which is equivalent to `lpeg.R(set..set)`. Note also that both `lpeg.S("")` and `lpeg.R()` are patterns that always fail.
---@param set string
---@return lpeg.pattern pattern
---
---@see lpeg.match
function lpeg.S(set) end

---Returns a pattern that matches a valid UTF-8 byte sequence representing a code point in the range `[cp1, cp2]`. The range is limited by the natural Unicode limit of `0x10FFFF`, but may include surrogates.
---@see lpeg.R - character range.
---@param cp1 integer Start of the UTF-8 code point range, inclusive.
---@param cp2 integer End of the UTF-8 code point range, inclusive.
---@return lpeg.pattern pattern
---
---@see lpeg.match
function lpeg.utfR(cp1, cp2) end

---This operation creates a non-terminal (a *variable*) for a grammar. The created non-terminal refers to the rule indexed by *v* in the enclosing grammar.
---@param v lpeg.patternlike
---@return lpeg.pattern pattern
---
---@see lpeg.match
function lpeg.V(v) end

---Returns a table with patterns for matching some character classes according to the current locale. Each pattern matches any single character that belongs to its class.
---@param table? table If called with an argument *table* then it creates those fields inside the given table and returns that table.
---@return lpeg.locale locale See [`lpeg.locale`](lua://lpeg.locale).
---
---@see lpeg.match
function lpeg.locale(table) end

---Creates a simple capture, which captures the substring of the subject that matches *patt*. The captured value is a string. If *patt* has other captures, their values are returned after this one.
---@param patt lpeg.patternlike
---@return lpeg.pattern pattern
---
---@see lpeg.match
function lpeg.C(patt) end

---Creates an *argument capture*. This pattern matches the empty string and produces the value given as the *n*<sup>th</sup> extra argument given in the call to [`lpeg.match`](lua://lpeg.match).
---@param n integer Argument index.
---@return lpeg.pattern
---
---@see lpeg.match
function lpeg.Carg(n) end

---Creates a *back capture*. This pattern matches the empty string and produces the values produced by the most recent group capture ([`lpeg.Cg`](lua://lpeg.Cg)) named *key* (where *key* can be any Lua value).
---
---*Most recent* means the last complete outermost group capture ([`lpeg.Cg`](lua://lpeg.Cg)) with the given *key*. A *Complete* capture means that the entire pattern corresponding to the capture has matched; in other words, the back capture is not nested inside the group. An *Outermost* capture means that the capture is not inside another complete capture that does not contain the back capture itself.
---
---In the same way that LPeg does not specify when it evaluates captures, it does not specify whether it reuses values previously produced by the group or re-evaluates them.
---@param key any
---@return lpeg.pattern pattern
---
---@see lpeg.match
function lpeg.Cb(key) end

---Creates a *constant capture*. This pattern matches the empty string and produces all given values as its captured values.
---@param ... any
---@return lpeg.pattern pattern
---
---@see lpeg.match
function lpeg.Cc(...) end

---Creates a *fold capture*. This construction is deprecated; use an accumulator pattern instead. In general, a fold like `lpeg.Cf(p1 * p2^0, func)` can be translated to `(p1 * (p2 % func)^0)`.
---@deprecated
---@param patt lpeg.patternlike
---@param func function
---
---@see lpeg.match
function lpeg.Cf(patt, func) end

---Creates a *group capture*. It groups all values returned by *patt* into a single capture. The group may be anonymous (if no key is given) or named with the given key (which can be any non-nil Lua value).
---
---An anonymous group serves to join values from several captures into a single capture. A named group has a different behavior. In most situations, a named group returns no values at all. Its values are only relevant for a following back capture ([`lpeg.Cb`](lua://lpeg.Cb)) or when used inside a table capture ([`lpeg.Ct`](lua://lpeg.Ct)).
---@param patt lpeg.patternlike
---@param key? any
---@return lpeg.pattern pattern
---
---@see lpeg.match
function lpeg.Cg(patt, key) end

---Creates a *position capture*. It matches the empty string and captures the position in the subject where the match occurs. The captured value is a number.
---@return lpeg.pattern pattern
---
---@see lpeg.match
function lpeg.Cp() end

---Creates a *substitution capture*, which captures the substring of the subject that matches *patt*, with *substitutions*. For any capture inside *patt* with a value, the substring that matched the capture is replaced by the capture value (which should be a string). The final captured value is the string resulting from all replacements.
---@param patt lpeg.patternlike
---@return lpeg.pattern pattern
---
---@see lpeg.match
function lpeg.Cs(patt) end

---Creates a *table capture*. This capture returns a table with all values from all anonymous captures made by *patt* inside this table in successive integer keys, starting at 1. Moreover, for each named capture group created by *patt*, the first value of the group is put into the table with the group key as its key. The captured value is only the table.
---@param patt lpeg.patternlike
---@return lpeg.pattern pattern
---
---@see lpeg.match
function lpeg.Ct(patt) end


---Creates a *match-time capture*. Unlike all other captures, this one is evaluated immediately when a match occurs (even if it is part of a larger pattern that fails later). It forces the immediate evaluation of all its nested captures and then calls *func*.
---
---The given function gets as arguments the entire subject, the current position (after the match of *patt*), plus any capture values produced by *patt*.
---
---The first value returned by *func* defines how the match happens. If the call returns a number, the match succeeds and the returned number becomes the new current position. (Assuming a subject *s* and current position *i*, the returned number must be in the range *\[i, len(s) + 1\]*.) If the call returns `true`, the match succeeds without consuming any input. (So, to return `true` is equivalent to return *i*.) If the call returns `false`, `nil`, or no value, the match fails.
---
---Any extra values returned by the function become the values produced by the capture.
---@param patt lpeg.patternlike
---@param func lpeg.matcher
---@return lpeg.pattern pattern
---
---@see lpeg.match
function lpeg.Cmt(patt, func) end
