import { IEntry } from '@basic';

export class Vector<T> extends IEntry {
	size: number;
	data_pointer: number;
	struct: InstanceType<T>;
	args: any[];
	*data(): Generator<InstanceType<T>, void, void>;
}