const b = require('@basic');
const { DB_TABLE, getTable } = require('@db');

const { abilities } = require('@db/abilities');
let table = getTable(abilities);
for (const entry of table.data()) {
	console.log('' + entry)
}
// console.log(b.print.cpp(DB_TABLE, abilities))

const uic_db = require('@static/uic_db');
let uic_db_entry = b.read_instance(undefined, uic_db.base, 0x00, uic_db.static_uic_db);
console.log('' + uic_db_entry)
for (const entry of uic_db_entry.uic_list.data()) {
	console.log('' + entry)
}
// console.log(b.print.cpp(uic_db.static_uic_db))

// const { Character } = require('@character');
// let char = b.read_instance(undefined, 0x470DDC50, 0x00, Character);
// console.log('' + char)
// console.log(b.print.cpp(Character))