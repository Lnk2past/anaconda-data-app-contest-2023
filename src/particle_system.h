#pragma once

#include <array>
#include <random>
#include <vector>

#include "particle.h"
#include "quadtree.h"


struct ParticleSystem {
    std::array<double, 2> ll {-1, -1};
    std::array<double, 2> ur {1, 1};
    QuadTree qt;
    double theta;

    ParticleSystem(const int num_particles, const double bounds, const double default_theta, const int seed=1337):
        ll {-bounds, -bounds},
        ur {bounds, bounds},
        theta (default_theta)
    {
        std::mt19937 eng(seed);
        std::uniform_real_distribution<double> dis(-bounds, bounds);
        particles.reserve(num_particles);
        for (auto i = 0; i < num_particles-1; ++i) {
            auto &p = particles.emplace_back(dis(eng), dis(eng));
        }
        particles.emplace_back(0, 0, 0, 0, 0, 0, 1e12);
    }

    void build_tree()
    {
        qt = {.theta=theta, .ll=ll, .ur=ur};
        for (auto &e : particles)
        {
            qt.add(e);
        }
        qt.get_cogs();
    }

    void collect_forces(std::size_t start, std::size_t count)
    {
        for (auto i = start; i < start + count; ++i) {
            qt.force(particles[i]);
        }
    }

    void integrate(const double delta_time) {
        double bounds = 0.0;
        for (auto &e : particles)
        {
            e.integrate(delta_time);

            if (std::abs(e.x) > bounds)
            {
                bounds = std::abs(e.x);
            }
            if (std::abs(e.y) > bounds)
            {
                bounds = std::abs(e.y);
            }
        }
        ll = {-bounds, -bounds};
        ur = {bounds, bounds};
    }

    std::vector<std::array<double, 4>> get_extents()
    {
        std::vector<std::array<double, 4>> extents;
        qt.get_extents(extents);
        return extents;
    }

    std::vector<Particle> particles;
};
