#pragma once

#include <cmath>
#include <iostream>

constexpr double G = 6.67408e-11;

struct Particle
{
    double x = 0.0;
    double y = 0.0;
    double vx = 0.0;
    double vy = 0.0;
    double ax = 0.0;
    double ay = 0.0;
    double m = 5.0e6;

    void force(const Particle &o)
    {
        double dx = o.x - x;
        double dy = o.y - y;
        force(dx, dy, o.m);
    }

    void force(const double dx, const double dy, const double omass)
    {
        double d = std::hypot(dx, dy);
        double t = std::atan2(dy, dx);
        double f = G * omass / (d * d);
        ax += f * std::cos(t);
        ay += f * std::sin(t);
    }

    void integrate(const double dt)
    {
        vx += ax * dt;
        vy += ay * dt;
        x += vx * dt;
        y += vy * dt;
        ax = 0.0;
        ay = 0.0;
    }

    void print()
    {
        std::cout << "<" << x << "," << y << ">" << std::endl;
    }
};