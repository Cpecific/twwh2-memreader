import { Vector } from '@basic/Vector';
import { CWrapPtr } from '@basic/WrapPtr';

export class DB_DATA<T>{
	entries: Vector<CWrapPtr<T>>;
}
export class DB_TABLE<T>{
	db_data: DB_DATA<T>;
	*data(): Generator<InstanceType<T>, void, void>;
}

export function getTable<T>(idx: number, struct: T): DB_TABLE<T>;
export function getTable<T>(struct: T): DB_TABLE<T>;