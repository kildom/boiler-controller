
import * as fs from 'fs';
import * as path from "path";
import * as crypto from 'crypto';

const DIR = path.dirname(process.argv[1]);

const inputs = {
    time: (output: string[], struct: StructItem, indent: string) => {
        output.push(`${indent}{ "${struct.comment}", &timeCbk.base, (const IntItemInfo<${struct.type}>[]) {{ &storage.${struct.fullName}, ${struct.max}, ${struct.min} }}}, \\`);
    },
    int: (output: string[], struct: StructItem, indent: string) => {
        output.push(`${indent}{ "${struct.comment}", &intCbk<${struct.type}>.base, (const IntItemInfo<${struct.type}>[]) {{ &storage.${struct.fullName}, ${struct.max}, ${struct.min} }}}, \\`);
    },
    temp: (output: string[], struct: StructItem, indent: string) => {
        output.push(`${indent}{ "${struct.comment}", &tempCbk<${struct.type}>.base, (const IntItemInfo<${struct.type}>[]) {{ &storage.${struct.fullName}, ${struct.max}, ${struct.min} }}}, \\`);
    },
};

const types = {
    int: (output: string[], struct: StructItem, indent: string) => {
        inputs[struct.input](output, struct, indent);
    },
    int16_t: (...a: any[]) => (types.int as any)(...a),
    bool: (output: string[], struct: StructItem, indent: string) => {
        output.push(`${indent}{ "${struct.comment}", &boolCbk.base, &storage.${struct.fullName} }, \\`);
    },
};

interface StructItem {
    name: string;
    fullName: string;
    structName: string;
    type: keyof typeof types;
    comment: string;
    input: keyof typeof inputs;
    default: string;
    min: string;
    max: string;
    cat: string[];
}

let defaultStructItem: StructItem = {
    name: '',
    fullName: '',
    structName: '',
    type: 'int',
    comment: '',
    input: 'int',
    default: '',
    min: '',
    max: '',
    cat: [],
};

interface Category {
    name: string;
    structures: StructItem[];
    sub: { [name: string]: Category };
};

let structures: { [name: string]: StructItem[] } = {};
let defaults: { [name: string]: string } = {};
let categoriesRoot: Category = { name: '', structures: [], sub: {} };
let main: StructItem[];
let defines: { [key: string]: string[] } = {};
let hash = crypto.createHash('md5');

function parseFile(path: string) {
    //console.log(path);
    let lines = fs.readFileSync(path, 'utf-8').split('\n').map(line => line.trim());
    let active = false;
    let structName = '';
    let item: StructItem = { ...defaultStructItem };
    let cat: string[] = [];
    for (let line of lines) {
        let m: RegExpMatchArray | null;
        if ((m = line.match(/^\s*\/\/\s*->\s*BEGIN\s*(.*)$/))) {
            active = true;
            structName = m[1].trim();
            //console.log(path, '-', structName);
        } else if (!active) {
            // ignore
        } else if (line.match(/^\s*\/\/\s*->\s*END\s*.*$/)) {
            active = false;
        } else if ((m = line.match(/^\s*\/\/\s*->\sDEFAULT\s(.+?)\s(.+)$/))) {
            defaults[m[1].trim()] = m[2].trim();
        } else if ((m = line.match(/^\s*\/\/\s*->\s*(.*)$/))) {
            cat = m[1].split('->').map(x => x.trim());
        } else if ((m = line.match(/^\s*\/\/(.*)$/))) {
            let parts = m[1].split(',').map(x => x.trim());
            let comment: string[] = [];
            item = { ...defaultStructItem };
            for (let part of parts) {
                if ((m = part.match(/^default:(.*)$/))) {
                    item.default = m[1].trim();
                } else if ((m = part.match(/^range:(.*)\.\.(.*)$/))) {
                    item.min = m[1].trim();
                    item.max = m[2].trim();
                } else if (part in inputs) {
                    item.input = part as any;
                } else {
                    comment.push(part);
                }
            }
            item.comment = comment.join(', ');
        } else if ((m = line.match(/^\s*(.*?)([a-z0-9_]+)(\[.*\])?;$/i))) {
            let [type, name, arr] = m.slice(1).map(x => x?.trim());
            if (arr !== undefined) throw new Error(`Not implemented: ${line}`);
            structures[structName] = structures[structName] || [];
            let struct: StructItem = { ...item, type: type as any, name, fullName: name, structName, cat };
            structures[structName].push(struct);
            item = { ...defaultStructItem };
        }
    }
}

