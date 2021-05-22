const b = require('@basic');

exports.Vector = b.define(class Vector {
	static size = 0x10;
	read(pointer, struct, ...args) {
		this.size = b.read_int32(pointer, 0x04);
		this.data_pointer = b.read_pointer(pointer, 0x08);
		this.struct = struct;
		this.args = args;
	}
	*data() {
		let { size, data_pointer, struct, args } = this;
		for (let i = 0; i < size; ++i) {
			if (struct) {
				yield b.read_instance(this, data_pointer, 0x00, struct, ...args);
				data_pointer += struct.size;
			} else {
				yield b.read_pointer(data_pointer, 0x00);
				data_pointer += 8; // sizeof(LPVOID)
			}
		}
	}
})