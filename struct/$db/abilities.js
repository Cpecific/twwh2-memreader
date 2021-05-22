const b = require('@basic');
const { agent_ability_categories } = require('@db/agent_ability_categories');

exports.abilities = b.define(class abilities {
	static table_name = 'abilities_tables';
	static size = 56;
	read(pointer) {
		this.ability_key = b.read_string(pointer, 0x00, false, false);
		this.row_num = b.read_uint32(pointer, 0x10);
		this.on_screen_name = b.read_string(pointer, 0x18, true, true);
		this.on_screen_description = b.read_string(pointer, 0x20, true, true);
		this.category = b.read_ref(this, pointer, 0x28, agent_ability_categories);
		this.on_screen_target = b.read_string(pointer, 0x30, true, true);
	}
})