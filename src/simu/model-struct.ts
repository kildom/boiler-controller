
import * as gen from './model-struct-generated';

export { State } from './model-struct-generated';

export const sizes = {
    state: 0,
    params: 0,
    fptype: 0,
};


export function setMode(mode: string | number) {
    mode = Math.round(parseFloat(mode.toString()));
    if (mode === 1 || mode === 8 || mode === 64) {
        sizes.state = gen.sizes64.state;
        sizes.params = gen.sizes64.params;
        sizes.fptype = gen.sizes64.fptype;
    } else {
        sizes.state = gen.sizes32.state;
        sizes.params = gen.sizes32.params;
        sizes.fptype = gen.sizes32.fptype;
    }
};


export function decode(view: ArrayBuffer | ArrayBufferView, result: gen.State): void {
    let dataView = (view instanceof DataView) ? view :
        (view instanceof ArrayBuffer) ? new DataView(view) :
        new DataView(view.buffer, view.byteOffset, view.byteLength);
    if (sizes.fptype === 8) {
        gen.decode64(dataView, result);
    } else {
        gen.decode32(dataView, result);
    }
}


export function encode(state: gen.State, view: ArrayBuffer | ArrayBufferView): void {
    let dataView = (view instanceof DataView) ? view :
        (view instanceof ArrayBuffer) ? new DataView(view) :
        new DataView(view.buffer, view.byteOffset, view.byteLength);
    if (sizes.fptype === 8) {
        gen.encode64(state, dataView);
    } else {
        gen.encode32(state, dataView);
    }
}


export function bitmap(fields: Set<string>): Uint8Array {
    if (sizes.fptype === 8) {
        return gen.bitmap64(fields);
    } else {
        return gen.bitmap32(fields);
    }
}
