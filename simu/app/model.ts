import { assert, bufferEqual, concatenateBuffers, fatal, SerialInterface, wait } from "./simu-common";
import { bitmap, decode, encode, setMode, sizes, State } from "./model-struct";

const RESPONSE_TIMEOUT = 2000;

enum CmdType {
    TYPE_DIAG = 1 << 5,
    TYPE_READ = 2 << 5,
    TYPE_WRITE = 3 << 5,
    TYPE_RUN = 4 << 5,
    TYPE_INIT = 5 << 5,
    TYPE_RESET = 6 << 5,
    TYPE_MASK = 7 << 5,
};

let listener: Listener;
let serial: SerialInterface;
let packetId: number = 1;
let magicBytes: Uint8Array;
let fptypeSize = 4;
let receiveBuffer = new Uint8Array(65536);
let receiveBufferLength = 0;
const diagToSend: Uint8Array[] = [];

let dataResolveFunc: undefined | ((value: void | PromiseLike<void>) => void) = undefined;

export { State };

export type Listener = () => void;

export const state: State = {} as State;

export const diagInterface: SerialInterface = {
    send(data: Uint8Array): void {
        diagToSend.push(data.slice());
    },
};

export async function initialize(aListener: Listener, aSerial: SerialInterface): Promise<void> { // TODO: wait for any pending input data on serial during serial interface creation (not needed for wasm)
    listener = aListener;
    serial = aSerial;
    magicBytes = new Uint8Array(new Array(32).fill(0).map(() => Math.floor(255 * Math.random())));
    receiveBufferLength = 0;
    packetId = 2;
    serial.onReceive = dataReceived;
    await initTransmission();
    modelLoop()
        .catch(reason => {
            console.error(reason);
            fatal(reason.toString());
        });
}

async function modelLoop() {
    let remoteStateBuffer = await readInitialState();
    let oldState = { ...state };
    let tempStateBuffer = new Uint8Array(sizes.state + sizes.params);
    let wasRunning = false;
    let paramsReadTimeout = Date.now() + 5000;
    encode(state, remoteStateBuffer);
    listener();
    while (true) {
        // Send any pending diagnostic data
        await sendDiagData();
        // Sleep and stop further processing if simulation is not running
        if (!state.running) {
            await wait(200);
            wasRunning = false;
            continue;
        }
        // Send any pending changes to device
        let sentChanges = compareStates(oldState, state);
        if (sentChanges.size > 0) {
            encode(state, tempStateBuffer);
            oldState = { ...state };
            await sendStateChanges(sentChanges, tempStateBuffer);
        }
        // Read also parameters from device once every 5 sec.
        if (Date.now() >= paramsReadTimeout) {
            paramsReadTimeout = Date.now() + 5000;
            await readParameters(remoteStateBuffer);
        }
        // Run simulation and update local copy of remote state
        await runSimulation(remoteStateBuffer, state.maxStepTime, state.maxSimuTime, !wasRunning);
        let remoteState = {} as State;
        decode(remoteStateBuffer, remoteState);
        // Put remote changes into local state except fields that were changed recently
        let newChanges = compareStates(oldState, state);
        updateState(remoteState, (sentChanges as any).union(newChanges));
        // Notify listener
        listener();
        wasRunning = true;
    }
}

function updateState(remoteState: State, sentChanges: Set<string>) {
    for (let name in state) {
        if (!sentChanges.has(name)) {
            state[name] = remoteState[name];
        }
    }
}


async function sendStateChanges(fields: Set<string>, buffer: Uint8Array): Promise<void> {
    let mask = bitmap(fields);
    let index = 0;
    while (index < mask.length) {
        // Skip unchanged fields
        while (index < fields.size && mask[index] === 0) {
            index++;
        }
        // Start at first changed field
        let begin = index;
        // End at four zeros or the end of packet or the end of state buffer
        let maxEnd = Math.min(mask.length, begin + 252);
        while (index < maxEnd && (mask[index] !== 0 || mask[index + 1] !== 0 || mask[index + 2] !== 0 || mask[index + 4] !== 0)) {
            index++;
        }
        let end = index;
        // Send write command on changed fields
        if (end > begin) {
            await exchangePackets([
                CmdType.TYPE_WRITE, 2 + end - begin,
                begin >> 8, begin && 0xFF,
                ...buffer.subarray(begin, end),
            ], false);
        }
    }
}

function compareStates(a: State, b: State) {
    let result = new Set<string>();
    for (let n in a) {
        if (a[n] !== b[n]) result.add(n);
    }
    return result;
}

async function runSimulation(remoteStateBuffer: Uint8Array, maxStepTime: number, maxSimuTime: number, resetTimeState: boolean) {
    let maxStepTimeMs = Math.max(1000, Math.max(3, Math.round(maxStepTime * 1000)));
    let maxSimuTimeMs = Math.max(1000, Math.max(3, Math.round(maxSimuTime * 1000)));
    let response = await exchangePackets([
        CmdType.TYPE_RUN, 5,
        maxStepTimeMs >> 8, maxStepTimeMs && 0xFF,
        maxSimuTimeMs >> 8, maxSimuTimeMs && 0xFF,
        resetTimeState ? 1 : 0
    ], true);
    remoteStateBuffer.set(response, 0);
}




