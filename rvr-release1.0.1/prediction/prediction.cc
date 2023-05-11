#include <memory>
#include <iostream>
#include <mutex>
#include <condition_variable>
#include <cstdint>
#include <thread>
#include <tuple>
#include <map>
#include <chrono>
#include <string>

#include <unistd.h>

#include "XLDisplay.h"

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

// Interpolación en el jugador 2
bool interpolation = false;

// Predicción en el jugador 1
bool prediction    = false;

// Tick interno de simulación
#define SIM_TICK 35

// define la frecuencia de actualizaciones que envía el servidor
#define UPDATE_FREQ 5

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

enum input_t : uint8_t { none = 0, left = 1, right = 2};

using state_t = std::tuple<uint32_t, input_t>;

using state_buffer_t = std::map<uint32_t, state_t>;


/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

class InputThread
{
public:
    void input()
    {
        XLDisplay& dpy = XLDisplay::display();

        while(true)
        {
            char k = dpy.wait_key();

            imutex.lock();

            switch (k)
            {
                case 'a':
                    key   = input_t::left;
                    ready = true;
                break;

                case 'd':
                    key = input_t::right;
                    ready = true;
                break;

                default:

                break;
            }

            imutex.unlock();
        }
    }

    bool read_input(input_t &i)
    {
        std::unique_lock<std::mutex> lock(imutex);

        bool r = ready;
        i      = key;

        ready = false;

        return r;
    }

private:
    std::mutex imutex;
    std::condition_variable icv;

    input_t key;
    bool    ready = false;
};

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

class Network
{
public:

    enum host_t { p1 = 0, p2 = 1, server = 2 };

    Network(uint32_t _lat1, uint32_t _lat2, uint32_t tick_time)
    {
        lat1 = _lat1 / tick_time;
        lat2 = _lat2 / tick_time;
    }

    void send_message(uint32_t t, host_t from, host_t to, state_t s_pair)
    {
        if ( from == host_t::p1 || to == host_t::p1 )
        {
            t = t + lat1;
        }
        else if ( from == host_t::p2 || to == host_t::p2 )
        {
            t = t + lat2;
        }

        switch (to)
        {
            case host_t::p1:
                p1_in.insert(std::pair<uint32_t, state_t>(t, s_pair));
                break;

            case host_t::p2:
                p2_in.insert(std::pair<uint32_t, state_t>(t, s_pair));
                break;

            case host_t::server:
                server_in.insert(std::pair<uint32_t, state_t>(t, s_pair));
                break;
        }
    }

    bool recv_message(uint32_t& t, host_t h, state_t& s)
    {
        state_buffer_t * sb;
        uint32_t lat;

        switch (h)
        {
            case host_t::p1:
                sb  = &p1_in;
                lat = lat1;
                break;

            case host_t::p2:
                sb  = &p2_in;
                lat = lat2;
                break;

            case host_t::server:
                sb  = &server_in;
                lat = 0;
                break;
        }

        auto msg_it = sb->find(t);

        if ( msg_it == sb->end() )
        {
            return false;
        }

        t -= lat;

        s = msg_it->second;

        return true;
    }

private:
    uint32_t lat1;
    uint32_t lat2;

    state_buffer_t p1_in;
    state_buffer_t p2_in;
    state_buffer_t server_in;
};

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */



class Player
{
public:
    Player(uint32_t tick_ini, uint32_t x_ini, uint32_t _y, XLDisplay::XLColor _c)
        :y(_y), c(_c)
    {
        insert_state(tick_ini, x_ini);
    };

    void insert_state(uint32_t tick, uint32_t x, input_t i = input_t::none)
    {
        state_t s_pair(x, i);

        states.insert(std::pair<uint32_t, state_t>(tick, s_pair));
    };

    void render(uint32_t tick)
    {
        XLDisplay& dpy = XLDisplay::display();

        uint32_t x;

        auto s_it = states.find(tick);

        if ( s_it == states.end() )
        {
            auto last = states.rbegin();

            x = std::get<0>(last->second);
        }
        else
        {
            x = std::get<0>(s_it->second);
        }

        dpy.set_color(c);

        dpy.rectangle(x, y, 45, 45, true);

    }

    void move(uint32_t tick, input_t k)
    {
        auto     s_it = states.find(tick - 1);
        uint32_t x    = std::get<0>(s_it->second);

        switch(k)
        {
            case input_t::left:
                x -= 5;
            break;
            case input_t::right:
                x += 5;
            break;
            case input_t::none:
            break;
        }

        insert_state(tick, x, k);
    }

    state_t get_state(uint32_t tick)
    {
        auto s_it = states.find(tick);

        return s_it->second;
    }

    void interpolate(uint32_t ini, uint32_t fin, state_t msg)
    {
        state_t  ini_state = get_state(ini);

        int32_t x_ini = (int32_t) std::get<0>(ini_state);
        int32_t x_fin = (int32_t) std::get<0>(msg);

        int32_t steps = fin - ini;
        int32_t delta = x_fin - x_ini;

        int32_t inc = delta /steps;
        uint32_t x  = std::get<0>(ini_state) + inc;

        for (uint32_t t=ini+1 ; t <= fin; t++, x+=inc)
        {
            insert_state(t, x, input_t::none);
        }
    }

private:

