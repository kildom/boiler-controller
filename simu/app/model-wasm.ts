
import { SerialInterface, WorkerModelEvent } from './simu-common';

let worker: Worker;

export const modelInterface: SerialInterface = {
    send(data: Uint8Array): void {
        let msg: WorkerModelEvent = { type: 'model', data };
        worker.postMessage(msg);
    }
}

export const commInterface: SerialInterface = {
    send(data: Uint8Array): void {
        let msg: WorkerModelEvent = { type: 'comm', data };
        worker.postMessage(msg);
    }
}

export function initWasmModel(): Promise<void> {
    return new Promise<void>(resolve => {
        let resolved = false;
        worker = new Worker('worker-model.js', { name: 'Model' });
        worker.onmessage = event => {
            if (!resolved) {
                resolve();
                resolved = true;
            }
            let data = event.data as WorkerModelEvent;
            if (data.type === 'comm') {
                commInterface.onReceive?.(data.data);
            } else if (data.type === 'model') {
                modelInterface.onReceive?.(data.data);
            }
        }
    });
}
