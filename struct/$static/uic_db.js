const b = require('@basic');
const { linkedList } = require('@basic/LinkedList');
const { UIC } = require('@uic');

let ptr = b.base + 0x03601F40;
exports.base = ptr;

const uic_db_entry = b.define(class uic_db_entry {
	static size = 0x10;
	read(pointer) {
		this.uic_path = b.read_string(pointer, 0x00, false, false);
		this.uic = b.read_ref(this, pointer, 0x10, UIC);
	}
})
exports.static_uic_db = b.define(class static_uic_db {
	static size = NaN;
	read(pointer) {
		this.uic_list = b.read_instance(this, pointer, 0x50, linkedList(uic_db_entry));
	}
});