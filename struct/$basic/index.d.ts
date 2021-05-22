import { Vector } from '@basic/Vector';

export function printPointer(pointer: number): string;

export class IEntry {
	constructor(parent: any, pointer: number, offset: number, ...args: any[]);
}
export function define<T>(struct: T): T;

export function read_float(pointer: number, offset: number): number;
export function read_pointer(pointer: number, offset: number): number;
export function read_uint8(pointer: number, offset: number): number;
export function read_int8(pointer: number, offset: number): number;
export function read_uint16(pointer: number, offset: number): number;
export function read_int16(pointer: number, offset: number): number;
export function read_uint32(pointer: number, offset: number): number;
export function read_int32(pointer: number, offset: number): number;
export function read_string(pointer: number, offset: number, isPtr: boolean, isWide: boolean): string;
export function read_array<T>(parent: any, pointer: number, offset: number, struct: T, ...args: any[]): Vector<T>;
export function read_ref<T>(parent: any, pointer: number, offset: number, struct: T, ...args: any[]): InstanceType<T> | null;
export function read_instance<T>(parent: any, pointer: number, offset: number, struct: T, ...args: any[]): InstanceType<T>;