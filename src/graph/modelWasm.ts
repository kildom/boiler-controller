
import { ModelChannel, WorkerModelEvent } from './graphCommon';


export class ModelWasm implements ModelChannel {

    worker: Worker;

    onReceive?: null | ((data: Uint8Array) => void);
    onCommReceive?: null | ((data: Uint8Array) => void);

    public async init(): Promise<void> {
        this.worker = new Worker('workerModel.js');
        this.worker.onmessage = event => {
            let data = event.data as WorkerModelEvent;
            if (data.type === 'comm') {
                this.onCommReceive?.(data.data);
            } else if (data.type === 'debug') {
                this.onReceive?.(data.data);
            }
        }
    }

    public send(data: Uint8Array): void {
        let msg: WorkerModelEvent = { type: 'debug', data };
        this.worker.postMessage(msg);
    }

    public commSend(data: Uint8Array): void {
        let msg: WorkerModelEvent = { type: 'comm', data };
        this.worker.postMessage(msg);
    }

}