function resolveInner(list: StructItem[], prefix: string, cat: string[]) {
    for (let i = 0; i < list.length; i++) {
        let item = list[i];
        if (item.type in types) {
            item.fullName = prefix + item.fullName;
            item.cat = [...cat, ...item.cat];
        } else if (item.type in structures) {
            let copy: StructItem[] = JSON.parse(JSON.stringify(structures[item.type]));
            resolveInner(copy, prefix + item.name + '.', [...cat, ...item.cat]);
            list.splice(i, 1, ...copy);
            i = i + copy.length - 1;
        } else {
            throw new Error(`Unknown type name: ${item.type}`);
        }
    }
}

function setDefaults(list: StructItem[]) {
    for (let item of list) {
        if (item.fullName in defaults) {
            item.default = defaults[item.fullName];
            delete defaults[item.fullName];
        }
    }
    if (Object.keys(defaults).length) {
        throw new Error(`Unknown item: ${Object.keys(defaults).join(', ')}`);
    }
}

function initials(list: string[]) {
    let indent = '    ';
    let prefix: string = '';
    for (let item of main) {
        let itemPrefix = item.fullName.substring(0, item.fullName.length - item.name.length);
        while (!itemPrefix.startsWith(prefix)) {
            let pos = prefix.substring(0, prefix.length - 1).lastIndexOf('.');
            prefix = prefix.substring(0, pos + 1);
            indent = indent.substring(4);
            list.push(`${indent}}, \\`);
        }
        while (itemPrefix.startsWith(prefix) && itemPrefix != prefix) {
            let sub = itemPrefix.substring(prefix.length).split('.')[0];
            list.push(`${indent}.${sub} = { \\`);
            prefix = prefix + sub + '.';
            indent += '    ';
        }
        list.push(`${indent}.${item.name} = ${item.default}, \\`);
    }
}

function menu(cat: Category, indent: string) {
    defines['STORAGE_MENU'] = defines['STORAGE_MENU'] || [];
    let output = defines['STORAGE_MENU'];
    for (let [name, sub] of Object.entries(cat.sub)) {
        output.push(`${indent}{ "${name}", &menuCbk, (const MenuItem[]) { \\`);
        menu(sub, indent + '    ');
        output.push(`${indent}    {}}}, \\`);
    }
    for (let struct of cat.structures) {
        if (!(struct.type in types)) {
            throw new Error(`Unknown type ${struct.type} in ${struct.fullName}`);
        }
        types[struct.type](output, struct, indent);
    }
}

function categorize() {
    for (let struct of main) {
        let branch = categoriesRoot;
        for (let name of struct.cat) {
            branch.sub[name] = branch.sub[name] || { name, structures: [], sub: [] };
            branch = branch.sub[name];
        }
        branch.structures.push(struct);
        hash.update(`|${struct.fullName}:${struct.type}`, 'utf-8');
    }
}

for (let file of fs.readdirSync(path.resolve(DIR, '../src/control'))) {
    if (file.match(/\.(hh|cc|cpp|h|hpp)$/)) {
        parseFile(path.resolve(DIR, `../src/control/${file}`));
    }
}

main = structures[''];
resolveInner(main, '', []);
setDefaults(main);
categorize();

defines['STORAGE_INITIAL'] = [];
initials(defines['STORAGE_INITIAL']);

menu(categoriesRoot, '    ');

let digest = hash.digest('hex').toUpperCase();
defines['STORAGE_MAGIC1_BASE'] = [ '    0x' + digest.substring(0, 8)];
defines['STORAGE_MAGIC2_BASE'] = [ '    0x' + digest.substring(8, 16)];

fs.mkdirSync(path.resolve(DIR, '../build/include'), {recursive: true});

fs.writeFileSync(path.resolve(DIR, '../build/include/autogen.inc'), Object.entries(defines).map(x => `#define ${x[0]} \\\n${x[1].join('\n')}`).join('\n\n') + '\n');

//console.log(structures);
//console.log(defaults);
//console.log(categoriesRoot.sub);

