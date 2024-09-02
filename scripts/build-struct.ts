import { readFileSync, writeFileSync } from "fs";
import * as path from "path";
import { StateStruct } from "./common";

const DIR = path.dirname(process.argv[1]);
const MODEL_PATH = path.resolve(DIR, '../simu/model/model.hh');
const OUTPUT_PATH = path.join(DIR, '../simu/app/model-struct-generated.ts');

let stateStruct: StateStruct = {};
let paramsStruct: StateStruct = {};
let paramsBegin32: number = 0;
let paramsBegin64: number = 0;
let paramsEnd32: number = 0;
let paramsEnd64: number = 0;

async function getStateStruct() {
    let _: any;
    let text = readFileSync(MODEL_PATH, 'utf-8');
    [_, text] = text.split('// BEGIN STATE', 2);
    [text] = text.split('// END PARAMS', 1);
    let currentStruct = stateStruct;
    let group: typeof stateStruct[''] = [];
    let offset32 = 0;
    let offset64 = 0;
    let lines = text
        .split('\n')
        .map(line => line.trim())
        .filter(line => line)
        .filter(line => line === '// BEGIN PARAMS' || !line.startsWith('#'));
    for (let line of lines) {
        let m: RegExpMatchArray | null;
        if (line === '// BEGIN PARAMS') {
            paramsBegin32 = offset32;
            paramsBegin64 = offset64;
            currentStruct = paramsStruct;
        } else if ((m = line.match(/^\/\/\s+(.*)$/))) {
            currentStruct[m[1]] = [];
            group = currentStruct[m[1]];
        } else if ((m = line.match(/^(.*)\s+(.*);\s*\/\/\s*(.*?)\s+(.*)$/))) {
            let type = m[1].trim();
            let method = m[3].trim();
            if (type !== 'bool' && type !== 'fptype' && type !== 'uint8_t') throw new Error(type);
            if (method !== 'mod' && method !== 'param' && method !== 'calc' && method !== 'out' && method !== 'in') throw new Error(method);
            let size32 = type === 'fptype' ? 4 : 1;
            let size64 = type === 'fptype' ? 8 : 1;
            offset32 = (offset32 + size32 - 1) & ~(size32 - 1);
            offset64 = (offset64 + size64 - 1) & ~(size64 - 1);
            group.push({
                type,
                name: m[2].trim(),
                method,
                comment: m[4].trim(),
                offset32: offset32,
                offset64: offset64,
            });
            offset32 += size32;
            offset64 += size64;
            paramsEnd32 = offset32;
            paramsEnd64 = offset64;
        }
    }
}

function cTypeToTsType(type: 'bool' | 'fptype' | 'uint8_t') {
    switch (type) {
        case 'bool':
            return 'boolean';
        case 'fptype':
        case 'uint8_t':
            return 'number';
    }
}


function buildStruct() {
    getStateStruct();
    let out = '\n';

    out += 'export const sizes32 = {\n';
    out += `    state: ${paramsBegin32},\n`;
    out += `    params: ${paramsEnd32 - paramsBegin32},\n`;
    out += `    fptype: 4,\n`;
    out += '};\n\n';

    out += 'export const sizes64 = {\n';
    out += `    state: ${paramsBegin64},\n`;
    out += `    params: ${paramsEnd64 - paramsBegin64},\n`;
    out += `    fptype: 8,\n`;
    out += '};\n\n';

    out += 'export interface State {\n\n';
    for (let struct of [stateStruct, paramsStruct]) {
        for (let group in struct) {
            out += `    // ${group}\n`;
            for (let field of struct[group]) {
                out += `    /** ${field.method} ${field.comment} */\n`;
                out += `    ${field.name}: ${cTypeToTsType(field.type)};\n`;
            }
            out += '\n';
        }
    }
    out += '};\n\n';

    for (let [bits, fieldName] of [[32, 'offset32'], [64, 'offset64']]) {
        out += `export function decode${bits}(view: DataView, result: State): void {\n`;
        for (let struct of [stateStruct, paramsStruct]) {
            for (let group in struct) {
                for (let field of struct[group]) {
                    switch (field.type) {
                        case 'bool':
                            out += `    result.${field.name} = view.getInt8(${field[fieldName]}) ? true : false;\n`;
                            break;
                        case 'fptype':
                            out += `    result.${field.name} = view.getFloat${bits}(${field[fieldName]}, true);\n`;
                            break;
                        case 'uint8_t':
                            out += `    result.${field.name} = view.getUint8(${field[fieldName]});\n`;
                            break;
                    }
                }
            }
        }
        out += '};\n\n';
    }
    
    for (let [bits, fieldName] of [[32, 'offset32'], [64, 'offset64']]) {
        out += `export function encode${bits}(state: State, view: DataView): void {\n`;
        for (let struct of [stateStruct, paramsStruct]) {
            for (let group in struct) {
                for (let field of struct[group]) {
                    switch (field.type) {
                        case 'bool':
                            out += `    view.setInt8(${field[fieldName]}, state.${field.name} ? 1 : 0);\n`;
                            break;
                        case 'fptype':
                            out += `    view.setFloat${bits}(${field[fieldName]}, 1 * (state.${field.name} as any), true);\n`;
                            break;
                        case 'uint8_t':
                            out += `    view.setUint8(${field[fieldName]}, 1 * Math.min(255, Math.max(0, Math.round(state.${field.name} as any))));\n`;
                            break;
                    }
                }
            }
        }
        out += '};\n\n';
    }
    
    for (let [bits, fieldName] of [[32, 'offset32'], [64, 'offset64']]) {
        out += `export function bitmap${bits}(fields: Set<string>): Uint8Array {\n`;
        out += `    let view = new DataView(new ArrayBuffer(sizes${bits}.state + sizes${bits}.params));\n`;
        for (let struct of [stateStruct, paramsStruct]) {
            for (let group in struct) {
                for (let field of struct[group]) {
                    switch (field.type) {
                        case 'bool':
                            out += `    if (fields.has('${field.name}')) view.setInt8(${field[fieldName]}, 1);\n`;
                            break;
                        case 'fptype':
                            if (bits === 64) {
                                out += `    if (fields.has('${field.name}')) {\n`;
                                out += `        view.setInt32(${field[fieldName]}, 0x01010101);\n`;
                                out += `        view.setInt32(${field[fieldName] + 4}, 0x01010101);\n`;
                                out += `    };\n`;
                            } else {
                                out += `    if (fields.has('${field.name}')) view.setInt32(${field[fieldName]}, 0x01010101);\n`;
                            }
                            break;
                        case 'uint8_t':
                            out += `    if (fields.has('${field.name}')) view.setInt8(${field[fieldName]}, 1);\n`;
                            break;
                    }
                }
            }
        }
        out += '    return new Uint8Array(view.buffer);\n';
        out += '};\n\n';
    }
    let old = '';
    /*try {
        old = readFileSync(OUTPUT_PATH, 'utf-8');
    } catch (e) { }*/
    if (old != out) {
        writeFileSync(OUTPUT_PATH, out);
    }
}

buildStruct();
