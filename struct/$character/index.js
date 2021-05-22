const b = require('@basic');

let R = {} // for js intellisense
R.CharacterDetails = b.define(class CharacterDetails {
	static size = NaN;
	read(pointer) {
		this.self = b.read_ref(this, pointer, 0x00, R.Character);
		this.name = b.read_string(pointer, 0x0420 - 0x03C8,true, true);
		this.available_points = b.read_int32(pointer, 0x0610 - 0x03C8);
		this.total_points = b.read_int32(pointer, 0x0614 - 0x03C8);
		this.current_xp = b.read_int32(pointer, 0x061C - 0x03C8);
		this.self_details = b.read_ref(this, pointer, 0x0638 - 0x03C8, R.CharacterDetails);
	}
})
R.Character = b.define(class Character {
	static size = NaN;
	read(pointer) {
		this.cqi = b.read_int32(pointer, 0xF0);
		this.details = b.read_instance(this, pointer, 0x03C8, R.CharacterDetails);
	}
})
exports.CharacterDetails = R.CharacterDetails;
exports.Character = R.Character;