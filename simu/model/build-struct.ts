import { readFileSync, writeFileSync } from "fs";
import * as path from "path";

const DIR = path.dirname(process.argv[1]);

type StateStruct = {
    [group: string]: {
        type: 'double' | 'bool',
        name: string,
        method: 'calc' | 'mod' | 'param' | 'out' | 'in',
        comment: string,
        offset: number
    }[]
};

let stateStruct: StateStruct = {};

async function getStateStruct() {
    let _;
    let text = readFileSync(path.resolve(DIR, 'model.hpp'), 'utf-8');
    [_, text] = text.split('// BEGIN STATE', 2);
    [text] = text.split('// END STATE', 1);
    let group: typeof stateStruct[''] = [];
    let offset = 0;
    for (let line of text.split('\n').map(line => line.trim()).filter(line => line)) {
        let m: RegExpMatchArray | null;
        if ((m = line.match(/^\/\/\s+(.*)$/))) {
            stateStruct[m[1]] = [];
            group = stateStruct[m[1]];
        } else if ((m = line.match(/^(.*)\s+(.*);\s*\/\/\s*(.*?)\s+(.*)$/))) {
            let type = m[1].trim();
            let method = m[3].trim();
            if (type !== 'bool' && type !== 'double') throw new Error(type);
            if (method !== 'mod' && method !== 'param' && method !== 'calc' && method !== 'out' && method !== 'in') throw new Error(method);
            let size = type == 'bool' ? 1 : 8;
            offset = (offset + size - 1) & ~(size - 1);
            group.push({
                type,
                name: m[2].trim(),
                method,
                comment: m[4].trim(),
                offset: offset
            });
            offset += size;
        }
    }
}

function cTypeToTsType(type: 'bool' | 'double') {
    switch (type) {
        case 'bool':
            return 'boolean';
        case 'double':
            return 'number';
    }
}

export function buildStruct() {
    getStateStruct();
    let out = '\nexport interface State {\n\n';
    for (let group in stateStruct) {
        out += `    // ${group}\n`;
        for (let field of stateStruct[group]) {
            out += `    /** ${field.method} ${field.comment} */\n`;
            out += `    ${field.name}: ${cTypeToTsType(field.type)};\n`;
        }
        out += '\n';
    }
    out += '};\n\n';
    out += 'export function get(view: DataView): State {\n';
    out += '    return {\n';
    for (let group in stateStruct) {
        for (let field of stateStruct[group]) {
            switch (field.type) {
                case 'bool':
                    out += `        ${field.name}: view.getInt8(${field.offset}) ? true : false,\n`;
                    break;
                case 'double':
                    out += `        ${field.name}: view.getFloat64(${field.offset}, true),\n`;
                    break;
            }
        }
    }
    out += '    };\n';
    out += '};\n\n';
    writeFileSync(path.join(DIR, '..', 'scripts', 'struct.ts'), out);
}

buildStruct();
