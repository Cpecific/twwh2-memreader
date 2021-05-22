class LinkedList<T>{
	actual_size: number;
	args: any[];
	*data(): Generator<InstanceType<T>, void, void>;
}
interface CLinkedList<T> {
	new(parent: any, pointer: number, offset: number, struct: T, ...args: any[]): LinkedList<T>;
}

export function linkedList<T>(struct: T): CLinkedList<T>;