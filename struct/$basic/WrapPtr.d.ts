class WrapPtr<T>{
	data: InstanceType<T>;
}
export interface CWrapPtr<T> {
	new(parent: any, pointer: number, struct: T, ...args: any[]): WrapPtr<T>;
}

export function wrapPtr<T>(struct: T): CWrapPtr<T>;