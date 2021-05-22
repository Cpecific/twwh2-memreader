# Lua Memory Reader (TWWH2)
`memreader` is Lua module for reading memory of current process.\
Big kudos to [squeek502/memreader](https://github.com/squeek502/memreader)\
Designed specificially for Total War Warhammer 2.\
Only works on Windows x64.
```lua
local mr = assert(_G.memreader)
local ptr = mr.base -- ex: 0x0000000140000000
out(mr.tostring(ptr)) -- 140000000
-- read pointer at address (base + 0x03601F98)
ptr = mr.read_pointer(ptr, '\152\31\96\3') -- 0x03601F98 (uint32)
-- if (ptr == NULL)
if mr.eq(ptr, '\0\0\0\0\0\0\0\0') then return end
-- read value at address (ptr + 0x14)
local luaNumber = mr.read_int32(ptr, 0x14)
-- in Warhammer lua_Number is float. Precision drops after 0x00FFFFFF (3 bytes).
local rawNumber = mr.read_int32(ptr, 0x14, true) -- returns userdata
assert(luaNumber == mr.tonumber(rawNumber)) -- conversion to lua_Number
luaNumber = luaNumber + 0x0150
-- !!! DO NOT USE BASIC LUA OPERATORS ON RETURNED userdata !!!
-- !!! USE PROVIDED LIBRARY FUNCTIONS INSTEAD              !!!
rawNumber = mr.add(rawNumber, 0x0150) -- also: sub, mult, div
-- assert(luaNumber == mr.tonumber(rawNumber)) -- could be false due to precision loss?

-- (struct_String) int32 size; int32 capacity; char *string;
local string = mr.read_string(ptr, 0x20, false, false) -- ex: Adam
-- pointer to struct_String
local string = mr.read_string(ptr, 0x30, true, false) -- ex: Adam
-- (struct_WString) int32 size; int32 capacity; wchar_t *string;
local wstring = mr.read_string(ptr, 0x40, false, true) -- ex: A\0d\0a\0m\0

-- (struct_Array) int32 capacity; int32 size; LPVOID ptr;
local size, pdata = mr.read_array(ptr, 0x50)
-- preallocate table with narr=size (raw array part), nrec=0 (hash part)
local entries = mr.createtable(size, 0)
for i = 1, size do
	-- advanced use-case example: 
	-- trait_categories__base = pointer to first entry. each entry has size 32.
	-- let's say you copy all this data into table tcdata = { first_entry, ... }
	-- pdata+0x00 points to one of such entries.
	-- read_rowidx returns idx of that entry (tcdata[ idx ]).
	local idx = read_rowidx(pdata, 0x00, trait_categories__base, 32) -- sizeof(trait_categories)
	local entry = tcdata[ idx ]
	pdata = mr.add(pdata, 0x08) -- sizeof(LPVOID)
end

-- probably you won't need this functionality at all
mr.write(ptr, 0x0100, false) -- boolean (1 byte)
mr.write(ptr, 0x0100, 165.48) -- float
mr.write(ptr, 0x0100, 'das\0\0\0\1\2\89fuw') -- array of bytes
mr.write(ptr, 0x0100, mr.pointer('\0\0\0\64\1\0\0\0')) -- pointer 0x0000000140000000
mr.write(ptr, 0x0100, mr.uint32('\0\0\0\64')) -- uint32 0x40000000
mr.write(ptr, 0x0100, mr.uint16(0x4000)) -- uint16 0x4000
mr.write(ptr, 0x0100, mr.uint8(0x40)) -- uin8 0x40

-- but this one is quite handy
local character = get_character_by_cqi(cqi)
local chptr = mr.ud_topointer(character)
tostring(character) -- ex: CHARACTER(0x000000004EB62210)
mr.tostring(chptr) -- ex: 4EB62210
-- mr.ud_debug would return pointer to internal lua gc value.
-- you could use it for other stuff, if you want more control over process.
-- after that you can do
chptr = mr.read_pointer(chptr, 0x10)
-- and now you have access to $pHero structure
```

# Installation
```
npm install
npm run create-symlink-windows
```


# Disclaimer
I'm not a fan of metatables, but of simple and efficient code.\
So you will not be able to do fancy stuff like:
```lua
local ptr = base + 32 * idx
local diff = ptr - base
```
And would have to resort to a more "ugly" approach:
```lua
local ptr = add(base, 32 * idx)
local diff = sub(ptr, base)
```
Why no Linux or Mac support?\
On Linux Lua was not configured with `LUA_DL_DLOPEN` enabled, therefore, `package.loadlib` and `require(cpath)` do not work.\
`io.popen` doesn't work as well (didn't check `os.execute`).\
On Mac any `file:write` functionality is broken (disabled?).\
If `os.execute` and `file:write` were present, then, theoretically, it would be possible to inject dll. But that is waaay too much work for mods.\
Better just nag at CA to enable this functionality.



