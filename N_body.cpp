#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string>
#include <random>
#include "vector.h"
#include "body.h"
#include "util.h"
#include <chrono>

#define KAPPA 6.673e-11
#define EPS 1e-3
#define ITERS 10000000
#define DELTA_T 2.0
#define FRAMES 2000
#define DEBUG false

Vector force(Body &b_1, Body &b_2)
{
    Vector diff = b_1.pos - b_2.pos;
    double dist = diff.length() + EPS;

    return diff * ((b_1.m * b_2.m) / (dist * dist * dist) * -KAPPA);
}

Vector acceleration(Body &b_1, Body &b_2)
{
    Vector diff = b_1.pos - b_2.pos;
    double dist = diff.length() + EPS;

    return diff * ((b_2.m) / (dist * dist * dist) * -KAPPA);
}

void print_states(double N, Body* bodies)
{
    for (int i = 0; i < N; ++i)
    {
        printf("Body %d\n", i);
        printf("Current position: ");
        bodies[i].pos.print_vector();
        printf("Current velocity: ");
        bodies[i].vel.print_vector();
    }
}

int main(int argc, char* argv[])
{
    int N;
    Body *bodies, *bodies_new;
    read_input(&N, &bodies, &bodies_new);

    Vector* log = new Vector[FRAMES * N * 2];

    if (DEBUG)
    {
        print_states(N, bodies);
        fflush(stdout);
    }

    auto time_start = std::chrono::steady_clock::now();

    int frame = 0;
    for (int iter = 0; iter < ITERS; ++iter)
    {

        for (int i = 0; i < N; ++i)
        {
            Vector accel_sum = Vector();
            for (int j = 0; j < N; ++j)
            {   
                if (i != j)
                {
                    accel_sum += acceleration(bodies[i], bodies[j]);
                }
            }

            bodies_new[i].pos = bodies[i].pos + bodies[i].vel * DELTA_T + accel_sum * (0.5 * DELTA_T * DELTA_T);
            bodies_new[i].vel = bodies[i].vel + accel_sum * DELTA_T;
        }

        if (iter % (ITERS / FRAMES) == 0) {
            for (int i = 0; i < N; ++i) {
                log[(frame * N + i) * 2 + 0] = Vector(bodies_new[i].pos);
                log[(frame * N + i) * 2 + 1] = Vector(bodies_new[i].vel);
            }
            ++frame;
        }

        Body* tmp = bodies_new;
        bodies_new = bodies;
        bodies = tmp;
    }

    auto time_end = std::chrono::steady_clock::now();
    double time = std::chrono::duration<double>(time_end - time_start).count();

    write_output(N, FRAMES, log);

    if (DEBUG)
        print_states(N, bodies);

    printf("Required time: %lfs\n", time);
}

/*
g++ N_body.cpp -o N_body
srun --ntasks=1 --nodes=1 --reservation=fri --mpi=pmix N_body 3
*/