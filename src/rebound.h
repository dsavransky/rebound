/**
 * @file 	main.h
 * @brief 	Main header file.
 * @author 	Hanno Rein <hanno@hanno-rein.de>
 * 
 * @section 	LICENSE
 * Copyright (c) 2015 Hanno Rein, Shangfei Liu
 *
 * This file is part of rebound.
 *
 * rebound is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * rebound is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with rebound.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#ifndef _MAIN_H
#define _MAIN_H
#ifndef M_PI
// Make sure M_PI is defined. 
#define M_PI           3.14159265358979323846
#endif
#include "integrator_ias15.h"
#include "integrator_whfast.h"
#include "integrator_sei.h"
#include "integrator_wh.h"
#include "integrator_hybrid.h"


/**
 * Generic 3d vector
 */
struct reb_vec3d {
	double x;
	double y;
	double z;
};

/**
 * This struct containes the relative position and velocity of a boundary box.
 * It is sometimes also used as the relative position and velocity of a 
 * particle to speed up calculation.
 */
struct reb_ghostbox{
	double shiftx;		/**< Relative x position */
	double shifty;		/**< Relative y position */
	double shiftz;		/**< Relative z position */
	double shiftvx;		/**< Relative x velocity */
	double shiftvy;		/**< Relative y velocity */
	double shiftvz;		/**< Relative z velocity */
};


/**
 * reb_particle structure.
 * @details This structure is used to represent one particle. Additional particle
 * properties should be added here. Note that when the data structure is changed, 
 * one must also update the equivalent declaration for MPI in communications_mpi.c.
 */
struct reb_particle {
	double x;	/**< x-position of the particle. */
	double y;	/**< y-position of the particle. */
	double z;	/**< z-position of the particle. */
	double vx;	/**< x-velocity of the particle. */
	double vy;	/**< y-velocity of the particle. */
	double vz;	/**< z-velocity of the particle. */
	double ax;	/**< x-acceleration of the particle. */
	double ay;	/**< y-acceleration of the particle. */
	double az;	/**< z-acceleration of the particle. */
	double m;	/**< Mass of the particle. */
	double r; 	/**< Radius of the particle. */
	double lastcollision;	/**< Last time the particle had a physical collision. */
	struct reb_treecell* c;		/**< Pointer to the cell the particle is currently in. */
	int id;		/**< Unique id to identify particle. */
};


/**
 * Struct representing a Keplerian orbit.
 */
struct reb_orbit {
	double a;
	double r;	// Radial distance from central object
	double h;	// Angular momentum
	double P;	// Orbital period
	double l;
	double e;
	double inc;
	double Omega; 	// longitude of ascending node
	double omega; 	// argument of perihelion
	double f; 	// true anomaly
};

/**
 * Collision structure of one single collisions
 * Used to save a collision during collision search. 
 */
struct reb_collision{
	int p1;			/**< First colliding particle. */
	int p2;			/**< Second colliding particle. */
	struct reb_ghostbox gb;	/**< Ghostbox (of particle p1). */
#if defined(COLLISIONS_SWEEP) || defined(COLLISIONS_SWEEPPHI)
	double time;		/**< Time of collision. */
	int crossing;		/**< Collision occurs at the interface of two sweep boxes. */
#endif // COLLISIONS_SWEEP
	int ri;	 		/**< Index of rootcell (Needed for MPI). */
};

/**
 * Main struct representing an entire REBOUND simulation.
 */
struct reb_simulation {
	double 	t;			/**< Current simulation time. */
	double 	G;			/**< Gravitational constant. Default: 1. */
	double 	softening;		/**< Gravitational softening parameter. Default: 0. */
	double 	dt;			/**< Current timestep. */
	double 	dt_last_done;		/**< Last full timestep (used if exact_finish_time==1). */
	int 	N;			/**< Current number of particles on this node. */
	int 	N_var;			/**< Number of variational particles. Default: 0.*/
	int 	N_active;		/**< Number of massive particles included in force calculation. Default: N.*/
	int 	allocatedN;		/**< Current maximum space allocated in the particles array on this node. */
	int 	exit_simulation;	/**< Set to 1 to exit the simulation at the end of the next timestep. */
	int 	exact_finish_time; 	/**< Set to 1 to finish the integration exactly at tmax. Set to 0 to finish at the next dt. */

	unsigned int force_is_velocitydependent; 	/**< Set to 1 if integrator needs to consider velocity dependent forces. */ 
	unsigned int gravity_ignore_10;			/**< Ignore the gravity form the central object (for WH-type integrators)*/
	double reb_output_timing_last; 			/**< Time when reb_output_timing() was called the last time. */

