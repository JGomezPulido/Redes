#include <X11/Xlib.h>
#include <stdlib.h>
#include <string>
#include <stdexcept>
#include <memory>
#include <vector>

#ifndef _XL_DISPLAY
#define _XL_DISPLAY

// -----------------------------------------------------------------------------
// Esta clase ofrece un interfaz sencillo para interacciones sencillas con el
// servidor gráfico del sistema X.
//
// Referencia: https://tronche.com/gui/x/xlib/
// -----------------------------------------------------------------------------
class XLDisplay
{
public:
    enum XLColor
    {
        RED    = 0,
        BROWN  = 1,
        BLUE   = 2,
        YELLOW = 3,
        GREEN  = 4,
        WHITE  = 5,
        BLACK  = 6
    };

    // -------------------------------------------------------------------------
    // Constructores. Implementa el patrón singleton
    // -------------------------------------------------------------------------
    // El programa principal debe llamar a la función init, ej.
    //
    //  XLDisplay::init(200, 300, "Título Ventana);
    //  ...
    //  XLDisplay& dpy = XLDisplay::display();
    //  ...
    //  dpy.circle(14, 14, 5);
    // -------------------------------------------------------------------------
    XLDisplay(const XLDisplay&) = delete;

    XLDisplay(const XLDisplay&&) = delete;

    XLDisplay& operator=(const XLDisplay&) = delete;

    static void init(int32_t w, int32_t h, const std::string& t);

    ~XLDisplay()
    {
        XFreeColormap(xl_dpy, xl_cm);
    }

    static XLDisplay& display()
    {
        return *_display.get();
    }

    // -------------------------------------------------------------------------
    // Funciones de render
    // -------------------------------------------------------------------------
    void set_color(XLColor c)
    {
        XSetForeground(xl_dpy, xl_gc, xl_colors[c]);
    }

    // Dibuja un punto en (x,y)
    void point( int32_t x, int32_t y )
    {
        XDrawPoint(xl_dpy, xl_wdw, xl_gc, x, y);
    }

    // Dibuja una línea desde (x1,y1) a (x2,y2)
    void line( int32_t x1, int32_t y1, int32_t x2, int32_t y2 )
    {
        XDrawLine(xl_dpy, xl_wdw, xl_gc,x1,y1,x2,y2);
    }

    // Dibuja una serie de líneas cuyos vértices están dados en un array de
    // estructuras XPoint.
    //
    // XPoint pts [] = {{0,0},{15,15},{0,15},{0,0}}
    // int npoints = 4;
    void lines(XPoint * points, int npoints)
    {
        XDrawLines(xl_dpy, xl_wdw, xl_gc, points, npoints, CoordModeOrigin);
    }

    // Dibuja un círculo con centro en (x,y) y radio r
    void circle(int32_t x, int32_t y, int32_t r)
    {
        XDrawArc(xl_dpy, xl_wdw, xl_gc, x-r, y - r, 2 * r, 2 * r, 0, 360*64);
    }

    // Dibuja un rectángulo con vértice superior-izquierdo en (x,y), ancho w
    // y alto h
    void rectangle(int32_t x, int32_t y, int32_t w, int32_t h, bool filled)
    {
        if (filled)
        {
            XFillRectangle(xl_dpy, xl_wdw, xl_gc, x, y, w, h);
        }
        else
        {
            XDrawRectangle(xl_dpy, xl_wdw, xl_gc, x, y, w, h);
        }

    }

    // Escribe un texto en la posición (x,y)
    void text(int32_t x, int32_t y, const std::string& txt)
    {
        XDrawString(xl_dpy, xl_wdw, xl_gc, x, y, txt.c_str(), txt.length());
    }

    // Vacía el buffer enviando los comandos al servidor y dibuja la ventana
    void flush()
    {
        XFlush(xl_dpy);
    }

    // Borra la ventana
    void clear()
    {
        XClearWindow(xl_dpy, xl_wdw);
    }

    // -------------------------------------------------------------------------
    // Eventos de entrada (teclado)
    // -------------------------------------------------------------------------

    //Espera por la pulsación de una tecla, que devuelve la función
    char wait_key();

private:
    XLDisplay() = default;

    static const char * font;

    static std::unique_ptr<XLDisplay> _display;

    // -------------------------------------------------------------------------
    // Xlib variables
    // -------------------------------------------------------------------------
    static Display * xl_dpy;

    static Window    xl_wdw;

    static GC        xl_gc;

    static Colormap  xl_cm;

    static XFontStruct* xl_font;

    static std::vector<int> xl_colors;
};

#endif
