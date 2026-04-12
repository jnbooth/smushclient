---@meta

---LuaBitOp: Bitwise operations on numbers.
---
---From https://bitop.luajit.org/index.html.
bit = {}

---Normalizes a number to the numeric range for bit operations and returns it. This function is usually not needed since all bit operations already normalize all of their input arguments.
---@param x number
---@return number y
function bit.tobit(x) end

---Converts a number to a hex string.
---@param x number Input number.
---@param n? integer Number of hex digits. Only the least-significant 4*|n| bits are used. Positive numbers between 1 and 8 generate lowercase hex digits. Negative numbers generate uppercase hex digits. Default: 8.
---@return string y Hex string encoding of input number.
function bit.tohex(x, n) end

---Returns the bitwise not of its argument. **Use `~x` instead.**
---@param x number
---@return number y
function bit.bnot(x) end

---Returns the bitwise or of all of its arguments. **Use `x1 | x2...` instead.**
---@param x1 number
---@param ... number
---@return number y
---
---@see bit.band - bitwise and.
---@see bit.xor - bitwise xor.
function bit.bor(x1, ...) end

---Returns the bitwise and of all of its arguments. **Use `x1 & x2...` instead.**
---@param x1 number
---@param ... number
---@return number y
---
---@see bit.bor - bitwise or.
---@see bit.xor - bitwise xor.
function bit.band(x1, ...) end

---Returns the bitwise xor of all of its arguments. **Use `x1 ~ x2...` instead.**
---@param x1 number
---@param ... number
---@return number y
---
---@see bit.band - bitwise and.
---@see bit.bor - bitwise or.
function bit.xor(x1, ...) end

---Returns the bitwise logical left-shift of its first argument by the number of bits given by the second argument. Logical shifts treat the first argument as an unsigned number and shift in 0-bits. **Use `x << n` instead.**
---@param x number Input number.
---@param n integer Number of bits to shift. Only the lower 5 bits of the shift count are used (reduces to the range \[0..31\]).
---@return number y
---
---@see bit.arshift - arithmetic right-shift.
---@see bit.rshift - logical right-shift.
function bit.lshift(x, n) end

---Returns the bitwise logical right-shift of its first argument by the number of bits given by the second argument. Logical shifts treat the first argument as an unsigned number and shift in 0-bits. **Use `x >> n` instead.
---@param x number Input number.
---@param n integer Number of bits to shift. Only the lower 5 bits of the shift count are used (reduces to the range \[0..31\]).
---@return number y
---
---@see bit.arshift - arithmetic right-shift.
---@see bit.lshift - logical left-shift.
function bit.rshift(x, n) end

---Returns the bitwise arithmetic right-shift of its first argument by the number of bits given by the second argument. Arithmetic right-shift treats the most-significant bit as a sign bit and replicates it.
---@param x number Input number.
---@param n integer Number of bits to shift. Only the lower 5 bits of the shift count are used (reduces to the range \[0..31\]).
---@return number y
---
---@see bit.lshift - logical left-shift.
---@see bit.rshift - logical right-shift.
function bit.arshift(x, n) end

---Returns the bitwise left rotation of its first argument by the number of bits given by the second argument. Bits shifted out on one side are shifted back in on the other side.
---@param x number Input number.
---@param n integer Number of bits to shift. Only the lower 5 bits of the shift count are used (reduces to the range \[0..31\]).
---@return number y
---
---@see bit.ror - bitwise right rotation.
function bit.rol(x, n) end

---Returns the bitwise right rotation of its first argument by the number of bits given by the second argument. Bits shifted out on one side are shifted back in on the other side.
---@param x number Input number.
---@param n integer Number of bits to shift. Only the lower 5 bits of the shift count are used (reduces to the range \[0..31\]).
---@return number y
---
---@see bit.rol - bitwise left rotation.
function bit.ror(x, n) end

---Swaps the bytes of its argument and returns it. This can be used to convert little-endian 32 bit numbers to big-endian 32 bit numbers or vice versa.
---@param x number
---@return number y
function bit.bswap(x) end
