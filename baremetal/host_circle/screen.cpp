#include <circle/screen.h>
#include <circle/usb/usbkeyboard.h>
#include <SDL2/SDL.h>
#include <stdio.h>
#include <vector>
#include <algorithm>

CUSBKeyboardDevice *g_pKeyboardDevice = nullptr;
CScreenDevice *g_pScreenDevice = nullptr;

static SDL_Window *g_Window = nullptr;
static SDL_Renderer *g_Renderer = nullptr;
static SDL_Texture *g_Texture = nullptr;

static std::vector<unsigned char> g_ActiveKeys;

static void AddActiveKey(unsigned char code) {
    if (code == 0) return;
    if (std::find(g_ActiveKeys.begin(), g_ActiveKeys.end(), code) == g_ActiveKeys.end()) {
        g_ActiveKeys.push_back(code);
    }
}

static void RemoveActiveKey(unsigned char code) {
    auto it = std::find(g_ActiveKeys.begin(), g_ActiveKeys.end(), code);
    if (it != g_ActiveKeys.end()) {
        g_ActiveKeys.erase(it);
    }
}

static void GetActiveKeys(unsigned char keys[6]) {
    memset(keys, 0, 6);
    for (size_t i = 0; i < g_ActiveKeys.size() && i < 6; i++) {
        keys[i] = g_ActiveKeys[i];
    }
}

static unsigned char SDLKeyToUSBScanCode(SDL_Keycode sym) {
    if (sym >= SDLK_a && sym <= SDLK_z) return 0x04 + (sym - SDLK_a);
    if (sym >= SDLK_1 && sym <= SDLK_9) return 0x1E + (sym - SDLK_1);
    if (sym == SDLK_0) return 0x27;
    switch (sym) {
        case SDLK_RETURN: return 0x28;
        case SDLK_ESCAPE: return 0x29;
        case SDLK_BACKSPACE: return 0x2A;
        case SDLK_TAB: return 0x2B;
        case SDLK_SPACE: return 0x2C;
        case SDLK_MINUS: return 0x2D;
        case SDLK_EQUALS: return 0x2E;
        case SDLK_LEFTBRACKET: return 0x2F;
        case SDLK_RIGHTBRACKET: return 0x30;
        case SDLK_BACKSLASH: return 0x31;
        case SDLK_SEMICOLON: return 0x33;
        case SDLK_QUOTE: return 0x34;
        case SDLK_BACKQUOTE: return 0x35;
        case SDLK_COMMA: return 0x36;
        case SDLK_PERIOD: return 0x37;
        case SDLK_SLASH: return 0x38;
        case SDLK_CAPSLOCK: return 0x39;
        case SDLK_F1: return 0x3A;
        case SDLK_F2: return 0x3B;
        case SDLK_F3: return 0x3C;
        case SDLK_F4: return 0x3D;
        case SDLK_F5: return 0x3E;
        case SDLK_F6: return 0x3F;
        case SDLK_F7: return 0x40;
        case SDLK_F8: return 0x41;
        case SDLK_F9: return 0x42;
        case SDLK_F10: return 0x43;
        case SDLK_F11: return 0x44;
        case SDLK_F12: return 0x45;
        case SDLK_PRINTSCREEN: return 0x46;
        case SDLK_SCROLLLOCK: return 0x47;
        case SDLK_PAUSE: return 0x48;
        case SDLK_INSERT: return 0x49;
        case SDLK_HOME: return 0x4A;
        case SDLK_PAGEUP: return 0x4B;
        case SDLK_DELETE: return 0x4C;
        case SDLK_END: return 0x4D;
        case SDLK_PAGEDOWN: return 0x4E;
        case SDLK_RIGHT: return 0x4F;
        case SDLK_LEFT: return 0x50;
        case SDLK_DOWN: return 0x51;
        case SDLK_UP: return 0x52;
    }
    return 0;
}

CBcmFrameBuffer::CBcmFrameBuffer(unsigned nWidth, unsigned nHeight)
    : m_nWidth(nWidth), m_nHeight(nHeight) {
    m_pBuffer = new u16[nWidth * nHeight];
    memset(m_pBuffer, 0, nWidth * nHeight * 2);
}

CBcmFrameBuffer::~CBcmFrameBuffer() {
    delete[] m_pBuffer;
}

CScreenDevice::CScreenDevice(unsigned nWidth, unsigned nHeight)
    : m_nWidth(nWidth), m_nHeight(nHeight) {
    m_pFB = new CBcmFrameBuffer(nWidth, nHeight);
    g_pScreenDevice = this;
}

