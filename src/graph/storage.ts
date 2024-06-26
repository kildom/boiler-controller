
let db: IDBDatabase;

let slotWriting = false;
let slotData = [new Uint8Array(0), new Uint8Array(0)];

export function storeRead(slot: number, dataOffset: number, size: number): void {
    let buffer = new Uint8Array(exports.memory.buffer, dataOffset, size);
    if (size != slotData[slot].length) {
        buffer.fill(0xFF);
    } else {
        buffer.set(slotData[slot]);
    }
}

export function storeWrite(state: number, slot: number, dataOffset: number, size: number): number {
    let data = new Uint8Array(exports.memory.buffer, dataOffset, size).slice();

    if (state == 0) {
        if (slotWriting) {
            throw new Error('Invalid slot state');
        }
        slotWriting = true;
        let store = db.transaction('storage', 'readwrite').objectStore('storage');
        let req = store.put({ key: slot, value: data });
        req.onsuccess = event => {
            store.transaction.commit();
            slotWriting = false;
            slotData[slot] = data;
        };
        req.onerror = event => {
            store.transaction.abort();
            slotWriting = false;
            console.error('Write to database error!', event, { key: slot, value: data });
        }
        return 1;
    } else if (state < 10) {
        state++;
        return state;
    } else {
        return slotWriting ? 100 : 0;
    }
}


export async function loadStorage() {
    let store: IDBObjectStore;
    let openReq = indexedDB.open('modelStorage', 1);
    let resolve: (value: string | PromiseLike<any>) => void;
    let promise = new Promise<any>(r => { resolve = r; });
    openReq.onerror = (event) => resolve(event.type);
    openReq.onsuccess = (event) => resolve(event.type);
    openReq.onupgradeneeded = (event) => resolve(event.type);
    openReq.onblocked = (event) => resolve(event.type);
    switch (await promise) {
        case 'success':
            db = openReq.result;
            store = db.transaction('storage', 'readonly').objectStore('storage');
            break;
        case 'blocked':
        case 'error':
            throw new Error('Open persistent storage failed!');
        case 'upgradeneeded':
            db = openReq.result;
            try {
                db.deleteObjectStore('storage');
            } catch (err) { }
            store = db.createObjectStore('storage', { keyPath: 'key' });
            break;
        default:
            throw new Error();
    }
    let getReq = store.getAll(IDBKeyRange.bound(0, 1));
    promise = new Promise<any>(r => { resolve = r; });
    getReq.onerror = (event) => resolve(event.type);
    getReq.onsuccess = (event) => resolve(event.type);
    switch (await promise) {
        case 'success':
            console.log(`Slots available: ${getReq.result.length}`);
            for (let row of (getReq.result || [])) {
                console.log(`Slot ${row.key} initialized`);
                slotData[row.key] = row.value;
            }
            break;
        case 'error':
            console.log('Storage read error!');
            break;
    }
    store.transaction.commit();
}