async function initTransmission() {
    // Reset transmission and initialize the model
    serial.send(new Uint8Array(255 + 2 + 2 + 8));
    serial.send(new Uint8Array([CmdType.TYPE_INIT | 0x01, magicBytes.length, ...magicBytes, 0x00]));
    // Read incoming data until magic bytes are received
    let endTime = Date.now() + RESPONSE_TIMEOUT;
    while (true) {
        for (let offset = 0; offset <= receiveBufferLength - magicBytes.length - 1; offset++) {
            if (bufferEqual(magicBytes, new Uint8Array(receiveBuffer.buffer, offset, magicBytes.length))) {
                // Set 32/64-bit floating point numbers mode
                fptypeSize = receiveBuffer[offset + magicBytes.length];
                setMode(fptypeSize);
                break;
            }
        }
        await waitForData(endTime - Date.now());
    }
}

async function readInitialState(): Promise<Uint8Array> {
    let totalSize = sizes.state + sizes.params;
    let response = await exchangePackets([
        CmdType.TYPE_READ, 4,
        0, 0,
        totalSize >> 8, totalSize & 0xFF
    ], true);
    decode(response, state);
    return response;
}

async function readParameters(buffer: Uint8Array): Promise<void> {
    let response = await exchangePackets([
        CmdType.TYPE_READ, 4,
        sizes.state >> 8, sizes.state & 0xFF,
        sizes.params >> 8, sizes.params & 0xFF,
    ], true);
    buffer.set(response, sizes.state);
}

async function exchangePackets(input: number[] | Uint8Array, chainResponse: boolean): Promise<Uint8Array> {
    // Postprocess the packet to send
    input = new Uint8Array(input);
    packetId = packetId >= 0x1F ? 1 : packetId + 1;
    let sendHead = input[0] | packetId;
    input[0] = sendHead;
    // Send the packet
    serial.send(input);
    // Receive incoming packet
    let endTime = Date.now() + RESPONSE_TIMEOUT;
    let packetChain: undefined | Uint8Array[] = chainResponse ? [] : undefined;
    while (true) {
        // Trim transmission synchronization bytes (zeros) if exists
        let firstNonZero = 0;
        while (receiveBuffer[firstNonZero] === 0 && firstNonZero < receiveBufferLength) {
            firstNonZero++;
        }
        if (firstNonZero > 0) {
            receiveBuffer.copyWithin(0, firstNonZero, receiveBufferLength - firstNonZero);
            receiveBufferLength -= firstNonZero;
        }
        // Loop over all fully received packets
        while (receiveBufferLength >= 2 && receiveBufferLength >= 2 + receiveBuffer[1]) {
            // Interpret the packet
            let dataSize = receiveBuffer[1];
            let receivedHead = receiveBuffer[0];
            let packetType = receivedHead & CmdType.TYPE_MASK;
            let packetId = receivedHead & ~CmdType.TYPE_MASK;
            let packetData = receiveBuffer.slice(2, dataSize);
            // Remove packet from the buffer
            receiveBuffer.copyWithin(0, 2 + dataSize, receiveBufferLength - 2 - dataSize);
            // Pass the packet to appropriate destination
            if (packetType == CmdType.TYPE_DIAG && packetId === 0) {
                // This is incoming diag data, so redirect it to diag interface
                diagDataReceived(packetData);
            } else if (sendHead === receivedHead) {
                // We received a correct response
                if (packetChain) {
                    packetChain.push(packetData);
                    if (packetData.length < 255 && packetChain.length > 1) {
                        return concatenateBuffers(packetChain);
                    } else {
                        return packetData;
                    }
                } else {
                    return packetData;
                }
            } else {
                // We get some different response - fatal error
                fatal('Unexpected response over the model serial interface.');
            }
        }
        await waitForData(endTime - Date.now());
    }
}

async function sendDiagData() {
    while (diagToSend.length > 0) {
        let list = diagToSend.splice(0);
        let totalSize = list.map(d => d.length).reduce((s, d) => s + d, 0);
        let buffer = new Uint8Array(totalSize);
        let offset = 0;
        for (let data of list) {
            buffer.set(data, offset);
            offset += data.length;
        }
        for (let offset = 0; offset < totalSize; offset += 255) {
            let chunkSize = Math.min(255, totalSize - offset);
            await exchangePackets([CmdType.TYPE_DIAG, chunkSize, ...buffer.subarray(offset, chunkSize)], false);
        }
    }
}

function diagDataReceived(data: Uint8Array) {
    if (diagInterface.onReceive) {
        diagInterface.onReceive(data);
    }
}

function waitForData(timeout: number): Promise<void> {
    assert(!dataResolveFunc, 'Unexpected state');
    return new Promise<void>((resolve, reject) => {
        let timeoutHandle = setTimeout(() => {
            dataResolveFunc = undefined;
            reject(new Error('Wait timeout'));
        }, timeout);
        dataResolveFunc = () => {
            clearTimeout(timeoutHandle);
            resolve();
        };
    });
}

function dataReceived(data: Uint8Array): void {
    // Append data to receive buffer
    assert(receiveBufferLength + data.length <= receiveBuffer.length,
        'Model serial interface receive buffer overflow!');
    receiveBuffer.set(data, receiveBufferLength);
    receiveBufferLength += data.length;
    // Notify if someone is waiting for data
    if (dataResolveFunc) {
        let resolve = dataResolveFunc;
        dataResolveFunc = undefined;
        resolve();
    }
}
