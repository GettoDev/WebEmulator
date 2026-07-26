// WebFileLoader.cpp
// Handles file loading from memory/FS for web version

#include "VirconEmulator.hpp"
#include "Globals.hpp"
#include <cstring>
#include <vector>
#include <string>

static std::vector<unsigned char> ROMBuffer;
static std::vector<unsigned char> MemoryCardBuffer;

#ifdef __EMSCRIPTEN__
#include <emscripten.h>

static void NotifyStatus(const char* message)
{
    EM_ASM({
        if (window.updateEmulatorStatus) {
            window.updateEmulatorStatus(UTF8ToString($0));
        }
    }, message);
}

static void PrepareCartridgeSlot()
{
    if (Vircon.PowerIsOn)
        Vircon.PowerOff();

    if (Vircon.HasCartridge())
        Vircon.UnloadCartridge();
}

static void PrepareMemoryCardSlot()
{
    if (Vircon.PowerIsOn)
        Vircon.PowerOff();

    if (Vircon.HasMemoryCard())
        Vircon.UnloadMemoryCard();
}

extern "C" {

    // Preferred path for large ROMs: JS writes to MEMFS, then calls this.
    EMSCRIPTEN_KEEPALIVE
    void loadCartridgeFromPath(const char* path)
    {
        if (!path || !path[0])
        {
            NotifyStatus("Ruta de cartucho vacia");
            return;
        }

        try
        {
            PrepareCartridgeSlot();
            Vircon.LoadCartridge(path);
            WindowActive = true;
            Vircon.PowerOn();
            Vircon.Resume();
            NotifyStatus("Cartucho cargado y consola encendida");
        }
        catch (const std::exception& e)
        {
            NotifyStatus(e.what());
        }
    }

    EMSCRIPTEN_KEEPALIVE
    void loadMemoryCardFromPath(const char* path)
    {
        if (!path || !path[0])
        {
            NotifyStatus("Ruta de tarjeta vacia");
            return;
        }

        try
        {
            PrepareMemoryCardSlot();
            Vircon.LoadMemoryCard(path);
            NotifyStatus("Tarjeta de memoria cargada");
        }
        catch (const std::exception& e)
        {
            NotifyStatus(e.what());
        }
    }

    // Kept for small buffers / compatibility. Prefer loadCartridgeFromPath for ROMs.
    EMSCRIPTEN_KEEPALIVE
    void loadCartridgeFromMemory(const unsigned char* data, int size)
    {
        if (!data || size <= 0)
            return;

        ROMBuffer.assign(data, data + size);

        try
        {
            PrepareCartridgeSlot();
            Vircon.LoadCartridgeFromMemory(ROMBuffer.data(), size);
            WindowActive = true;
            Vircon.PowerOn();
            Vircon.Resume();
            NotifyStatus("Cartucho cargado y consola encendida");
        }
        catch (const std::exception& e)
        {
            NotifyStatus(e.what());
        }
    }

    EMSCRIPTEN_KEEPALIVE
    void loadMemoryCardFromMemory(const unsigned char* data, int size)
    {
        if (!data || size <= 0)
            return;

        MemoryCardBuffer.assign(data, data + size);

        try
        {
            PrepareMemoryCardSlot();
            Vircon.LoadMemoryCardFromMemory(MemoryCardBuffer.data(), size);
            NotifyStatus("Tarjeta de memoria cargada");
        }
        catch (const std::exception& e)
        {
            NotifyStatus(e.what());
        }
    }

    EMSCRIPTEN_KEEPALIVE
    int getMemoryCardSize()
    {
        if (Vircon.HasMemoryCard())
            return Constants::MemoryCardSize * 4 + 8; // data + 8-byte signature
        return 0;
    }

    // Force an immediate save of the memory card contents to MEMFS so that
    // the JS side can read the file and offer it as a download.
    EMSCRIPTEN_KEEPALIVE
    void flushMemoryCard()
    {
        if (!Vircon.HasMemoryCard()) return;

        try
        {
            const std::string& path = Vircon.MemoryCardController.CardSavePath;
            if (!path.empty())
                Vircon.MemoryCardController.SaveContents(path);
        }
        catch (const std::exception& e)
        {
            NotifyStatus(e.what());
        }
    }
}
#endif
