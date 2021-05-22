const assert = require('assert');
const memoryjs = require('memoryjs');

const { handle, modBaseAddr: base } = memoryjs.openProcess('Warhammer2.exe');
assert(handle);

exports.base = base;

const printPointer = pointer => {
	let str = pointer.toString(16);
	if (str.length % 2 === 1) { str = `0${str}`; }
	return `0x${str}`;
};
exports.printPointer = printPointer;

const colour = {
	reset: '\x1b[0m',
	bright: '\x1b[1m',
	dim: '\x1b[2m',
	underscore: '\x1b[4m',
	blink: '\x1b[5m',
	black: '\x1b[30m',
	red: '\x1b[31m',
	green: '\x1b[32m',
	yellow: '\x1b[33m',
	blue: '\x1b[34m',
	magenta: '\x1b[35m',
	cyan: '\x1b[36m',
	white: '\x1b[37m',
	bg_black: '\x1b[40m',
	bg_red: '\x1b[41m',
	bg_green: '\x1b[42m',
	bg_yellow: '\x1b[43m',
	bg_blue: '\x1b[44m',
	bg_magenta: '\x1b[45m',
	bg_cyan: '\x1b[46m',
	bg_white: '\x1b[47m',
}
const define = struct => {
	assert(typeof struct.size !== 'undefined');
	class Struct {
		static name = struct.name;
		constructor(parent, pointer, ...args) {
			// console.log('new Struct', pointer)
			let cur = parent;
			while (cur) {
				if (cur._pointer === pointer) { return cur; }
				cur = cur._parent;
			}
			let entry = new struct(parent, pointer, ...args);
			Object.defineProperty(entry, '_pointer', {
				value: pointer,
				enumerable: false,
			});
			Object.defineProperty(entry, '_parent', {
				value: parent,
				enumerable: false,
			});
			entry.read(pointer, ...args);
			return entry;
		}
	}
	for (const key in struct) { Struct[key] = struct[key]; }
	const c = colour;
	Object.defineProperty(struct.prototype, 'toString', {
		value: function (stack) {
			if (!stack) { stack = []; }
			let ptr_name = `${c.red + c.bright}${printPointer(this._pointer)}${c.reset}`;
			for (let i = 0; i < stack.length; ++i) {
				if (stack[i] === this) { return `${ptr_name} *recursion*`; }
			}
			stack.push(this);
			let tab = '  '.repeat(stack.length);
			let output = `${ptr_name} { ${c.cyan}// ${struct.name}${c.reset}\n`;
			for (const key in this) {
				let v = this[key];
				// console.log('toString', key, v)
				if (typeof v === 'string') {
					v = `${c.green}${JSON.stringify(v)}${c.reset}`;
				} else if (typeof v === 'number') {
					v = `${c.yellow}${v}${c.reset}`;
				} else if (typeof v === 'boolean' || v === null) {
					v = `${c.blue}${v}${c.reset}`;
				} else if (v instanceof Array) {
					v = JSON.stringify(v);
				} else {
					v = v.toString(stack);
				}
				output += `${tab}${key}: ${v},\n`;
			}
			output += `${tab.substr(2)}}`;
			return output;
		},
		enumerable: false,
	});
	Object.defineProperty(struct.prototype, 'inspect', {
		value: function (stack) {
			return this.toString(stack)
		},
		enumerable: false,
	});
	return Struct;
};
exports.define = define;

const { Vector } = require('@basic/Vector');

const read_memory = (pointer, type) => memoryjs.readMemory(handle, pointer, type);
const read_buffer = (pointer, size) => memoryjs.readBuffer(handle, pointer, size);

const read_float = (pointer, offset) => read_memory(pointer + offset, memoryjs.FLOAT);
const read_pointer = (pointer, offset) => read_memory(pointer + offset, memoryjs.PTR);
const read_uint8 = (pointer, offset) => read_memory(pointer + offset, memoryjs.BYTE);
const read_int8 = (pointer, offset) => read_memory(pointer + offset, memoryjs.BYTE) - (1 << 7);
const read_uint16 = (pointer, offset) => read_memory(pointer + offset, memoryjs.SHORT) + (1 << 15);
const read_int16 = (pointer, offset) => read_memory(pointer + offset, memoryjs.SHORT);
const read_uint32 = (pointer, offset) => read_memory(pointer + offset, memoryjs.UINT32);
const read_int32 = (pointer, offset) => read_memory(pointer + offset, memoryjs.INT32);
const read_string = (pointer, offset, isPtr, isWide) => {
	pointer += offset;
	if (isPtr) {
		pointer = read_memory(pointer, memoryjs.PTR);
	}
	let size = read_memory(pointer, memoryjs.INT32);
	if (size === 0) { return ''; }
	if (size > 2048) { return null; }
	if (isWide) { size *= 2; }
	pointer = read_memory(pointer + 0x08, memoryjs.PTR);
	let buffer = read_buffer(pointer, size);
	return buffer.toString(isWide ? 'utf16le' : 'ascii');
};
const read_array = (parent, pointer, offset, struct, ...args) => {
	return new Vector(parent, pointer + offset, struct, ...args);
};
const read_ref = (parent, pointer, offset, struct, ...args) => {
	pointer = read_memory(pointer + offset, memoryjs.PTR);
	if (pointer === 0) { return null; }
	return new struct(parent, pointer, ...args);
};
const read_instance = (parent, pointer, offset, struct, ...args) => {
	return new struct(parent, pointer + offset, ...args);
};



