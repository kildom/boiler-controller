import { ModelChannel } from "./graphCommon";
import { State } from "./struct";


const MAX_BYTES_ON_WIRE = 2000;


enum CmdType {
    TYPE_DIAG = 0x00,
    TYPE_READ = 0x20,
    TYPE_WRITE = 0x40,
    TYPE_RUN = 0x60,
    TYPE_MASK = 0xE0,
};


interface Packet {
    type: CmdType;
    size: number;
}

interface RequestRead extends Packet {
    type: CmdType.TYPE_READ;
}

interface RequestWrite extends Packet {
    type: CmdType.TYPE_WRITE;
    offset: number;
    data: Uint8Array;
}

interface RequestDiag extends Packet {
    type: CmdType.TYPE_DIAG;
    data: Uint8Array;
}

interface RequestRun extends Packet {
    type: CmdType.TYPE_RUN;
    maxStepTimeMs: number;
    maxSimuTimeMs: number;
    resetTimeState: boolean;
}

export class Model {

    public state: State;
    private stateShadow: State;
    private buffer = new Uint8Array(2048);
    private view = new DataView(this.buffer.buffer, this.buffer.byteOffset, this.buffer.byteLength);
    private bufferUsed = 0;
    private running = false;

    private bytesOnWire: number = 0;
    private packetsOnWire: number = 0;

    constructor(private channel: ModelChannel) {
        channel.onReceive = data => {
            if (this.bufferUsed + data.length > this.buffer.length) {
                let buffer = new Uint8Array(2 * (this.bufferUsed + data.length));
                buffer.set(this.buffer);
                this.buffer = buffer;
                this.view = new DataView(this.buffer.buffer, this.buffer.byteOffset, this.buffer.byteLength);
            }
            this.buffer.set(data, this.bufferUsed);
            this.bufferUsed += data.length;
            this.parsePackets();
        };
    }

    parsePackets() {
        while (this.bufferUsed >= 2) {
            let type = this.view.getUint8(0);
            let size = this.view.getUint8(1);
            if (this.bufferUsed >= size + 2) {
                this.packet(type, size);
                this.buffer.copyWithin(0, size + 2, this.bufferUsed);
                this.bufferUsed -= size + 2;
            } else {
                break;
            }
        }
    }

    packet(type: number, size: number) {
        let b = this.buffer.subarray(2, 2 + size);
        console.log(new TextDecoder().decode(b));
    }

    start() {

    }

    stop() {

    }



}
