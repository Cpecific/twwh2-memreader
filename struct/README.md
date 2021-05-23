# Structure definitions
This directory contains decoded game structures.\
Decided not to write all decoded structures in js, because current project is crude. Feel free to suggest on how to improve basic functionaility.\
If you want to contribute, then only publish js code, don't publish Reclass.net project file (I will sync the changes when I have free time). It would be good to make Reclass.net project file generation from js, but it seems too much of an effort (from quick REClass.net source code inspection).\
Alternatively you can [use lua inside of Cheat Engine](https://wiki.cheatengine.org/index.php?title=Lua).
```lua
-- Just add this line at the top to clear output every time you execute the script
getLuaEngine().Menu.Items[0][5].doClick(); -- clear output
```
BTW Reclass.net is atrocious piece of garbage. Performance in large project such as this is terrible. Don't even try pressing "Enable hierarchy view". Don't use enums, they are god awful, you can't even delete them without being a pain in the ass. If you want to know where the class is used, then you are in a hard spot (can't use hierarchy view; alternatively you can use "Generate C++ code"), so I would strongly suggest to always comment at the top of your new structure where it can be found (didn't do it personally for some structures, and I was in a lot of agonizing pain after that). Be careful, this piece of shit won't even promt you to save changes when you exit the program. Authors of this shit program are a bunch of retards...

If you are going to decode memory structures, then you should learn to immediately recognise some common structures.
> Source: ["Auto-Padding" for class recreation (by Public_Void)](https://guidedhacking.com/threads/auto-padding-for-class-recreation.13478/#post-79838)
```h
#define STR_MERGE_IMPL(a, b) a##b
#define STR_MERGE(a, b) STR_MERGE_IMPL(a, b)
#define MAKE_PAD(size) STR_MERGE(_pad, __COUNTER__)[size]
#define DEFINE_MEMBER_N(type, name, offset) struct {unsigned char MAKE_PAD(offset); type name;}
```
### Strings
```h
// @string, @unicode
struct CA_String {
	union {
		DEFINE_MEMBER_N(int32, length, 0x00);
		DEFINE_MEMBER_N(int32, capacity, 0x04);
		DEFINE_MEMBER_N(char*, data, 0x08);
	}
}
```
### Arrays
```h
struct CA_Array<T> {
	union {
		DEFINE_MEMBER_N(int32, capacity, 0x00);
		DEFINE_MEMBER_N(int32, length, 0x04);
		DEFINE_MEMBER_N(T*, data, 0x08);
	}
}
```
### Linked list
This suckers have hash part. CA uses this all over da place.\
You really need to recognise these structures at first glance, otherwise you will waste too much time following pointers.\
`head->prev == nullptr`, `tail->next == CA_Shit_List<T>` (be careful, when traversing with scripts).\
If `actual_size == 0`, then `head == tail == CA_Shit_List<T>; capacity == size == 2`.\
Size of `hash` part is always power of 2.\
It is easy to recognise if you are inside of `hash` part. You will see continious array of points, and some of these pointers will be repeated (consequtively). And, of course, you would need to look whether it's items are of `CA_Shit_Member<T>` type.\
If you want to iterate over linked list, you should go from `tail` to the `head`. `head` always has `prev == nullptr`.\
If you are inside `CA_Shit_Member<T>` and want to find you where the fuck is `CA_Shit_List<T>`, follow `next`, it will bring you to `CA_Shit_List<T>`. Simple condition `if (cur->next.prev != cur) { found the sucker }`.
```h
struct CA_Shit_Member<T> {
	union {
		DEFINE_MEMBER_N(CA_Shit_Member<T>*, prev, 0x00);
		DEFINE_MEMBER_N(CA_Shit_Member<T>*, next, 0x08);
		DEFINE_MEMBER_N(T, data, 0x10);
	}
}
// sizeof = 0x30 = 48
struct CA_Shit_List<T> {
	union {
		DEFINE_MEMBER_N(int32, actual_size, 0x00);
		DEFINE_MEMBER_N(CA_Shit_Member<T>*, prev, 0x08); // tail
		DEFINE_MEMBER_N(CA_Shit_Member<T>*, next, 0x10); // head
		DEFINE_MEMBER_N(int32, capacity, 0x18);
		DEFINE_MEMBER_N(int32, size, 0x1C);
		DEFINE_MEMBER_N(CA_Shit_Member<T>**, hash, 0x20);
		DEFINE_MEMBER_N(float, some_number, 0x28); // I've only seen value 1.0
	}
}
```

## Static pointers
### Database table list
*-- Only tested in frontend and campaign!*
```js
// Points to the very first db table
// They are not continious, some (DB_DATA) are overwritten with junk
// [[[<Warhammer2.exe>+03601F98]+20]+10]
// [[[base + 03601F40 + 50 + 08]+20]+10]
const b = require('@basic')
const uic_db = require('@static/uic_db')
let ptr = b.read_pointer(b.base, 0x03601F40) // @static/uic_db.base
let uic_db_entry = b.read_instance(undefined, ptr, 0x00, uic_db.static_uic_db)
for (const entry of uic_db_entry.uic_list.data()) { // uic_list: 0x50; prev: 0x08
	// prev(0): idx = 0
	if (entry.uic_path === 'ui\\common ui\\tab_completer') {
		let ptr = entry.uic._pointer // @uic.UIC: 0x20
		ptr = b.read_pointer(ptr, 0x10) // @db.DB_TABLE
		break;
	}
}
```
#### LUA
```lua
-- UICreated @event
-- Could also find where this `tab_completer` hides among root children
local ptr = mr.base
ptr = read_pointer(ptr, '\152\31\96\3') -- 0x03601F98
ptr = read_pointer(ptr, 0x20)
ptr = read_pointer(ptr, 0x10)
```

### Current selected character (+hover)
```js
// There are several other UICs that point to current character.
// Some of the point to actual selected character (not hover).
// Would be pain in the ass to track them with pointer scan (did it, not fun).
// Better to programmatically do it with js over all children of root.
// [[[[[<Warhammer2.exe>+03737E08]+18]+50]+08]]
// [[[[[base + 03737E00 + 08]+18]+50]+08]]
const b = require('@basic')
const { wrapPtr } = require('@basic/WrapPtr')
const { UIC } = require('@uic')
let ptr = b.base + 0x03737E00
let arr = b.read_array(undefined, ptr, 0x00, wrapPtr(UIC))
for (let uic of arr.data()) { // items[]: 0x08
	uic = uic.data
	// 0x18: idx = 4
	if (uic.name === 'war_coordination_buttonset') {
		let ptr = uic._pointer
		ptr = b.read_pointer(ptr, 0x50) // UIC.cco_selected? => $uic__cco_selected?
		ptr = b.read_pointer(ptr, 0x08) // @character.CharacterDetails
		ptr = b.read_pointer(ptr, 0x00) // @character.Character
		let cqi = b.read_int32(ptr, 0xF0)
		break;
	}
}
```
#### LUA
```lua
-- Does it already exist in UICreated? Does it get recreated at some conditions?
local CA_cip = root:SequentialFind(
	'layout',
	'info_panel_holder',
	'primary_info_panel_holder',
	'info_panel_background',
	'CharacterInfoPopup')
local ptr = ud_topointer(CA_cip)
ptr = read_pointer(ptr, 0x00) -- @uic.UIC
ptr = read_pointer(ptr, 0x50) -- cco_selected
ptr = read_pointer(ptr, 0x08) -- @character.CharacterDetails
ptr = read_pointer(ptr, 0x00) -- @character.Character
local cqi = read_int32(ptr, 0xF0)
```