CScreenDevice::~CScreenDevice() {
    delete m_pFB;
    if (g_Texture) SDL_DestroyTexture(g_Texture);
    if (g_Renderer) SDL_DestroyRenderer(g_Renderer);
    if (g_Window) SDL_DestroyWindow(g_Window);
    SDL_Quit();
}

boolean CScreenDevice::Initialize() {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return FALSE;
    }

    g_Window = SDL_CreateWindow("SPC-1000 Emulator (Linux Debug)",
                                SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                m_nWidth * 2, m_nHeight * 2, SDL_WINDOW_SHOWN);
    if (!g_Window) {
        fprintf(stderr, "SDL_CreateWindow failed: %s\n", SDL_GetError());
        return FALSE;
    }

    g_Renderer = SDL_CreateRenderer(g_Window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!g_Renderer) {
        g_Renderer = SDL_CreateRenderer(g_Window, -1, SDL_RENDERER_SOFTWARE);
    }
    if (!g_Renderer) {
        fprintf(stderr, "SDL_CreateRenderer failed: %s\n", SDL_GetError());
        return FALSE;
    }

    g_Texture = SDL_CreateTexture(g_Renderer, SDL_PIXELFORMAT_RGB565, SDL_TEXTUREACCESS_STREAMING, m_nWidth, m_nHeight);
    if (!g_Texture) {
        fprintf(stderr, "SDL_CreateTexture failed: %s\n", SDL_GetError());
        return FALSE;
    }

    return TRUE;
}

void CScreenDevice::Write(const char *pBuffer, unsigned nLength) {}

CBcmFrameBuffer *CScreenDevice::GetFrameBuffer() {
    return m_pFB;
}

void HostUpdate() {
    static unsigned last_draw_time = 0;
    SDL_Event event;
    bool keys_changed = false;

    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            exit(0);
        }
        else if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) {
            // Check for key reset F12 or F10
            if (event.key.keysym.sym == SDLK_F12 || event.key.keysym.sym == SDLK_F10) {
                keys_changed = true;
            }
            unsigned char usb_code = SDLKeyToUSBScanCode(event.key.keysym.sym);
            if (usb_code != 0) {
                if (event.type == SDL_KEYDOWN) {
                    AddActiveKey(usb_code);
                } else {
                    RemoveActiveKey(usb_code);
                }
                keys_changed = true;
            }
        }
    }

    if (keys_changed && g_pKeyboardDevice && g_pKeyboardDevice->m_pHandler) {
        Uint8 modifiers = 0;
        SDL_Keymod mod = SDL_GetModState();
        if (mod & KMOD_LCTRL) modifiers |= 0x01;
        if (mod & KMOD_LSHIFT) modifiers |= 0x02;
        if (mod & KMOD_LALT) modifiers |= 0x04;
        if (mod & KMOD_LGUI) modifiers |= 0x08;
        if (mod & KMOD_RCTRL) modifiers |= 0x10;
        if (mod & KMOD_RSHIFT) modifiers |= 0x20;
        if (mod & KMOD_RALT) modifiers |= 0x40;
        if (mod & KMOD_RGUI) modifiers |= 0x80;

        unsigned char raw_keys[6];
        GetActiveKeys(raw_keys);
        
        // Emulate F12 / F10 key reset inside KeyStatusHandlerRaw
        if (SDL_GetKeyboardState(NULL)[SDL_SCANCODE_F12]) {
            raw_keys[0] = 0x45; // F12 keycode mapped in KeyStatusHandlerRaw
        } else if (SDL_GetKeyboardState(NULL)[SDL_SCANCODE_F10]) {
            raw_keys[0] = 0x43; // F10 keycode
        }
        
        g_pKeyboardDevice->m_pHandler(modifiers, raw_keys);
    }

    unsigned now = SDL_GetTicks();
    if (now - last_draw_time >= 16) {
        last_draw_time = now;
        if (g_Window && g_Renderer && g_Texture && g_pScreenDevice && g_pScreenDevice->GetFrameBuffer()) {
            u16 *pixels = (u16*)g_pScreenDevice->GetFrameBuffer()->GetBuffer();
            SDL_UpdateTexture(g_Texture, NULL, pixels, 320 * 2);
            SDL_RenderClear(g_Renderer);
            SDL_RenderCopy(g_Renderer, g_Texture, NULL, NULL);
            SDL_RenderPresent(g_Renderer);
        }
    }
}