	//////////////////////////////////////////////
	/// Boxes
	struct  reb_vec3d boxsize;	/**< Size of the entire box, root_x*boxsize. */
	double 	boxsize_max;		/**< Maximum size of the entire box in any direction. Set in box_init().*/
	double  root_size;		/**< Size of a root box. */
	int 	root_n;			/**< Total number of root boxes in all directions, root_nx*root_ny*root_nz. Default: 1. Set in box_init().*/
	int 	root_nx;		/**< Number of root boxes in x direction. Default: 1. */
	int 	root_ny;		/**< Number of root boxes in y direction. Default: 1. */
	int 	root_nz;		/**< Number of root boxes in z direction. Default: 1. */
	int	nghostx;		/**< Number of ghostboxes in x direction. */
	int 	nghosty;		/**< Number of ghostboxes in y direction. */
	int 	nghostz;		/**< Number of ghostboxes in z direction. */

	//////////////////////////////////////////////
	/// Collisions
	struct reb_collision* collisions;		/**< Array of all collisions. */
	int collisions_allocatedN;			/**< Size allocated for collisions.*/
	double minimum_collision_velocity;		/**< Used for hard sphere collision model. */
	double collisions_plog;				/**< Keep track of momentum exchange (used to calculate collisional viscosity in ring systems. */
	double max_radius[2];				/**< Two largest particle radii. Needed for collision search. */
	long collisions_Nlog;				/**< Keep track of Number of collisions. */

	//////////////////////////////////////////////
	/// MEGNO
	int calculate_megno;	// Flag that determines if megno is calculated (default=0, but megno_init() sets it to 1)
	double megno_Ys;
	double megno_Yss;
	double megno_cov_Yt;	// covariance of <Y> and t
	double megno_var_t;  	// variance of t 
	double megno_mean_t; 	// mean of t
	double megno_mean_Y; 	// mean of Y
	long   megno_n; 	// number of covariance updates

	//////////////////////////////////////////////
	/// Modules
	
	/**
	 * Available collision routines.
	 */
	enum {
		REB_COLLISION_NONE = 0,
		REB_COLLISION_DIRECT = 1,
		REB_COLLISION_TREE = 2,
		} collision;
	/**
	 * Available integrators.
	 */
	enum {
		REB_INTEGRATOR_IAS15 = 0,
		REB_INTEGRATOR_WHFAST = 1,
		REB_INTEGRATOR_SEI = 2,
		REB_INTEGRATOR_WH = 3,
		REB_INTEGRATOR_LEAPFROG = 4,
		REB_INTEGRATOR_HYBRID = 5,
		REB_INTEGRATOR_NONE = 6,
		} integrator;

	/**
	 * Available boundary conditions.
	 */
	enum {
		REB_BOUNDARY_NONE = 0,
		REB_BOUNDARY_OPEN = 1,
		REB_BOUNDARY_PERIODIC = 2,
		REB_BOUNDARY_SHEAR = 3,
		} boundary;

	/**
	 * Available gravity routines.
	 */
	enum {
		REB_GRAVITY_NONE = 0,
		REB_GRAVITY_BASIC = 1,
		REB_GRAVITY_COMPENSATED = 2,
		REB_GRAVITY_TREE = 3,
		} gravity;



	//////////////////////////////////////////////
	/// reb_particles
	struct reb_particle* particles; 			/**< Main particle array. This contains all particles on this node.  */
	struct reb_vec3d* gravity_cs;				/**< Vector containing the information for compensated gravity summation */
	int gravity_cs_allocatedN;				/**< Current number of allocated space for cs array*/
	
	//////////////////////////////////////////////
	/// Tree
	struct reb_treecell** tree_root; 			/**< Pointer to the roots of the trees. */
	double opening_angle2;	 			/**< Square of the cell opening angle \f$ \theta \f$. */

	
	//////////////////////////////////////////////
	/// Integrators
	struct reb_simulation_integrator_whfast ri_whfast;		/**< The WHFast struct */
	struct reb_simulation_integrator_ias15 ri_ias15;		/**< The IAS15 struct */
	struct reb_simulation_integrator_sei ri_sei;		/**< The SEI struct */
	struct reb_simulation_integrator_wh ri_wh;			/**< The WH struct */
	struct reb_simulation_integrator_hybrid ri_hybrid;		/**< The Hybrid struct */

	//////////////////////////////////////////////
	/// Callback functions
	/*
	 * This function allows the user to add additional (non-gravitational) forces.
	 */
	void (*additional_forces) (struct reb_simulation* const r);
	/*
	 * This function allows the user to modify the dditional (non-gravitational) forces.
	 */
	void (*post_timestep_modifications) (struct reb_simulation* const r);
	/**
	 * This function is called at the beginning of the simulation and at the end of
	 * each timestep.
	 */
	void (*heartbeat) (struct reb_simulation* r);
	/**
	 * Return the coefficient of restitution. If NULL, assume a coefficient of 1.
	 */
	double (*coefficient_of_restitution) (const struct reb_simulation* const r, double v); 

	/**
	 * Resolve collision within this function. If NULL, assume hard sphere model.
	 */
	void (*collision_resolve) (struct reb_simulation* const r, struct reb_collision);
};


/**
 * Initializes all REBOUND variables and returns a REBOUND handle.. 
 * This function must be called from problem_init() before any particles are added.
 */
struct reb_simulation* reb_create_simulation();

/**
 * Performon integration step.
 */
void reb_step(struct reb_simulation* const r);