    uint32_t y;

    XLDisplay::XLColor c;

    state_buffer_t states;
};

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

class Simulation
{
public:
    Simulation(uint32_t p1_lat, uint32_t p2_lat):
        tick(1),
        net(p1_lat, p2_lat, SIM_TICK),
        server(0, 100, 111, XLDisplay::BROWN),
        p1(0, 100, 45, XLDisplay::GREEN),
        p2(0, 100, 177, XLDisplay::RED)
    {
        // Desplaza el tick en el jugador 2 para poder usar el mismo tick
        p2_tick_delta = (p2_lat / SIM_TICK) + UPDATE_FREQ;

        ithread.reset(new std::thread([&](){it.input();}));
    }

    void server_step()
    {
        static uint32_t send = 0;

        state_t msg;
        input_t p1_input = input_t::none;

        if ( net.recv_message(tick, Network::server, msg) )
        {
            p1_input = std::get<1>(msg);
        }

        server.move(tick, p1_input);

        if ( send == UPDATE_FREQ )
        {
            send = 0;

            state_t current = server.get_state(tick);

            net.send_message(tick, Network::server, Network::p1, current);
            net.send_message(tick, Network::server, Network::p2, current);
        }

        send++;
    }

    void player_one_step()
    {
        input_t p1_key;

        bool kr = it.read_input(p1_key);

        if (kr)
        {
            state_t istate(0, p1_key);

            net.send_message(tick, Network::p1, Network::server, istate);
        }
        else
        {
            p1_key = input_t::none;
        }

        /* --------------------------- */
        /* Prediction                  */
        /* --------------------------- */
        if (prediction)
        {
            p1.move(tick, p1_key);
        }

        uint32_t msg_tick = tick;

        state_t msg;

        if ( net.recv_message(msg_tick, Network::p1, msg) )
        {
            if (prediction)
            {
                state_t past_state = p1.get_state(msg_tick);

                if ( std::get<0>(past_state) != std::get<1>(msg) )
                {
                    /* --------------------------- */
                    /* Correction                  */
                    /* --------------------------- */
                }
            }
            else
            {
                p1.insert_state(tick, std::get<0>(msg));
            }
        }
    }

    void player_two_step()
    {
        static uint32_t last_prev_tick = 0;
        static uint32_t last_tick      = 0;

        uint32_t msg_tick = tick;

        state_t msg;

        if ( net.recv_message(msg_tick, Network::p2, msg) )
        {
            if (interpolation)
            {
                /* --------------------------- */
                /* Interpolation               */
                /* --------------------------- */
                last_prev_tick = last_tick;
                last_tick      = msg_tick;

                p2.interpolate(last_prev_tick, last_tick, msg);
            }
            else
            {
                p2.insert_state(tick, std::get<0>(msg));
            }
        }
    }

    void loop()
    {
        while(true)
        {
            player_one_step();

            server_step();

            player_two_step();

            render();

            std::this_thread::sleep_for(std::chrono::milliseconds(SIM_TICK));

            tick++;
        }
    }

    void render()
    {
        XLDisplay& dpy = XLDisplay::display();

        dpy.clear();

        dpy.set_color(XLDisplay::BLACK);

        dpy.text(10,95, "Jugador 1");

        dpy.line(0,101,500,101);

        dpy.text(10, 161, "Servidor");

        dpy.line(0,167,500,167);

        dpy.text(10, 230, "Jugador 2");

        server.render(tick);

        p1.render(tick);

        if ( interpolation )
        {
            if ( tick < p2_tick_delta )
            {
                p2.render(0);

            }
            else
            {
                p2.render(tick - p2_tick_delta);
            }

        }
        else
        {
            p2.render(tick);
        }

        dpy.flush();
    }

private:
    uint32_t tick;
    uint32_t p2_tick_delta;

    Network net;

    InputThread it;

    Player server;
    Player p1;
    Player p2;

    std::unique_ptr<std::thread> ithread;
};


/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

int main(int argc, char **argv)
{
    int c;

    std::string lat1_s;
    std::string lat2_s;

    XLDisplay::init(100, 200, "RVR Modulo 3");

    while ((c = getopt(argc, argv, "1:2:pih")) != -1)
    {
        switch (c)
        {
            case 'p':
                prediction = true;
                break;
            case 'i':
                interpolation = true;
                break;
            case '1':
                lat1_s = optarg;
                break;
            case '2':
                lat2_s = optarg;
                break;
            case '?':
            case 'h':
                std::cerr << "Uso prediccion -1 <latencia jug1> -2 <latencia jug2> -p -i\n";
                return -1;
        }
    }

    std::cout << "P1 Latencia: " << lat1_s << std::endl;
    std::cout << "P2 Latencia: " << lat2_s << std::endl;
    std::cout << "Interpolación: " << interpolation << std::endl;
    std::cout << "Predicción: " << prediction << std::endl;

    Simulation sim(std::stoi(lat1_s), std::stoi(lat2_s));

    sim.loop();

    return 0;
};

