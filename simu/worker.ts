
import { Message, StateStruct, StateType } from './common';

interface WebAssemblyExports {
    memory: WebAssembly.Memory;
    _initialize(): void;
    getState(): number;
    steps(count: number, maxStepTime: number): void;
};

let module: WebAssembly.Module;
let model: WebAssembly.Instance;
let exports: WebAssemblyExports;
let memory: WebAssembly.Memory;
let stateOffset: number;
let running: boolean;
let stepsPerSecond: number = 10;
let stateStruct: StateStruct = {};
let state: StateType = {};

async function startSimulation() {
    if (running) return;
    try {
        running = true;
        let lastIterTime = Date.now();
        updateState();
        let expectedSimuTime: number = state.Time as number;
        while (running) {
            let time = Date.now();
            expectedSimuTime += (state.speed as number) * (time - lastIterTime) / 1000;
            lastIterTime = time;
            let timeToSimulate = Math.max(0, expectedSimuTime - (state.Time as number)) + (state.period as number) * (state.speed as number);
            let steps = Math.ceil(Math.min(timeToSimulate / (state.maxStepSize as number), stepsPerSecond * (state.period as number)));
            let t = Date.now();
            exports.steps(steps, (state.maxStepSize as number));
            let elapsed = Math.max(1, Date.now() - t);
            updateState();
            let newStepsPerSecond = steps / elapsed * 1000;
            stepsPerSecond = Math.ceil(0.1 * newStepsPerSecond + 0.9 * stepsPerSecond);
            let delta = (state.Time as number) - expectedSimuTime;
            await new Promise(r => setTimeout(r, Math.round(1000 * Math.min(2 * (state.period as number), Math.max(0, delta / (state.speed as number))))));
            if (delta < -(state.period as number) * (state.speed as number)) {
                expectedSimuTime = state.Time as number;
            }
        }
    } finally {
        running = false;
    }
}

async function stopSimulation() {
    running = false;
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
        default:
            throw new Error();
    }
};


function msg(data: Message) {
    postMessage(data);
}

async function reset() {
    model = await WebAssembly.instantiate(module);
    exports = model.exports as unknown as WebAssemblyExports;
    exports._initialize();
    memory = exports.memory;
    stateOffset = exports.getState();
}

async function getStateStruct() {
    let _;
    let response = await fetch('model.hpp');
    if (response.status !== 200) throw new Error();
    let text = await response.text();
    [_, text] = text.split('// BEGIN STATE', 2);
    [text] = text.split('// END STATE', 1);
    let group: typeof stateStruct[''] = [];
    let offset = 0;
    for (let line of text.split('\n').map(line => line.trim()).filter(line => line)) {
        let m: RegExpMatchArray;
        if ((m = line.match(/^\/\/\s+(.*)$/))) {
            stateStruct[m[1]] = [];
            group = stateStruct[m[1]];
        } else if ((m = line.match(/^(.*)\s+(.*);\s*\/\/\s*(.*?)\s+(.*)$/))) {
            let type = m[1].trim();
            let method = m[3].trim();
            if (type !== 'bool' && type !== 'double') throw new Error(type);
            if (method !== 'mod' && method !== 'param' && method !== 'calc' && method !== 'out' && method !== 'in') throw new Error(method);
            let size = type == 'bool' ? 1 : 8;
            offset = (offset + size - 1) & ~(size - 1);
            group.push({
                type,
                name: m[2].trim(),
                method,
                comment: m[4].trim(),
                offset: offset
            });
            offset += size;
        }
    }
}

function updateState(sendMessage: boolean = true) {
    let view = new DataView(memory.buffer, stateOffset);
    for (let groupName in stateStruct) {
        let group = stateStruct[groupName];
        for (let field of group) {
            if (field.type === 'bool') {
                state[field.name] = view.getInt8(field.offset) ? true : false;
            } else if (field.type === 'double') {
                state[field.name] = view.getFloat64(field.offset, true);
            }
        }
    }
    if (sendMessage) {
        msg({ type: 'state', state: state });
    }
}

function setState(name: string, value: number | boolean) {
    let view = new DataView(memory.buffer, stateOffset);
    for (let groupName in stateStruct) {
        let group = stateStruct[groupName];
        for (let field of group) {
            if (field.name === name) {
                if (field.type === 'bool') {
                    view.setInt8(field.offset, value ? 1 : 0);
                } else if (field.type === 'double') {
                    view.setFloat64(field.offset, value as number, true);
                }
                updateState();
                return;
            }
        }
    }
}

async function main() {
    await getStateStruct();
    module = await WebAssembly.compileStreaming(fetch('model.wasm'));
    await reset();
    updateState(false);
    msg({ type: 'ready', stateStruct, state });
}

main();

