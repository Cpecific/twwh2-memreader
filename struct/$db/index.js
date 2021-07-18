const assert = require('assert');
const path = require('path');
const fs = require('fs');
const b = require('@basic');
const { wrapPtr } = require('@basic/WrapPtr');

const DB_DATA = b.define(class DB_DATA {
	static size = 120;
	read(pointer, struct) {
		// console.log('DB_DATA.read', pointer, struct)
		this.entries = b.read_array(this, pointer, 0x08, wrapPtr(struct));
	}
})
const DB_TABLE = b.define(class DB_TABLE {
	static size = 56;
	read(pointer, struct) {
		// console.log('DB_TABLE.read', pointer, struct)
		this.db_data = b.read_ref(this, pointer, 0x00, DB_DATA, struct);
	}
	*data() {
		for (const entry of this.db_data.entries.data()) {
			yield entry.data;
		}
	}
})
exports.DB_DATA = DB_DATA;
exports.DB_TABLE = DB_TABLE;

let base = b.read_pointer(b.base, 0x03694798);
base = b.read_pointer(base, 0x20);
base = b.read_pointer(base, 0x10);
exports.base = base;

const tables = {};
// let output = [];
let ptr = base;
for (let idx = 0; idx < 1200; ++idx) {
	let p = b.read_pointer(ptr, 0x00);
	let tbl_name = b.read_string(p, 0x48, false, true);
	if (typeof tbl_name === 'string') {
		tables[tbl_name] = idx;
	}
	// output.push(typeof tbl_name === 'string' ? tbl_name : 'nil');
	ptr += 56;
}
// fs.writeFileSync(path.join(__dirname, '$tables2.txt'), output.join('\n'))

exports.getTable = (idx, struct) => {
	if (typeof idx !== 'number') {
		struct = idx;
		idx = tables[struct.table_name];
		assert(idx);
	}
	assert(struct.table_name);
	let pointer = base + 56 * idx;
	return b.read_instance(undefined, pointer, 0x00, DB_TABLE, struct);
}