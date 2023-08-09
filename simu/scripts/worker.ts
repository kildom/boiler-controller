
import { Message, StateType } from './common.js';
import { State, get, set } from './struct.js';

interface WebAssemblyExports {
    memory: WebAssembly.Memory;
    _initialize(): void;
    getState(): number;
    steps(count: number, maxStepTime: number): void;
    commRecv(size: number): number;
    button(index: number, state: number): void;
};

interface WebAssemblyImports {
    env: {
        commSend(data: number, size: number): void;
        storeRead(buffer: number, size: number): void;
        storeWrite(buffer: number, size: number): void;
    };
    wasi_snapshot_preview1: {
        fd_close(): number;
        fd_seek(): number;
        fd_write(): number;
    }
};

let module: WebAssembly.Module;
let model: WebAssembly.Instance;
let exports: WebAssemblyExports;
let memory: WebAssembly.Memory;
let stateOffset: number;
let running: boolean;
let stepsPerSecond: number = 10;
let state: StateType = {};
let storage: Uint8Array | null = null;
let db: IDBDatabase;


async function startSimulation() {
    console.log("Starting simulation");
    if (running) {
        console.log("Simulation already running");
        return;
    }
    try {
        running = true;
        let lastIterTime = Date.now();
        updateState();
        let st = state as unknown as State;
        let expectedSimuTime: number = st.Time as number;
        while (running) {
            let time = Date.now();
            expectedSimuTime += st.speed * (time - lastIterTime) / 1000;
            lastIterTime = time;
            let timeToSimulate = Math.max(0, expectedSimuTime - st.Time);
            let steps = Math.ceil(Math.min(timeToSimulate / st.maxStepSize, stepsPerSecond * st.period));
            let t = Date.now();
            exports.steps(steps, st.maxStepSize);
            let elapsed = Math.max(1, Date.now() - t) / 1000;
            updateState();
            st = state as unknown as State;
            let newStepsPerSecond = steps / elapsed;
            stepsPerSecond = Math.ceil(0.1 * newStepsPerSecond + 0.9 * stepsPerSecond);
            let delta = st.Time - expectedSimuTime;
            let sleepTime = Math.round(1000 * Math.min(2 * st.period, Math.max(0, delta / st.speed)));
            await new Promise(r => setTimeout(r, sleepTime));
            if (delta < -4 * st.period * st.speed) {
                setState('speed', Math.round(st.speed * 0.95));
                expectedSimuTime = st.Time;
            }
        }
    } finally {
        running = false;
    }
}

function stopSimulation() {
    running = false;
}

function setState(name: string, value: number | boolean) {
    let view = new DataView(memory.buffer, stateOffset);
    set(view, name, value);
    state[name] = value;
}

function commRecv(buffer: Uint8Array) {
    let size = buffer.length;
    let offset = exports.commRecv(size);
    let out = new Uint8Array(memory.buffer, offset, size);
    out.set(buffer);
}

onmessage = (e: MessageEvent<Message>) => {
    let data = e.data;
    switch (data.type) {
        case 'start':
            startSimulation();
            break;
        case 'stop':
            stopSimulation();
            break;
        case 'set-state':
            setState(data.name, data.value);
            break;
        case 'button':
            exports.button(data.index, data.state ? 1 : 0);
            break;
        case 'comm':
            commRecv(data.buffer);
            break;
        default:
            throw new Error();
    }
};

function msg(message: Message) {
    postMessage(message);
}

function commSend(dataOffset: number, size: number): void {
    let buffer = new Uint8Array(exports.memory.buffer, dataOffset, size).slice();
    msg({ type: 'comm', buffer: buffer });
};


function storeRead(dataOffset: number, size: number): void {
    let buffer = new Uint8Array(exports.memory.buffer, dataOffset, size);
    if (storage === null || size != storage.length) {
        buffer.fill(0);
    } else {
        buffer.set(storage);
    }
}

let storageWriteState: 'none' | 'writing' | 'pending' = 'none';

function writeStorageToDatabase() {
    if (storageWriteState != 'none') {
        storageWriteState = 'pending';
        return;
    }
    storageWriteState = 'writing';
    let store = db.transaction('storage', 'readwrite').objectStore('storage');
    let req = store.add({ key: 1, value: storage });
    req.onsuccess = event => {
        store.transaction.commit();
        if (storageWriteState == 'pending') {
            storageWriteState = 'none';
            writeStorageToDatabase();
        }
        storageWriteState = 'none';
    };
    req.onerror = event => {
        store.transaction.abort();
        storageWriteState = 'none';
        console.error('Write to database error!');
    }
}

function storeWrite(dataOffset: number, size: number): void {
    storage = new Uint8Array(exports.memory.buffer, dataOffset, size).slice();
    writeStorageToDatabase();
}

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

async function reset() {
    let imports: WebAssemblyImports = { env: { commSend, storeRead, storeWrite }, wasi_snapshot_preview1: wasiStubs };
    model = await WebAssembly.instantiate(module, imports as unknown as WebAssembly.Imports);
    exports = model.exports as unknown as WebAssemblyExports;
    exports._initialize();
    memory = exports.memory;
    stateOffset = exports.getState();
}

function updateState(sendMessage: boolean = true) {
    let view = new DataView(memory.buffer, stateOffset);
    state = get(view);
    if (sendMessage) {
        msg({ type: 'state', state: state });
    }
}

async function loadStorage() {
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
    }
    let getReq = store.get(1);
    promise = new Promise<any>(r => { resolve = r; });
    getReq.onerror = (event) => resolve(event.type);
    getReq.onsuccess = (event) => resolve(event.type);
    switch (await promise) {
        case 'success':
            if (getReq.result) {
                storage = getReq.result.value;
            } else {
                storage = null;
                console.log('Storage empty!');
            }
            break;
        case 'error':
            storage = null;
            console.log('Storage read error!');
            break;
    }
    store.transaction.commit();
}

async function main() {
    module = await WebAssembly.compileStreaming(fetch('model.wasm'));
    await loadStorage();
    await reset();
    updateState(false);
    msg({ type: 'ready', state });
}

main();

