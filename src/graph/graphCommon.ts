

export interface ModelChannel {
    init(): Promise<void>;
    send(data: Uint8Array): void;
    onReceive?: null | ((data: Uint8Array) => void);
    commSend(data: Uint8Array): void;
    onCommReceive?: null | ((data: Uint8Array) => void);
}

export interface WorkerModelEvent {
    type: 'debug' | 'comm';
    data: Uint8Array;
}
