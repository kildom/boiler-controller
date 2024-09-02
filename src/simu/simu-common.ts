

export interface ModelChannel {
    init(): Promise<void>;
    send(data: Uint8Array): void;
    onReceive?: null | ((data: Uint8Array) => void);
    commSend(data: Uint8Array): void;
    onCommReceive?: null | ((data: Uint8Array) => void);
}

export interface WorkerModelEvent {
    type: 'model' | 'comm';
    data: Uint8Array;
}

export interface SerialInterface {
    send(data: Uint8Array): void;
    onReceive?: (data: Uint8Array) => void;
}

export class AsyncSignal<T> {

    private state: 'idle' | 'resolved' | 'waiting' = 'idle';
    private rejected: boolean = false;
    private resolvedValue?: any;
    private resolveFunc?: (value: T | PromiseLike<T>) => void;
    private rejectFunc?: (reason?: any) => void;

    public wait(): T | Promise<T> {
        if (this.state === 'resolved') {
            this.state = 'idle';
            if (this.rejected) {
                throw this.resolvedValue;
            } else {
                return this.resolvedValue;
            }
        } else {
            this.state = 'waiting';
            return new Promise<T>((resolve, reject) => {
                this.resolveFunc = resolve;
                this.rejectFunc = reject;
            });
        }
    }

    public signal(value: T): void {
        let resolveFunc = this.resolveFunc;
        this.resolveFunc = undefined;
        this.rejectFunc = undefined;
        if (resolveFunc) {
            resolveFunc(value);
        } else {
            this.resolved = true;
            this.resolvedValue = value;
        }
    }

    public error(reason?: any): void {
        let rejectFunc = this.rejectFunc;
        this.resolveFunc = undefined;
        this.rejectFunc = undefined;
        if (rejectFunc) {
            rejectFunc(value);
        } else {
            this.resolved = true;
            this.resolvedValue = value;
        }
    }
}

export function fatal(message: string): never {
    // TODO: show fatal error in GUI
    throw new Error(message);
}


export function assert(value: unknown, message: string): asserts value {
    if (!value) {
        fatal(message);
    }
}

export function bufferEqual(a: Uint8Array, b: Uint8Array) {
    if (a.length !== b.length) return false;
    if (typeof indexedDB !== 'undefined') {
        return indexedDB.cmp(a, b) === 0;
    } else {
        for (let i = 0; i < a.length; i++) {
            if (a[i] !== b[i]) return false;
        }
        return true;
    }
}

export function wait(time: number): Promise<void> {
    return new Promise(r => setTimeout(r, time));
}

export function concatenateBuffers(list: Uint8Array[]): Uint8Array {
    let size = list.reduce((a, b) => a + b.length, 0);
    let result = new Uint8Array(size);
    let offset = 0;
    for (let a of list) {
        result.set(a, offset);
        offset += a.length;
    }
    return result;
}

export enum SimulationMode {
    SOFTWARE,
    HARDWARE,
};

export function getSimulationMode(): undefined | SimulationMode {
    if (window.location.hash === '#hw') {
        return SimulationMode.HARDWARE;
    } else if (window.location.hash === '#sw') {
        return SimulationMode.SOFTWARE;
    } else {
        return undefined;
    }
}

