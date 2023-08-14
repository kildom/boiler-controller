
export interface ReadyMsg {
    type: 'ready',
    state: StateType,
};

export interface StartMsg {
    type: 'start',
};

export interface StopMsg {
    type: 'stop',
};

export interface StateMsg {
    type: 'state',
    state: StateType,
};

export interface SetStateMsg {
    type: 'set-state',
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

export type Message = ReadyMsg | StartMsg | StopMsg | StateMsg | SetStateMsg | ButtonMsg | CommMsg;

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
