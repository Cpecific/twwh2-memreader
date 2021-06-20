local assert, require, package, os, io = assert, require, package, os, io
local loadlib = package.loadlib

local _SEP = package.config:sub(1, 1)
local _WIN32 = (_SEP == '\\')
-- local _LINUX = (_SEP == '/')
-- assert(_WIN32)
-- assert(_LINUX)

local APP_VERSION = 1.1
local platform = 'Windows_NT-x64'
-- if _WIN32 then platform = 'Windows_NT-x64'
-- elseif _LINUX then platform = 'linux-x64' end
-- out('[cpecific] platform = '.. tostring(platform))

local filename = 'twwh2-memreader'


local status, mr = pcall(require, filename)
if not status or mr.version ~= APP_VERSION then
	local ext = '.dll'
	-- if _WIN32 then ext = '.dll'
	-- elseif _LINUX then ext = '.so' end
	package.loaded[ filename ] = nil -- the exact same thing as `debug.getregistry()._LOADED[ filename ]`
	if status and mr and type(mr) == 'table' then
		-- if on some other systems (ex: linux) line below won't work, then (I guess) the only way would be to
		-- traverse all registry keys and string.match by filename..ext
		-- as this lua sucker saves cpath filename, not a modname
		debug.getregistry()[ 'LOADLIB: .'.. _SEP .. filename .. ext ] = nil
		for k, v in next, mr do
			mr[k] = nil
		end
		mr = nil
	end
	-- don't want to test it out whether we need 2 gc. it just works
	local collectgarbage = collectgarbage
	collectgarbage()
	collectgarbage()
	-- we don't need to cache value of the file (don't use require)
	local data = assert(loadfile('/script/memreader/bin/'.. platform))()
	local file = assert(io.open(filename .. ext, 'wb'))
	file:write(data)
	file:close()

	package.loaded[ filename ] = nil
	mr = require(filename)
end

-- out('[cpecific] mr = '.. tostring(mr))
-- out('[cpecific] createtable = '.. tostring(mr.createtable))
-- local tbl = mr.createtable(10, 0)
-- out('[cpecific] tbl = '.. tostring(tbl))

_G.memreader = mr