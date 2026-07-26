// Web interface for Vircon32 Web Emulator
// Handles file loading and communication between browser and C++ code.
// IMPORTANT: this file must load BEFORE Vircon32Web.js so Module is configured first.

// Guard: only define Module once. Vircon32Web.js may also reference Module but must
// NOT redefine it — the Emscripten shell template uses {{{ SCRIPT }}} which appends
// to the page after this file, so as long as this file runs first, Module is ready.
var Module = (typeof Module !== 'undefined') ? Module : {};

// Route stdout/stderr from C++ to the browser console for easier debugging.
Module.print = function(text) {
    console.log('[Vircon32]', text);
};
Module.printErr = function(text) {
    console.error('[Vircon32]', text);
};

// Called when WASM compilation or runtime encounters a fatal error.
Module.onAbort = function(reason) {
    console.error('[Vircon32] WASM aborted:', reason);
    const status = document.getElementById('status');
    if (status) status.textContent = 'Estado: ERROR FATAL — ' + reason;
    const loading = document.getElementById('loading');
    if (loading) {
        loading.innerHTML = '<div style="color:#ff4444;font-size:1rem;padding:20px;text-align:center;">' +
            'Error al inicializar el emulador.<br>' +
            '<small>' + String(reason).substring(0, 200) + '</small></div>';
        loading.style.display = 'flex';
    }
};

// Hint for LEGACY_GL_EMULATION FFP shader selection.
// Vircon32 GPU uses exactly 1 texture unit at a time.
// Emscripten's FFP emulation places the color vertex attribute at index
// (GL_MAX_TEXTURE_IMAGE_UNITS * 2), so keeping this at 1 gives index 2,
// well within WebGL1's guaranteed minimum of 8 vertex attribs.
// A value of 8 or higher pushes the index to 16+ which exceeds
// MAX_VERTEX_ATTRIBS and floods the console with GL_INVALID_VALUE errors.
Module.GL_MAX_TEXTURE_IMAGE_UNITS = 1;

Module.canvas = (function() {
    const canvas = document.getElementById('canvas');
    if (canvas) {
        canvas.addEventListener("webglcontextlost", function(e) {
            console.error('WebGL context lost. Please reload the page.');
            e.preventDefault();
        }, false);
    }
    return canvas;
})();

Module.onRuntimeInitialized = function() {
    console.log('[Vircon32] WASM runtime initialized');
    setupFileUpload();

    const loading = document.getElementById('loading');
    if (loading) {
        loading.style.display = 'none';
    }

    const status = document.getElementById('status');
    if (status) {
        status.textContent = 'Estado: Emulador listo — inserta un cartucho (.v32)';
    }
};

function setupFileUpload() {
    const fileInput = document.getElementById('romFileInput');
    if (fileInput) {
        fileInput.addEventListener('change', handleFileSelect, false);
    }

    const memoryCardInput = document.getElementById('memoryCardInput');
    if (memoryCardInput) {
        memoryCardInput.addEventListener('change', loadMemoryCardFile, false);
    }
}

// Large ROMs must NOT use ccall type 'array' (that copies onto the WASM stack).
// Write into Emscripten's MEMFS and load by path instead.
function writeAndLoad(path, uint8Array, exportName) {
    if (typeof FS === 'undefined' || typeof FS.writeFile !== 'function') {
        console.error('Emscripten FS is not available');
        if (window.updateEmulatorStatus) {
            window.updateEmulatorStatus('FS no disponible');
        }
        return;
    }

    if (typeof Module.ccall !== 'function') {
        console.error('Module.ccall is not available — WASM runtime not ready or aborted');
        return;
    }

    try {
        FS.createPath('/', 'uploads', true, true);
    } catch (e) {
        // directory may already exist
    }

    try {
        FS.writeFile(path, uint8Array);
        Module.ccall(exportName, null, ['string'], [path]);
    } catch (err) {
        console.error('Failed to load file via FS:', err);
        if (window.updateEmulatorStatus) {
            window.updateEmulatorStatus('Error al cargar archivo');
        }
    }
}

function handleFileSelect(event) {
    const file = event.target.files[0];
    if (!file) return;

    console.log('Loading ROM:', file.name);

    const reader = new FileReader();
    reader.onload = function(e) {
        const uint8Array = new Uint8Array(e.target.result);
        writeAndLoad('/uploads/cartridge.v32', uint8Array, 'loadCartridgeFromPath');
    };
    reader.readAsArrayBuffer(file);
}

function loadMemoryCardFile(event) {
    const file = event.target.files[0];
    if (!file) return;

    console.log('Loading Memory Card:', file.name);

    const reader = new FileReader();
    reader.onload = function(e) {
        const uint8Array = new Uint8Array(e.target.result);
        writeAndLoad('/uploads/memorycard.memc', uint8Array, 'loadMemoryCardFromPath');
    };
    reader.readAsArrayBuffer(file);
}

function saveMemoryCard() {
    if (typeof Module === 'undefined' || typeof Module.ccall !== 'function') return;

    const size = Module.ccall('getMemoryCardSize', 'number', [], []);
    if (size <= 0) {
        console.warn('saveMemoryCard: no memory card loaded');
        return;
    }

    try {
        // Force flush of in-memory card data to MEMFS before reading the file
        Module.ccall('flushMemoryCard', null, [], []);

        // Read the memory card file from Emscripten's virtual FS
        const data = FS.readFile('/uploads/memorycard.memc', { encoding: 'binary' });
        const blob = new Blob([data], { type: 'application/octet-stream' });
        const url = URL.createObjectURL(blob);

        const a = document.createElement('a');
        a.href = url;
        a.download = 'memorycard.memc';
        document.body.appendChild(a);
        a.click();
        document.body.removeChild(a);
        URL.revokeObjectURL(url);

        console.log('Memory card saved (' + size + ' bytes)');
        if (window.updateEmulatorStatus) {
            window.updateEmulatorStatus('Tarjeta de memoria guardada');
        }
    } catch (err) {
        console.error('saveMemoryCard failed:', err);
        if (window.updateEmulatorStatus) {
            window.updateEmulatorStatus('Error al guardar tarjeta');
        }
    }
}

document.addEventListener('keydown', function(event) {
    const emulatorKeys = [
        'Escape', 'F1', 'F2', 'F3', 'F4', 'F5', 'F6', 'F7', 'F8', 'F9', 'F10', 'F11', 'F12'
    ];

    if (emulatorKeys.includes(event.key)) {
        event.preventDefault();
    }
});
