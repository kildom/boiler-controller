
export interface ReadyMsg {
    type: 'ready',
    stateStruct: StateStruct,
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

export type Message = ReadyMsg | StartMsg | StopMsg | StateMsg | SetStateMsg;

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
