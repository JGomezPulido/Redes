#include "XLDisplay.h"
#include <stdexcept>

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//
std::unique_ptr<XLDisplay> XLDisplay::_display = nullptr;

//------------------------------------------------------------------------------

Display* XLDisplay::xl_dpy;
Window   XLDisplay::xl_wdw;

GC XLDisplay::xl_gc;

Colormap XLDisplay::xl_cm;
std::vector<int> XLDisplay::xl_colors;

XFontStruct* XLDisplay::xl_font;

const char * XLDisplay::font =  "*-helvetica-*-r-*-*-12-*";
// const char * XLDisplay::font = "*-liberation sans-*-r-*-*-*-*";

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

void XLDisplay::init(int32_t w, int32_t h, const std::string& t)
{
    if (_display != nullptr)
    {
        return;
    }

    XInitThreads();

    xl_dpy = XOpenDisplay(0);

    if(xl_dpy == nullptr)
    {
        throw std::runtime_error("XOpenDisplay: cannot open display.\n");
    }

    int black = BlackPixel(xl_dpy, DefaultScreen(xl_dpy));
    int white = WhitePixel(xl_dpy, DefaultScreen(xl_dpy));

    // (0,0) coordenadas del origen, esquina superior izquierda de la ventana
    // 3 borde de la ventana
    // white, negro (black) borde y blanco (white) fondo de la ventana
    xl_wdw = XCreateSimpleWindow(xl_dpy, DefaultRootWindow(xl_dpy),
            0, 0, w, h, 3, black, white);

    XSetWindowAttributes attr;
    attr.backing_store = Always;

    XChangeWindowAttributes(xl_dpy, xl_wdw, CWBackingStore, &attr);

    XStoreName(xl_dpy, xl_wdw, t.c_str());

    XSelectInput(xl_dpy, xl_wdw, StructureNotifyMask | KeyPressMask);

    // "Mapea" la ventana en la pantalla y crea el "contexto gráfico" asociado
    XMapWindow(xl_dpy, xl_wdw);

    xl_gc = XCreateGC(xl_dpy, xl_wdw, 0, 0);

    XSetForeground(xl_dpy, xl_gc, black);

    //Inicializa el mapa de colores
    xl_cm =  DefaultColormap(xl_dpy, DefaultScreen(xl_dpy));

    std::vector<std::string> named = {"red", "brown", "blue", "yellow", "green"};

    for (auto nc : named)
    {
        XColor tmpc;

        int rc = XAllocNamedColor(xl_dpy, xl_cm, nc.c_str(), &tmpc, &tmpc);

        if (rc == 0)
        {
            throw std::runtime_error("XAllocNamedColor: cannot allocate" + nc + "\n");
        }

        xl_colors.push_back(tmpc.pixel);
    }

    xl_colors.push_back(white);

    xl_colors.push_back(black);

    // Carga la fuente por defecto
    xl_font = XLoadQueryFont(xl_dpy, const_cast<char *>(font));

    if ( xl_font == nullptr )
    {
        throw std::runtime_error("XLoadQueryFont: cannot load font.\n");
    }

    XSetFont(xl_dpy, xl_gc, xl_font->fid);

    // Espera al evento MapNotify
    while (true)
    {
        XEvent e;

        XNextEvent(xl_dpy, &e);

        if (e.type == MapNotify)
            break;
    }

    //Inicializa el puntero singleton
    _display = std::unique_ptr<XLDisplay>(new XLDisplay);
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

char XLDisplay::wait_key()
{
    XEvent event;

    while(true)
    {
        XNextEvent(xl_dpy, &event);

        if ( event.type != KeyPress )
        {
            continue;
        }

        return XLookupKeysym(&event.xkey,0);
    }
}

