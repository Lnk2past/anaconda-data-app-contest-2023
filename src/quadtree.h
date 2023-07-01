#pragma once

#include <array>
#include <iostream>
#include <memory>

#include "particle.h"

struct QuadTree
{
    double theta = 0.5;

    std::array<double, 2> ll {-1.0, -1.0};
    std::array<double, 2> ur {1.0, 1.0};

    Particle *particle {nullptr};

    std::unique_ptr<QuadTree> ne {nullptr};
    std::unique_ptr<QuadTree> nw {nullptr};
    std::unique_ptr<QuadTree> sw {nullptr};
    std::unique_ptr<QuadTree> se {nullptr};

    std::array<double, 2> center {0.0, 0.0};
    double m {0.0};

    std::unique_ptr<QuadTree>& _get_quadrant(Particle &e)
    {
        double dxh = 0.5 * (ur[0] + ll[0]);
        double dyh = 0.5 * (ur[1] + ll[1]);
        if (e.x > dxh && e.y >= dyh)
        {
            if (!ne)
            {
                ne.reset(new QuadTree {theta, {dxh, dyh}, ur});
            }
            return ne;
        }
        else if (e.x <= dxh && e.y > dyh)
        {
            if (!nw)
            {
                nw.reset(new QuadTree {theta, {ll[0], dyh}, {dxh, ur[1]}});
            }
            return nw;
        }
        else if (e.x < dxh && e.y <= dyh)
        {
            if (!sw)
            {
                sw.reset(new QuadTree {theta, ll, {dxh, dyh}});
            }
            return sw;
        }
        else
        {
            if (!se)
            {
                se.reset(new QuadTree {theta, {dxh, ll[1]}, {ur[0], dyh}});
            }
            return se;
        }
    }

    void _subdivide(Particle &e)
    {
        auto &existing_particle_quadrant = _get_quadrant(*particle);
        auto _particle = particle;
        particle = nullptr;
        existing_particle_quadrant->add(*_particle);

        auto &new_particle_quadrant = _get_quadrant(e);
        new_particle_quadrant->add(e);
    }

    void add(Particle &e)
    {
        if (ne || nw || sw || se)
        {
            auto &particle_quadrant = _get_quadrant(e);
            particle_quadrant->add(e);
        }
        else if (particle)
        {
            _subdivide(e);
        }
        else
        {
            particle = &e;
        }
    }

    void get_cogs()
    {
        if (particle)
        {
            m = particle->m;
            center = {particle->x, particle->y};
        }
        else
        {
            m = 0.0;
            center = {0.0, 0.0};
            if (ne)
            {
                ne->get_cogs();
                center[0] += ne->center[0] * ne->m;
                center[1] += ne->center[1] * ne->m;
                m += ne->m;
            }
            if (nw)
            {
                nw->get_cogs();
                center[0] += nw->center[0] * nw->m;
                center[1] += nw->center[1] * nw->m;
                m += nw->m;
            }
            if (sw)
            {
                sw->get_cogs();
                center[0] += sw->center[0] * sw->m;
                center[1] += sw->center[1] * sw->m;
                m += sw->m;
            }
            if (se)
            {
                se->get_cogs();
                center[0] += se->center[0] * se->m;
                center[1] += se->center[1] * se->m;
                m += se->m;
            }
            center[0] /= m;
            center[1] /= m;
        }
    }

   void force(Particle &e)
   {
        if (particle)
        {
            if (particle != &e)
            {
               e.force(*particle);
            }
        }
        else
        {
            double dx = center[0] - e.x;
            double dy = center[1] - e.y;
            double d = std::hypot(dx, dy);

            if ((ur[0] - ll[0]) / d < theta)
            {
                e.force(dx, dy, m);
            }
            else
            {
                if (ne)
                {
                    ne->force(e);
                }
                if (nw)
                {
                    nw->force(e);
                }
                if (sw)
                {
                    sw->force(e);
                }
                if (se)
                {
                    se->force(e);
                }
            }
        }
    }

    void get_extents(std::vector<std::array<double, 4>> &extents)
    {
        if (particle)
        {
            extents.push_back({ll[0], ll[1], ur[0], ur[1]});
        }
        if (ne)
        {
            ne->get_extents(extents);
        }
        if (nw)
        {
            nw->get_extents(extents);
        }
        if (sw)
        {
            sw->get_extents(extents);
        }
        if (se)
        {
            se->get_extents(extents);
        }
    }

    void print()
    {
        if (ne)
        {
            ne->print();
        }
        if (nw)
        {
            nw->print();
        }
        if (sw)
        {
            sw->print();
        }
        if (se)
        {
            se->print();
        }
        if (particle)
        {
            std::cout << ll[0] << " " << ll[1] << " " << ur[0] << " " << ur[1] << std::endl;
        }
    }
};
