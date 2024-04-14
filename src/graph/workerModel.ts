import { WorkerModelEvent } from "./graphCommon";
import { loadStorage, storeRead, storeWrite } from "./storage";

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
    msg({ type: 'debug', data });
};

function time(): number {
    return (Date.now() - startTime) & 0x7FFFFFFF;
}

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
    let imports: WebAssemblyImports = { env: { storeRead, storeWrite, commSend, modelSend, time }, wasi_snapshot_preview1: wasiStubs };
    model = await WebAssembly.instantiate(wasmModule, imports as unknown as WebAssembly.Imports);
    startTime = Date.now();
    wasmExports = model.exports as unknown as WebAssemblyExports;
    wasmExports._initialize();
    bufferOffset = 0;
    bufferSize = 0;
}

async function main() {
    await loadStorage();
    wasmModule = await WebAssembly.compileStreaming(fetch('model.wasm'));
    await reset();
    msg({ type: 'debug', data: new Uint8Array() });
}

onmessage = (e: MessageEvent<WorkerModelEvent>) => {
    let msg = e.data;
    let data = msg.data;
    let offset = getBuffer(data.length);
    new Uint8Array(wasmExports.memory.buffer, offset, data.length).set(data);
    if (msg.type === 'comm') {
        wasmExports.modelRecv(offset, data.length);
    } else if (msg.type === 'debug') {
        wasmExports.modelRecv(offset, data.length);
    }
}

main();
