import { ModelAccess } from "./model-comm.js";


export const COMM_PORT = 0;
export const AUX_PORT = 1;


export interface ModelMessage {
    port: number;
    data: Uint8Array;
};

export class ModelWorker implements ModelAccess {

    private worker: Worker;

    public onRecvComm: null | ((data: Uint8Array) => void) = null;
    public onRecvAux: null | ((data: Uint8Array) => void) = null;

    public constructor() {
        this.worker = new Worker('model-worker.js', { type: 'module' });
        this.worker.onerror = (...e) => console.log(...e);
        this.worker.onmessage = (e: MessageEvent<ModelMessage>) => {
            let data = e.data;
            if (data.port === COMM_PORT) {
                this.onRecvComm?.(data.data);
            } else {
                this.onRecvAux?.(data.data);
            }
        };
    }

    private msg(data: ModelMessage) {
        this.worker.postMessage(data);
    }

    public sendComm(data: Uint8Array) {
        this.msg({ port: COMM_PORT, data });
    }

    public sendAux(data: Uint8Array) {
        this.msg({ port: AUX_PORT, data });
    }

}