const setRead = () => {
	exports.read_float = read_float;
	exports.read_pointer = read_pointer;
	exports.read_uint8 = read_uint8;
	exports.read_int8 = read_int8;
	exports.read_uint16 = read_uint16;
	exports.read_int16 = read_int16;
	exports.read_uint32 = read_uint32;
	exports.read_int32 = read_int32;
	exports.read_string = read_string;
	exports.read_array = read_array;
	exports.read_ref = read_ref;
	exports.read_instance = read_instance;
};
setRead();

let track;
const track_next = v => {
	let symbol = Symbol();
	v.symbol = symbol;
	track.push(v);
	return symbol;
};
const setReadDummy = () => {
	exports.read_float = (pointer, offset) => track_next({ type: 'float', pointer, offset });
	exports.read_pointer = (pointer, offset) => track_next({ type: 'pointer', pointer, offset });
	exports.read_uint8 = (pointer, offset) => track_next({ type: 'uint8', pointer, offset });
	exports.read_int8 = (pointer, offset) => track_next({ type: 'int8', pointer, offset });
	exports.read_uint16 = (pointer, offset) => track_next({ type: 'uint16', pointer, offset });
	exports.read_int16 = (pointer, offset) => track_next({ type: 'int16', pointer, offset });
	exports.read_uint32 = (pointer, offset) => track_next({ type: 'uint32', pointer, offset });
	exports.read_int32 = (pointer, offset) => track_next({ type: 'int32', pointer, offset });
	exports.read_string = (pointer, offset, isPtr, isWide) => track_next({ type: 'string', pointer, offset, isPtr, isWide });
	exports.read_array = (parent, pointer, offset, struct, ...args) => track_next({ type: 'array', parent, pointer, offset, struct, args });
	exports.read_ref = (parent, pointer, offset, struct, ...args) => track_next({ type: 'ref', parent, pointer, offset, struct, args });
	exports.read_instance = (parent, pointer, offset, struct, ...args) => track_next({ type: 'instance', parent, pointer, offset, struct, args });
};

const get_struct = (parent, struct, ...args) => {
	track = [];
	let entry = new struct(undefined, 0, ...args);
	for (const v of track) {
		for (const key in entry) {
			const symbol = entry[key];
			if (symbol === v.symbol) {
				v.name = key;
				break;
			}
		}
	}
	return track;
};
const get_struct_tree = (struct, ...args) => {
	setReadDummy();
	const tree = new Map();
	const output = [];
	const stack = [[undefined, struct, args]];
	while (stack.length > 0) {
		const [parent, struct, args] = stack.shift();
		const track = get_struct(parent, struct, ...args);
		tree.set(struct, track);
		output.push([struct, track]);
		for (const v of track) {
			let { struct } = v;
			if (!struct) { continue; }
			while (struct._wrapPtr) {
				struct = struct._wrapPtr;
			}
			if (tree.has(struct)) { continue; }
			stack.push([v.parent, struct, v.args]);
		}
	}
	setRead();
	output.reverse();
	return output;
};
exports.print = {
	cpp: function (struct, ...args) {
		const tree = get_struct_tree(struct, ...args);
		let output = [];
		for (const [struct, track] of tree) {
			let statementList = [];
			for (const v of track) {
				let { type } = v;
				if (type === 'float') {
					type = 'float';
				} else if (type === 'pointer') {
					type = 'LPVOID';
				} else if (type === 'string') {
					const { isWide, isPtr } = v;
					type = (isWide ? 'CA_wstring' : 'CA_string') + (isPtr ? '*' : '');
				} else if (v.struct) {
					let { struct } = v;
					let wrapPtr_depth = 0;
					while (struct._wrapPtr) {
						struct = struct._wrapPtr;
						++wrapPtr_depth;
					}
					let struct_name = struct.name + '*'.repeat(wrapPtr_depth);
					if (type === 'array') {
						type = `CA_array<${struct_name}>`;
					}
					else if (type === 'ref') {
						type = `${struct_name}*`;
					} else if (type === 'instance') {
						type = struct_name;
					}
				}
				statementList.push(`DEFINE_MEMBER_N(${type}, ${v.name}, ${printPointer(v.offset)})`);
			}
			output.push(`struct ${struct.name} {
	union {
		${statementList.join('\n\t\t')}
	}
}`)
		}
		return output.join('\n');
	}
}