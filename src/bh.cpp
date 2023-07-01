#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
namespace py = pybind11;

#include "particle_system.h"
#include "syncable.h"


struct MultithreadedParticleSystem : ParticleSystem {
    MultithreadedParticleSystem(const int num_particles, const double bounds, const int seed, const double theta, const double dt, const std::size_t num_threads):
        ParticleSystem(num_particles, bounds, theta, seed),
        delta_time(dt),
        pool(num_threads)
    {
        auto slice_size = num_particles / num_threads;
        for (int i = 0; i < num_threads; ++i)
        {
            callables.emplace_back(
                std::bind(
                    &ParticleSystem::collect_forces,
                    std::ref(*this),
                    i * slice_size,
                    slice_size
                )
            );
        }
        pool.initialize(callables);
    }

    void update() {
        build_tree();
        pool.trigger();
        integrate(delta_time);
        simulation_time += delta_time;
    }

    std::vector<std::function<void(void)>> callables;
    double simulation_time = 0.0;
    double delta_time = 1.0;

    Syncable pool;
};

PYBIND11_MODULE(ParticleModel, m) {
    py::class_<MultithreadedParticleSystem>(m, "MultithreadedParticleSystem")
        .def(py::init<const int, const double, const int, const double, const double, const std::size_t>())
        .def("update", &MultithreadedParticleSystem::update)
        .def("get_extents", &MultithreadedParticleSystem::get_extents)
        .def_readwrite("ll", &MultithreadedParticleSystem::ll)
        .def_readwrite("ur", &MultithreadedParticleSystem::ur)
        .def_readwrite("simulation_time", &MultithreadedParticleSystem::simulation_time)
        .def_readwrite("particles", &MultithreadedParticleSystem::particles);

    py::class_<Particle>(m, "Particle")
        .def_readwrite("x", &Particle::x)
        .def_readwrite("y", &Particle::y)
        .def_readwrite("vx", &Particle::vx)
        .def_readwrite("vy", &Particle::vy)
        .def_readwrite("m", &Particle::m);
}

