import { WorkerModelEvent } from "./simu-common";

const decoder = new TextDecoder();

interface WebAssemblyExports {
    memory: WebAssembly.Memory;
    _initialize(): void;
    commRecv(ptr: number, size: number): void;
    modelRecv(ptr: number, size: number): void;
    malloc(size: number): number;
    free(pre: number): number;
};

interface WebAssemblyImports {
    env: {
        storeRead(slot: number, buffer: number, size: number): void;
        storeWrite(state: number, slot: number, buffer: number, size: number): number;
        commSend(data: number, size: number): void;
        modelSend(data: number, size: number): void;
        time(): number;
        log(level: number, messageOffset: number): void;
    };
    wasi_snapshot_preview1: {
        fd_close(): number;
        fd_seek(): number;
        fd_write(): number;
    }
};

let wasmModule: WebAssembly.Module;
let model: WebAssembly.Instance;
let wasmExports: WebAssemblyExports;
let startTime = Date.now();

let wasiStubs = {
    fd_close() {
        return 0;
    },
    fd_seek() {
        return -1;
    },
    fd_write() {
        return -1;
    }
}

function msg(message: WorkerModelEvent) {
    postMessage(message);
}

function commSend(dataOffset: number, size: number): void {
    let data = new Uint8Array(wasmExports.memory.buffer, dataOffset, size).slice();
    msg({ type: 'comm', data });
};

function modelSend(dataOffset: number, size: number): void {
    let data = new Uint8Array(wasmExports.memory.buffer, dataOffset, size).slice();
    msg({ type: 'model', data });
};

function time(): number {
    return (Date.now() - startTime) & 0x7FFFFFFF;
}

function log(level: number, messageOffset: number): void {
    let data = new Uint8Array(wasmExports.memory.buffer, messageOffset, Math.min(65536, wasmExports.memory.buffer.byteLength - messageOffset));
    let length = 0;
    while (data[length] !== 0 && length < data.length) {
        length++;
    }
    let message = decoder.decode(new Uint8Array(wasmExports.memory.buffer, messageOffset, length));
    switch (level) {
        case 0:
            console.debug(message);
            break;
        case 1:
            console.info(message);
            break;
        default:
            console.error(message);
            break;
    }
};

let bufferOffset = 0;
let bufferSize = 0;

function getBuffer(size: number) {
    if (bufferSize < size || bufferSize === 0) {
        if (bufferOffset !== 0) {
            wasmExports.free(bufferOffset);
        }
        bufferOffset = wasmExports.malloc(Math.max(65536, size));
        bufferSize = size;
    }
    return bufferOffset;
}

async function reset() {
    let imports: WebAssemblyImports = { env: { storeRead, storeWrite, commSend, modelSend, time, log }, wasi_snapshot_preview1: wasiStubs };
    model = await WebAssembly.instantiate(wasmModule, imports as unknown as WebAssembly.Imports);
    startTime = Date.now();
    wasmExports = model.exports as unknown as WebAssemblyExports;
    wasmExports._initialize();
    bufferOffset = 0;
    bufferSize = 0;
}

async function main() {
    console.log('Loading storage...');
    await loadStorage();

    console.log('Compiling WebAssembly module...');
    wasmModule = await WebAssembly.compileStreaming(fetch('model.wasm'));

    console.log('Resetting module...');
    await reset();

    console.log('Worker initialization done.');
    msg({ type: 'model', data: new Uint8Array() });
}

onmessage = (e: MessageEvent<WorkerModelEvent>) => {
    let msg = e.data;
    let data = msg.data;
    let offset = getBuffer(data.length);
    new Uint8Array(wasmExports.memory.buffer, offset, data.length).set(data);
    if (msg.type === 'comm') {
        wasmExports.commRecv(offset, data.length);
    } else if (msg.type === 'model') {
        wasmExports.modelRecv(offset, data.length);
    }
}

let db: IDBDatabase;

let slotWriting = false;
let slotData = [new Uint8Array(0), new Uint8Array(0)];

export function storeRead(slot: number, dataOffset: number, size: number): void {
    let buffer = new Uint8Array(wasmExports.memory.buffer, dataOffset, size);
    if (size != slotData[slot].length) {
        buffer.fill(0xFF);
    } else {
        buffer.set(slotData[slot]);
    }
}

export function storeWrite(state: number, slot: number, dataOffset: number, size: number): number {
    let data = new Uint8Array(wasmExports.memory.buffer, dataOffset, size).slice();

    if (state == 0) {
        if (slotWriting) {
            throw new Error('Invalid slot state');
        }
        slotWriting = true;
        let store = db.transaction('storage', 'readwrite').objectStore('storage');
        let req = store.put({ key: slot, value: data });
        req.onsuccess = event => {
            store.transaction.commit();
            slotWriting = false;
            slotData[slot] = data;
        };
        req.onerror = event => {
            store.transaction.abort();
            slotWriting = false;
            console.error('Write to database error!', event, { key: slot, value: data });
        }
        return 1;
    } else if (state < 10) {
        state++;
        return state;
    } else {
        return slotWriting ? 100 : 0;
    }
}


export async function loadStorage() {
    let store: IDBObjectStore;
    let openReq = indexedDB.open('modelStorage', 1);
    let resolve: (value: string | PromiseLike<any>) => void;
    let promise = new Promise<any>(r => { resolve = r; });
    openReq.onerror = (event) => resolve(event.type);
    openReq.onsuccess = (event) => resolve(event.type);
    openReq.onupgradeneeded = (event) => resolve(event.type);
    openReq.onblocked = (event) => resolve(event.type);
    switch (await promise) {
        case 'success':
            db = openReq.result;
            store = db.transaction('storage', 'readonly').objectStore('storage');
            break;
        case 'blocked':
        case 'error':
            throw new Error('Open persistent storage failed!');
        case 'upgradeneeded':
            db = openReq.result;
            try {
                db.deleteObjectStore('storage');
            } catch (err) { }
            store = db.createObjectStore('storage', { keyPath: 'key' });
            break;
        default:
            throw new Error();
    }
    let getReq = store.getAll(IDBKeyRange.bound(0, 1));
    promise = new Promise<any>(r => { resolve = r; });
    getReq.onerror = (event) => resolve(event.type);
    getReq.onsuccess = (event) => resolve(event.type);
    switch (await promise) {
        case 'success':
            console.log(`Slots available: ${getReq.result.length}`);
            for (let row of (getReq.result || [])) {
                console.log(`Slot ${row.key} initialized`);
                slotData[row.key] = row.value;
            }
            break;
        case 'error':
            console.log('Storage read error!');
            break;
    }
    store.transaction.commit();
}

main();