# API Reference
Notation `typeA:typeB` stands for: pass `typeA` value, library will treat (cast) it as `typeB` value.\
Don't confuse with `argName: type`.\
#### `pointer(string:LPVOID): pointer`
#### `pointer(pointer): pointer`
Returns new userdata pointer. String conversion requires sizeof(LPVOID)=8 bytes of data.
#### `uint8(float): uint8`
#### `int8(float): int8`
#### `uint16(float): uint16`
#### `int16(float): int16`
#### `uint32(float): uint32`
#### `int32(float): int32`
Returns new userdata uint8..int32
#### `uint32(string:UINT32): uint32`
#### `int32(string:UINT32): int32`
String conversion requires sizeof(UINT32)=4 bytes of data.

## Addition +
#### `add(float, float): float`
#### `add(float, uint8..int32): float`
#### `add(pointer, float:UINT32): pointer`
#### `add(pointer, string:UINT32): pointer`
#### `add(pointer, uint8..int32): pointer`
#### `add(uint8..int32, float:UINT32): uint8..int32`
#### `add(uint8..int32, string:UINT32): uint8..int32`
#### `add(uint8..int32, uint8..int32): uint8..int32`

## Subtraction -
#### `sub(float, float): float`
#### `sub(float, uint8..int32): float`
#### `sub(pointer, float:UINT32): pointer`
#### `sub(pointer, string:UINT32): pointer`
#### `sub(pointer, pointer): pointer`
#### `sub(pointer, uint8..int32): pointer`
#### `sub(uint8..int32, float:UINT32): uint8..int32`
#### `sub(uint8..int32, string:UINT32): uint8..int32`
#### `sub(uint8..int32, uint8..int32): uint8..int32`

## Multiplication *
#### `mult(float, float): float`
#### `mult(float, uint8..int32): float`
#### `mult(uint8..int32, float:UINT32): uint8..int32`
#### `mult(uint8..int32, string:UINT32): uint8..int32`
#### `mult(uint8..int32, uint8..int32): uint8..int32`

## Division /
#### `div(float, float): float`
#### `div(float, uint8..int32): float`
#### `div(pointer:ptrdiff_t, float:UINT32): pointer`
#### `div(pointer:ptrdiff_t, string:UINT32): pointer`
#### `div(pointer:ptrdiff_t, uint8..int32): pointer`
#### `div(uint8..int32, float:UINT32): uint8..int32`
#### `div(uint8..int32, string:UINT32): uint8..int32`
#### `div(uint8..int32, uint8..int32): uint8..int32`

## Comparison
#### `eq(pointer, string:LPVOID): boolean`
#### `eq(pointer, pointer): boolean`
#### `eq(uint32..int32, float:UINT32): boolean`
#### `eq(uint32..int32, string:UINT32): boolean`
#### `eq(uint32..int32, uint32..int32): boolean`
#### `eq := ==`
#### `lt := <`
#### `gt := >`
`eq`, `lt` and `gt` have the exact same function declarations.


