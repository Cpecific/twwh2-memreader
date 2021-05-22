const b = require('@basic');

exports.wrapPtr = struct => {
	return b.define(class Ptr {
		static size = 0x08;
		static name = `Ptr<${struct.name}>`;
		static _wrapPtr = struct;
		read(pointer, ...args) {
			this.data = b.read_ref(this, pointer, 0x00, struct, ...args);
		}
	});
}