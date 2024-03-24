import { ModelState } from "./struct.js";


export interface ModelAccess {
    onRecvAux: null | ((data: Uint8Array) => void);
    sendAux(data: Uint8Array): void;
};

enum ModelPacketType {
    EXECUTE = 1,
    GET_STATE = 2,
    SET_STATE = 3,
    DIAG_RECV = 4,
    DIAG_SEND = 5,
    RESPONSE = 0x40,
}

interface ModelPacket {
    type: ModelPacketType;
    count?: number;
    maxStepTime?: number;
    offset?: number;
    data?: Uint8Array;
    response?: ModelPacket;
};

export class ModelComm {

    private state: ModelState;

    constructor(
        public access: ModelAccess
    ) {
        this.state = {} as ModelState; // TODO: Initialize
        access.onRecvAux = (data: Uint8Array) => this.onRecv(data);
    }

    private onRecv(data: Uint8Array): void {

    }

    private async send(packet: ModelPacket): Promise<ModelPacket> {

    }

    private flush() {

    }

    private async steps(count: number): Promise<void> {
        let offset = 0;
        let stepsPromise = this.send({ type: ModelPacketType.EXECUTE, count, maxStepTime: this.state.maxStepSize });
        let statePromise = this.send({ type: ModelPacketType.GET_STATE, offset });
        this.flush();
    }

};

