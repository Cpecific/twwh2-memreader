const b = require('@basic');

exports.UIC = b.define(class UIC {
	static size = NaN;
	read(pointer) {
		this.name = b.read_string(pointer, 0x0150, false, false);
	}
});