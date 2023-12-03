

export interface ServerMessage {
};

export class ServerWorkerConnection { // TODO: implement ServerConnection (the same as ServerWebsocketConnection)

    public onRecv: null | ((data: Uint8Array) => void) = null;

    public constructor(
        private worker: Worker,
        private id: number) { }
    
    public send(data: Uint8Array): void {
    }
};

export class ServerWorker {

    private worker: Worker;
    public onRecvComm: null | ((data: Uint8Array) => void) = null;

    public constructor() {
        this.worker = new Worker('server-worker.js', { type: 'module' });
        this.worker.onerror = (...e) => console.log(...e);
        this.worker.onmessage = (e: MessageEvent<ServerMessage>) => {
            let data = e.data;
        };
    }

    private msg(data: ServerMessage) {
        this.worker.postMessage(data);
    }

    public sendComm(data: Uint8Array) {
        this.msg({});
    }

    public async request(data: any): Promise<any> {
        return {};
    }

    public async connect(): Promise<ServerWorkerConnection> {
        return new ServerWorkerConnection(this.worker, 0);
    }

};
