---@meta

---This connects the current world. Note that since connecting is "asynchronous" you are not actually connected when this command completes. A return value of `error_code.eOK` simply means that the connection process has successfully started.
---@return error_code code #
---`error_code.eWorldOpen`: World is already open.\
---`error_code.eOK`: Initiated connection.
---
---@see Disconnect - inverse.
---@see IsConnected - test whether the world is currently connected.
function Connect() end

---This disconnects from the current world. (ie. It disconnects from the MUD server).
---@return error_code code #
---`error_code.eWorldClosed`: World is already closed.\
---`error_code.eOK`: Disconnected OK.
---
---@see Connect - inverse.
---@see IsConnected - test whether the world is currently connected.
function Disconnect() end

---Gets the number of seconds the world has been connected.
---@return integer seconds Number of seconds, or 0 if the world is not currently connected.
---
---@see ResetStatusTime
function GetConnectDuration() end

---Returns an array which is a list of all the TCP/IP addresses which correspond to a given IP address, using DNS (Domain Name Server).
---
---Warning - because this function has to connect to a DNS server and await a response it may take some time to execute. It should not be used in a script where speed is the essence, or which is executed frequently. If you need to know the answer multiple times you should "cache" the result for future use.
---
---The reason an array is returned is because a single name might have multiple hosts (eg. Microsoft, Netscape) so a list of all of them is returned.
---@param hostName string
---@return string[] addresses List of IP addresses.
---
---@see GetHostName
function GetHostAddress(hostName) end

---This returns the name of the host which has a specified IP address, using DNS (Domain Name Server).
---
---Warning - because this function has to connect to a DNS server and await a response it may take some time to execute. It should not be used in a script where speed is the essence, or which is executed frequently. If you need to know the answer multiple times you should "cache" the result for future use.
---@param ipAddress string Address supplied as a "dotted decimal" string, like "66.36.226.56".
---@return string names Name corresponding to the supplied IP address, or an empty string if it could not be translated.
---
---@see GetHostAddress
function GetHostName(ipAddress) end

---Returns a count of the number of bytes received from the world.
---@return integer bytes Byte count.
---
---@see GetSentBytes
function GetReceivedBytes() end

---Returns a count of the number of bytes actually sent to the world. (This will exclude commands that are queued up that haven't yet been sent).
---@return integer bytes Byte count.
---
---@see GetReceivedBytes
function GetSentBytes() end

---Returns `true` if the client is fully connected to the MUD and false for any other phase, including network lookup.
---@return boolean connected
---
---@see Connect
---@see Disconnect
function IsConnected() end

---Returns the TCP/IP address of the current world.
---@return string address TCP/IP address, or an empty string if disconnected.
---
---@see WorldPort
function WorldAddress() end

---Returns the port number of the current world.
---@return integer port Port number, or 0 if disconnected.
---
---@see WorldAddress
function WorldPort() end