## Reading
#### `read_float(pointer, [offset]): float`
#### `read_pointer(pointer, [offset]): pointer`
#### `read_uint8(pointer, [offset]): float`
#### `read_uint8(pointer, [offset], return_userdata: boolean): uint8`
#### `same function declarations for int8..int32`
If you pass `return_userdata=true`, then function will return userdata uint8..int32, instead of casting it to float\
Optional argument `offset` can be passed with types:\
`nil=0`\
`float:UINT32`\
`string:UINT32`\
`uint32..int32`
#### `read_boolean(pointer, [offset]): boolean`
Will return `true` if `byte ~= 0x00`
#### `read_string(pointer, [offset], isPtr: boolean, isWide: boolean): string`
Structure for string `{ INT32 size; INT32 capacity; char *pStr; }`\
`isWide=true` will return wchar_t raw data (not very userful in Lua)\
If you pass `isPtr=true`, then library will do the following:
```lua
local ptr = read_pointer(ptr, offset)
return read_string(ptr, 0, false, isWide)
```
#### `read_array(pointer, [offset]): float, pointer`
#### `read_array(pointer, [offset], return_userdata: boolean): int32, pointer`
Structure for array `{ INT32 capacity; INT32 size; LPVOID pData; }`\
Returns size of array and pointer to data.

#### `read_rowidx(pEntry: pointer, [offset], base: pointer, entry_size: float): float`
Basically library does the following:
```lua
local entry = read_pointer(pEntry, offset)
return 1 + (entry - base) / entry_size
```
### `read(pointer, [offset], bytes: float:UINT32): string`
Reads `bytes` from memory. String contains raw data. It is not null terminated.


## Writing
#### `write(pointer, [offset], boolean)`
#### `write(pointer, [offset], float)`
#### `write(pointer, [offset], string)`
#### `write(pointer, [offset], pointer)`
#### `write(pointer, [offset], uint8..int32)`
Write functionality hasn't been tested yet.


## Modules
#### `modules(): { base: pointer, size: float, name: string, path: string }`
```lua
for module in mr.modules() do
	out('base = '.. mr.tostring(module.base) ..', size = '.. tostring(module.size) ..', name = '.. module.name ..', path = '.. module.path)
end
```


## Misc: type
#### `type(nil): 'nil'`
#### `type(boolean): 'boolean'`
#### `type(float): 'float'`
#### `type(string): 'bytes'`
#### `type(pointer): 'pointer'`
#### `type(uint8..int32): 'uint8'..'int32'`

## Misc: tostring
#### `tostring(nil | boolean | number): tostring`
Behaves same as default lua tostring function.
#### `tostring(string): string`
Returns hex representation of string.
#### `tostring(pointer): string // %p`
#### `tostring(uint8): string // %hhu`
#### `tostring(int8): string // %hhd`
#### `tostring(uint16): string // %hu`
#### `tostring(int16): string // %hd`
#### `tostring(uint32): string // %u`
#### `tostring(int32): string // %d`
Refer to `sprintf_s` documentation.

## Misc: tonumber
#### `tonumber(float:UINT32): float`
#### `tonumber(string:UINT32): float`
#### `tonumber(pointer): float`
#### `tonumber(uint32..int32): float`

## Misc: userdata
#### `ud_topointer(userdata): pointer`
Returns pointer of userdata value.
```lua
local chptr = ud_topointer(character)
chptr = read_pointer(chptr, 0x10)
local cqi = read_int32(chptr, 0xF0)
```
#### `ud_debug(userdata): float, pointer`
Returns lua type of value (TValue.tt) and pointer value `p` (TValue.value.p). Refer to lua documentation and source code.

## Misc: createtable
#### `createtable(narr, nrec): table`
Creates new table with preallocated space narr (raw array part) and nrec (hash part)\
`narr` and `nrec` can be the following types:\
`float:UINT32`\
`string:UINT32`\
`uint32..int32`
