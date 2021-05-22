const b = require('@basic');

// exports.LinkedEntry = b.define(class LinkedEntry {
// 	static size = NaN;
// 	read(pointer, struct, ...args) {
// 		this.data = b.read_instance(this, pointer, 0x10, struct, ...args);
// 	}
// })

exports.linkedList = struct => {
	return b.define(class LinkedList {
		static size = 0x30;
		read(pointer, ...args) {
			this.actual_size = b.read_int32(pointer, 0x00);
			// this.struct = struct;
			this.args = args;
		}
		*data() {
			let { actual_size, args } = this;
			if (actual_size === 0) { return; }
			let cur = b.read_pointer(this._pointer, 0x08);
			for (let i = 0; i < actual_size; ++i) {
				yield b.read_instance(this, cur, 0x10, struct, ...args);
				cur = b.read_pointer(cur, 0x00);
				if (cur === 0) { break; }
			}
		}
	});
}