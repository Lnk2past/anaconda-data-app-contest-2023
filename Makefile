all:
	g++ -shared -fPIC -std=c++20 -isystem$(CONDA_PREFIX)/include -isystem$(CONDA_PREFIX)/include/python3.11 -Isrc src/bh.cpp -o app/ParticleModel$(shell python3-config --extension-suffix)
