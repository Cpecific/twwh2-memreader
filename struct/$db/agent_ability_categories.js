const b = require('@basic');

exports.agent_ability_categories = b.define(class agent_ability_categories {
	static table_name = 'agent_ability_categories_tables';
	static size = 24;
	read(pointer) {
		this.on_screen_name = b.read_string(pointer, 0x00, true, true);
		this.category = b.read_string(pointer, 0x08, false, false);
	}
})