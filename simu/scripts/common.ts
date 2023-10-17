import * as xterm from 'xterm';
import { State } from './struct.js';

export interface EmptyMsg {
    type: 'start' | 'stop' | 'get-state',
};

export interface ReadyMsg {
    type: 'ready',
    state: State,
};

export interface StateMsg {
    type: 'state',
    state: State,
};

export interface SetStateMsg {
    type: 'set-state' | 'inc-state',
    name: string,
    value: number | boolean,
};

export interface ButtonMsg {
    type: 'button',
    index: number,
    state: boolean,
};

export interface CommMsg {
    type: 'comm' | 'diag',
    buffer: Uint8Array,
};

export type Message = ReadyMsg | EmptyMsg  | StateMsg | SetStateMsg | ButtonMsg | CommMsg;

export type StateStruct = {
    [group: string]: {
        type: 'double' | 'bool',
        name: string,
        method: 'calc' | 'mod' | 'param' | 'out' | 'in',
        comment: string,
        offset: number
    }[]
};

export type StateType = { [name: string]: number | boolean };

export interface RunParameters {
    period: number,
    maxStepSize: number,
    speed: number,
};

export class StateBase {
    constructor(public view: DataView) { }
    update(offset: number, size: number) { }
}

export interface MainToGui {
    onDiagTermInput(value: string): void;
}


export interface GuiToMain {
    term: xterm.Terminal
}


