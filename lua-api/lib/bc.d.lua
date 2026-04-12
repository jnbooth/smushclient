---@meta

---lbc: A simple arbitrary-precision library for Lua based on GNU bc.
---
---From https://web.tecgraf.puc-rio.br/~lhf/ftp/lua/index.html.
---@class bc: userdata
---@operator add(bc|number|string): bc
---@operator div(bc|number|string): bc?
---@operator mod(bc|number|string): bc?
---@operator mul(bc|number|string): bc
---@operator pow(bc|number|string): bc
---@operator sub(bc|number|string): bc
---@operator unm: bc
bc = {}

---@type string
bc.version = bc.version

function bc.cleanup() end

---@param a bc|number|string
---@return bc
function bc.abs(a) end

---@param a bc|number|string
---@param b bc|number|string
---@return bc
function bc.add(a, b) end

---@param a bc|number|string
---@param b bc|number|string
---@return -1 | 0 | 1
function bc.compare(a, b) end

---@param setting? integer
---@return integer setting
function bc.digits(setting) end

---@param a bc|number|string
---@param b bc|number|string
---@return bc?
function bc.div(a, b) end

---@param a bc|number|string
---@param b bc|number|string
---@return bc? quot
---@return bc? rem
function bc.quotrem(a, b) end

---@param a bc|number|string
---@return boolean
function bc.isneg(a) end

---@param a bc|number|string
---@return boolean
function bc.iszero(a) end

---@param a bc|number|string
---@param b bc|number|string
---@return bc?
function bc.mod(a, b) end

---@param a bc|number|string
---@param b bc|number|string
---@return bc
function bc.mul(a, b) end

---@param a bc|number|string
---@return bc
function bc.neg(a) end

---@param a bc|number|string
---@return bc
function bc.new(a) end

---@param a bc|number|string
---@param b bc|number|string
---@return bc
function bc.pow(a, b) end

---@param a bc|number|string
---@param k bc|number|string
---@param m bc|number|string
---@return bc?
function bc.powmod(a, k, m) end

---@param a bc|number|string
---@return bc?
function bc.sqrt(a) end

---@param a bc|number|string
---@param b bc|number|string
---@return bc
function bc.sub(a, b) end

---@param a bc
---@return number
function bc.tonumber(a) end

---@param a bc
---@return string
function bc.tostring(a) end

---@param a bc|number|string
---@param n? integer
---@return bc
function bc.trunc(a, n) end
