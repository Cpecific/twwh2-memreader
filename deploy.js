const path = require('path');
const fs = require('fs');

// const __dirname = path.resolve();
const build_dir = path.join(__dirname, 'twwh2-memreader', 'out/build');

const filenameList = [
	// [path.join(build_dir, 'WSL-Release', 'libtwwh2_memreader.so'), 'linux-x64', 'so'],
	[path.join(build_dir, 'x64-Release', 'twwh2_memreader.dll'), 'Windows_NT-x64', 'dll'],
];
const outputFolder = path.join(__dirname, 'script/memreader/bin');
if (!fs.existsSync(outputFolder)) {
	fs.mkdirSync(outputFolder, { recursive: true });
}

const chEscape = {
	[0x0A]: '\\n',
	[0x0D]: '\\r',

	[0x22]: '\\"',
	[0x5C]: '\\\\',
}
for (let [filepath, platform, extension] of filenameList) {
	const buffer = fs.readFileSync(filepath);
	let output = 'return "';
	for (let i = 0; i < buffer.length; ++i) {
		const byte = buffer[i];
		const ch = String.fromCharCode(byte);
		if (chEscape[byte]) { output += chEscape[byte]; }
		// else { output += ch; }
		// RPFM utf-8 conversion unwanted behaviour
		else if (byte <= 127) { output += ch; }
		else { output += '\\' + byte; }
	}
	output += '"';

	// const filename = `lmr-${path.parse(filepath).name}`;
	fs.writeFileSync(path.join(outputFolder, `${platform}.lua`), output, { encoding: 'ascii' });
	if (extension === 'dll') {
		const filename = `twwh2-memreader`;
		const rmFile = path.join(`E:/Steam/steamapps/common/Total War WARHAMMER II/${filename}.${extension}`);
		if (fs.existsSync(rmFile)) { fs.rmSync(rmFile); }
	}
}