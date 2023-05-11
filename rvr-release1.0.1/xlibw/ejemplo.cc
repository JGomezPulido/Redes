#include "XLDisplay.h"
#include <unistd.h>

void draw_forms()
{
    XLDisplay& dpy = XLDisplay::display();

    dpy.set_color(XLDisplay::RED);

    dpy.point(50,50);

    dpy.line(20,20,100,50);

    dpy.set_color(XLDisplay::BROWN);

    dpy.circle(45,45,15);

    dpy.set_color(XLDisplay::BLUE);

    dpy.rectangle(60,60,30,15);

    XPoint pts[] = {{100,100},{130,130},{100,130},{100,100}};

    dpy.set_color(XLDisplay::YELLOW);

    dpy.lines(pts, 4);

    dpy.set_color(XLDisplay::GREEN);

    dpy.text(90, 80, "HOLA MUNDO!");

    dpy.flush();
}

void wait()
{
    XLDisplay& dpy = XLDisplay::display();

    char k;

    do
    {
         k = dpy.wait_key();
    } while (k != 'q');

    dpy.clear();

    dpy.flush();

    sleep(1);
}


int main()
{
    XLDisplay::init(100, 200, "Ejemplo");

    draw_forms();

    wait();

    return 0;
}