/**
 * Performon an integration. Starting at the current time t and until time tmax.
 * tmax==0 means integrate forever.
 */
int reb_integrate(struct reb_simulation* const r, double tmax);

/*
 * Synchronize particles manually at end of timestep.
 */
void reb_integrator_synchronize(struct reb_simulation* r);

/* 
 * Cleanup all temporarily stored values.
 **/
void reb_integrator_reset(struct reb_simulation* r);

/**
 * Helper function to configure box.
 */
void reb_configure_box(struct reb_simulation* const r, const double boxsize, const int root_nx, const int root_ny, const int root_nz);

/**
 * This function is called once before the integration and then after every timestep.
 * The simulation exits immediately if it returns 1.
 */
int reb_check_exit(struct reb_simulation* const r, const double tmax);

/**
 * Frees up all space.
 */
void reb_free_simulation(struct reb_simulation* const r);

/*
 * Function used to allow binary input.
 */
void reb_reset_temporary_pointers(struct reb_simulation* const r);
void reb_reset_function_pointers(struct reb_simulation* const r);

/** 
 * Adds a particle to the simulation. 
 */
void reb_add(struct reb_simulation* const r, struct reb_particle pt);


/**
 * Remove all particles
 */
void reb_remove_all(struct reb_simulation* const r);

/**
 * Remove particle by position in particles array
 * if keepSorted is set, then particles with indices higher than index
 * are all shifted down one position, ensuring the ordering remains.
 * Returns 1 if particle was successfully removed, 0 if index passed was 
 * out of range.
 */
int reb_remove(struct reb_simulation* const rindex, int id, int keepSorted);

/**
 * Remove particle by id.
 * if keepSorted is set, the particles with indices in the particles array
 * higher than the one with the passed id are all shifted down one position,
 * ensuring the ordering remains. Returns 1 if particle successfully removed,
 * 0 if id was not found in the particles array.
 */
int reb_remove_by_id(struct reb_simulation* const r, int id, int keepSorted);

////////////////////////////////
// Tools (random numbers)

/**
 * Calculates a random variable in a given range.
 * @param min Minimum value.
 * @param max Maximum value.
 */
double reb_random_uniform(double min, double max);

/**
 * Calculates a random variable drawn form a powerlaw distribution.
 * @param min Minimum value.
 * @param max Maximum value.
 * @param slop Slope of powerlaw distribution.
 */
double reb_random_powerlaw(double min, double max, double slope);

/**
 * Calculate a random number with normal distribution.
 * Algorithm by D.E. Knut, 1997, The Art of Computer Programmin, Addison-Wesley. 
 * @param variance Variance of normal distribution.
 * @return Random number with normal distribution (mean 0). 
 */
double reb_random_normal(double variance);

/**
 * Calculates a random variable drawn form a Rayleigh distribution.  Calculated as described on Rayleigh distribution wikipedia page
 * @param sigma Scale parameter.
 */
double reb_random_rayleigh(double sigma);


////////////////////////////////
// Tools (center of mass)


/**
 * Move to center of momentum and center of mass frame.
 */
void reb_move_to_com(struct reb_simulation* const r);

/**
 * Returns the center of mass of particle p1 and p2.
 */
struct reb_particle reb_get_com(struct reb_particle p1, struct reb_particle p2);

////////////////////////////////
// Output functions

/**
 * This function checks if a new output is required at this time.
 * @return The return value is 1 if an output is required and 0 otherwise.
 * @param interval Output interval.
 */
int reb_output_check(struct reb_simulation* r, double interval);

/**
 * Outputs the current number of particles, the time and the time difference since the last output to the screen.
 */
void reb_output_timing(struct reb_simulation* r, const double tmax);

/**
 * Appends an ASCII file with orbital paramters of all particles.
 * @details The orbital parameters are calculated with respect the center of mass.
 * reb_particles are assumed to be sorted from the inside out, the central object having index 0. 
 * @param filename Output filename.
 */
void reb_output_orbits(struct reb_simulation* r, char* filename);


////////////////////////////////
// Tools (setup)

/**
 * This function calculated orbital elements for a given particle. 
 * @param p reb_particle for which the orbit is calculated.
 * @param star Star or central object particle
 * @return Orbital parameters. 
 */
struct reb_orbit reb_tools_p2orbit(double G, struct reb_particle p, struct reb_particle star);

/**
 * Reads a binary file.
 * @param filename Filename to be read.
 */
struct reb_simulation* reb_create_simulation_from_binary(char* filename);


////////////////////////////////
// Tools (misc)
/**
 * Calculate the total energy (potential and kinetic).
 * Might not work for WH.
 * @return Total energy. 
 */
double reb_tools_energy(struct reb_simulation* r);

/* 
 * Init the MEGNO particles
 **/
void reb_tools_megno_init(struct reb_simulation* const r, double delta);

/*
 * Returns the current value of <Y>
 **/
double reb_calculate_megno(struct reb_simulation* r);

/*
 * Returns the largest Lyapunov characteristic number (LCN), or maximal Lyapunov exponent
 **/
double reb_calculate_lyapunov(struct reb_simulation* r);

#endif
