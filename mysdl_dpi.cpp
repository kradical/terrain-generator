#include "mysdl_dpi.h"

#include <SDL2/SDL.h>

void MySDL_GetDisplayDPI(int displayIndex, float* hdpi, float* vdpi, float* defaultDpi)
{
    static const float kSysDefaultDpi =
#ifdef __APPLE__
        72.0f;
#elif defined(_WIN32)
        96.0f;
#else
        96.0f;
#endif

    if (SDL_GetDisplayDPI(displayIndex, NULL, hdpi, vdpi))
    {
        if (hdpi) *hdpi = kSysDefaultDpi;
        if (vdpi) *vdpi = kSysDefaultDpi;
    }

    if (defaultDpi) *defaultDpi = kSysDefaultDpi;
}
