
import { Message, StateStruct, StateType } from './common';
import { State, get, set } from './struct';

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
        let st = state as unknown as State;
        running = true;
        let lastIterTime = Date.now();
        updateState();
        let expectedSimuTime: number = st.Time as number;
        while (running) {
            let time = Date.now();
            expectedSimuTime += (st.speed as number) * (time - lastIterTime) / 1000;
            lastIterTime = time;
            let timeToSimulate = Math.max(0, expectedSimuTime - (st.Time as number)) + (st.period as number) * (st.speed as number);
            let steps = Math.ceil(Math.min(timeToSimulate / (st.maxStepSize as number), stepsPerSecond * (st.period as number)));
            let t = Date.now();
            exports.steps(steps, (st.maxStepSize as number));
            let elapsed = Math.max(1, Date.now() - t);
            updateState();
            let newStepsPerSecond = steps / elapsed * 1000;
            stepsPerSecond = Math.ceil(0.1 * newStepsPerSecond + 0.9 * stepsPerSecond);
            let delta = (st.Time as number) - expectedSimuTime;
            await new Promise(r => setTimeout(r, Math.round(1000 * Math.min(2 * (st.period as number), Math.max(0, delta / (st.speed as number))))));
            if (delta < -(st.period as number) * (st.speed as number)) {
                expectedSimuTime = st.Time as number;
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

async function reset() {
    model = await WebAssembly.instantiate(module);
    exports = model.exports as unknown as WebAssemblyExports;
    exports._initialize();
    memory = exports.memory;
    stateOffset = exports.getState();
}

function updateState(sendMessage: boolean = true) {
    let view = new DataView(memory.buffer, stateOffset);
    state = get(view);
    if (sendMessage) {
        postMessage({ type: 'state', state: state });
    }
}

async function main() {
    module = await WebAssembly.compileStreaming(fetch('model.wasm'));
    await reset();
    updateState(false);
    postMessage({ type: 'ready', stateStruct, state });
}

main();

