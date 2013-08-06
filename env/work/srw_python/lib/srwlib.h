/************************************************************************//**
 * File: srwlib.h
 * Description: C/C++ API header
 * Project: Synchrotron Radiation Workshop Library (SRWLib)
 * First release: October 2010
 *
 * SRW is Copyright (C) European Synchrotron Radiation Facility, Grenoble, France
 * SRW C/C++ API (SRWLib) is Copyright (C) European XFEL, Hamburg, Germany
 * All Rights Reserved
 *
 * @author O.Chubar, G.Geloni, L.Samoylova
 * @version see srwlUtiVerNo
 ***************************************************************************/

#ifndef __SRWLIB_H
#define __SRWLIB_H

/************************************************************************//**
 * Platform- and compiler-dependent macro definitions
 ***************************************************************************/
#if !(defined(SRWLIB_CLIENT) || defined(SRWLIB_STATIC))
/** For CodeWarrior PowerMac
 */
#if defined __POWERPC__
#if defined SRWLIB_SHARED || defined MATLAB_MEX_FILE
#ifndef EXP
#define EXP __declspec(export)
#endif
#endif
/** For Visual C++
 */
#elif defined __INTEL__ || defined WIN32
#if defined SRWLIB_SHARED || defined MATLAB_MEX_FILE
#ifndef EXP
#define EXP __declspec(dllexport)
#endif
#else
#ifndef EXP
#define EXP __declspec(dllimport)
#endif
#endif
#ifndef CALL
#if defined CALL_MS_STD
#define CALL __stdcall
#else
#define CALL __cdecl
#endif
#endif
/** For HP-UX, GCC
 */
#else
#endif
#endif

#ifndef EXP
#define EXP
#endif

#ifndef CALL
#define CALL
#endif

/***************************************************************************/

#ifdef __cplusplus  
extern "C" {
#endif

/************************************************************************//**
 * Auxiliary SRW C API Structures
 ***************************************************************************/
/**
 * Charged Particle
 */
struct SRWLStructParticle {
	double x, y, z; /* instant coordinates */
	double xp, yp; /* instant transverse velocity components: btx = vx/c, bty = vy/c (angles for relativistic particle) */
	double gamma; /* relative energy */	
	double relE0; /* rest mass (energy) in units of electron rest mass: =1 for electron, =1836.1526988 (=938.272013/0.510998902) for proton */
	int nq; /* charge of the particle related to absolute value of electron charge: =-1 for electron, =1 for positron and for proton */
};
typedef struct SRWLStructParticle SRWLParticle;

/**
 * Particle Beam
 */
struct SRWLStructParticleBeam {
	double Iavg; /* average current [A] */
	double nPart; /* number of electrons (in a bunch) */
	SRWLParticle partStatMom1; /* particle type and 1st order statistical moments */
	double arStatMom2[21]; /* 2nd order statistical moments: ... */
	//double *arStatMom2; 
};
typedef struct SRWLStructParticleBeam SRWLPartBeam;

/**
 * Magnetic Field:
 * Arbitrary 3D ('a' type) 
 * Note: "center point" of the field is not defined in this structure; it should be defined in magn. field container (SRWLStructMagnFieldCont)
 */
struct SRWLStructMagneticField3D {
	double *arBx, *arBy, *arBz; /* pointers to 3D magnetic field component arrays */
	int nx, ny, nz; /* numbers of magnetic field data points in the horizontal, vertical and longitudinal directions */
	double rx, ry, rz; /* ranges of horizontal, vertical and longitudinal coordinates for which the field is defined */
	double *arX, *arY, *arZ; /* optional arrays of transverse and longitudinal coordinates of an irregular 3D mesh (if these pointers are not zero, rx, ry, rz will be ignored; to define a regular (constant-step) 3D mesh, these pointers should be set to zero) */
	int nRep; /* "number of periods", i.e. number of times the field is "repeated" in the longitudinal direction */
	int interp; /* interpolation method to use (e.g. for trajectory calculation): 1- bi-linear (3D), 2- (bi-)quadratic (3D), 3- (bi-)cubic (3D) */
};
typedef struct SRWLStructMagneticField3D SRWLMagFld3D;

/**
 * Magnetic Field:
 * Multipole Magnet ('m' type) 
 * Note: "center point" of the field is not defined in this structure; it should be defined in magn. field container (SRWLStructMagnFieldCont)
 */
struct SRWLStructMagneticFieldMultipole {
	double G; /* field parameter: [T] for dipole, [T/m] for quadrupole (negative means defocusing for x), [T/m^2] for sextupole, [T/m^3] for octupole */
	char m; /* multipole order: 1 for dipole, 2 for quadrupoole, 3 for sextupole, 4 for octupole */
	char n_or_s; /* normal ('n') or skew ('s') */	
	double Leff; /* effective length [m] */
	double Ledge; /* "soft" edge length for field variation from 10% to 90% [m]; G/(1 + ((z-zc)/d)^2)^2 fringe field dependence is assumed */
};
typedef struct SRWLStructMagneticFieldMultipole SRWLMagFldM;

/**
 * Magnetic Field:
 * Solenoid ('s' type) 
 */
struct SRWLStructMagneticFieldSolenoid {
	double B; /* magnetic field [T] enough?  */
	double Leff; /* effective length [m] */
};
typedef struct SRWLStructMagneticFieldSolenoid SRWLMagFldS;

/**
 * Magnetic Field:
 * Undulator Harmonic ('h' type) 
 */
struct SRWLStructMagneticFieldHarmonic {
	int n; /* harmonic number */
	char h_or_v; /* magnetic field plane: horzontal ('h') or vertical ('v') */
	double B; /* magnetic field amplitude [T] */
	double ph; /* phase [rad] */
	int s; /* symmetry vs longitudinal position: 1 - symmetric (B ~ cos(2*Pi*n*z/per + ph)) , -1 - anti-symmetric (B ~ sin(2*Pi*n*z/per + ph)) */
	double a; /* coefficient for transverse depenednce: B*cosh(2*Pi*n*a*y/per)*cos(2*Pi*n*z/per + ph) */
};
typedef struct SRWLStructMagneticFieldHarmonic SRWLMagFldH;

/**
 * Magnetic Field:
 * Undulator ('u' type) 
 */
struct SRWLStructMagneticFieldUndulator {
	SRWLMagFldH *arHarm; /* array of field harmonics */
	int nHarm; /* number of field harmonics */
	double per; /* period length [m] */
	int nPer; /* number of periods */
};
typedef struct SRWLStructMagneticFieldUndulator SRWLMagFldU;

/**
 * Magnetic Field:
 * Container ('c' type)
 */
struct SRWLStructMagneticFieldContainer {
	void **arMagFld; /* array of pointers to magnetic field elements */
	char *arMagFldTypes; /* types of magnetic field elements in arMagFld array */
	double *arXc; /* horizontal center positions of magnetic field elements in arMagFld array */
	double *arYc; /* vertical center positions of magnetic field elements in arMagFld array */
	double *arZc; /* longitudinal center positions of magnetic field elements in arMagFld array */
	int nElem; /* number of magnetic field elements in arMagFld array */
};
typedef struct SRWLStructMagneticFieldContainer SRWLMagFldC;

/**
 * Charged Particle Trajectory
 */
struct SRWLStructParticleTrajectory {
	double *arX, *arXp, *arY, *arYp, *arZ, *arZp; /* arrays of horizontal, vertical and longitudinal positions and relative velocities */
	double *arBx, *arBy, *arBz; /* arrays of horizontal, vertical and longitudinal magnetic field components 'seen' by particle (along trajectory) */
	int np; /* number of trajectory points */
	double ctStart, ctEnd; /* start and end values of independent variable (c*t) for which the trajectory should be (/is) calculated (is constant step enough?) */
	SRWLParticle partInitCond; /* particle type and initial conditions for which the trajectory should be (/is) calculated */
};
typedef struct SRWLStructParticleTrajectory SRWLPrtTrj;

/**
 * Kick Matrix (for fast trajectory calculation)
 */
struct SRWLStructKickMatrix {
	double *arKickMx, *arKickMy; /* horizontal and vertucal kick-matrices tabulated of the same transverse grid (vs x and y) */ 
	char order; /* kick order: 1- first order (in this case kick matrix data is assumed to be in [T*m]), 2- second order (kick matrix data is assumed to be in [T^2*m^2]) */
	int nx, ny, nz; /* nx, ny are numbers of points in kick matrices in horizontal and vertical directions, nz is number of steps in longitudinal direction */
	double rx, ry, rz; /* rx, ry are ranges covered by kick matrices in horizontal and vertical directions, rz is extension in longitudinal direction */
	double x, y, z; /* horizontal, vertical and longitudinal coordinates of center point */
};
typedef struct SRWLStructKickMatrix SRWLKickM;

/**
 * Coherent Gaussian Beam
 */
struct SRWLStructGaussianBeam {
	double x, y, z; /* average coordinates at waist [m] */
	double xp, yp; /* average angles at waist [rad] */
	double avgPhotEn; /* average photon energy [eV] */
	double pulseEn; /* pulse energy [J] */
	double repRate; /* rep. rate [Hz] */
	char polar; /* polarization: 1- lin. hor., 2- lin. vert., 3- lin. 45 deg., 4- lin.135 deg., 5- circ. right, 6- circ. left */
	double sigX, sigY; /* rms beam sizes vs horizontal, vertical position [m] and time [s] at waist (for intensity) */
	double sigT; /* rms pulse duration [s] (for intensity) */
	char mx, my; /* transverse Gauss-Hermite mode orders */
};
typedef struct SRWLStructGaussianBeam SRWLGsnBm;

/**
 * Radiation Mesh (for Electric Field, Stokes params, etc.)
 * TENTATIVE VERSION !
 */
struct SRWLStructRadMesh {
	double eStart, eFin, xStart, xFin, yStart, yFin, zStart; /* initial and final values of photon energy (/time), horizontal, vertical and longitudinal positions */
	long ne, nx, ny; /* numbers of points vs photon energy, horizontal and vertical positions */
};
typedef struct SRWLStructRadMesh SRWLRadMesh;

/**
 * Radiation Wavefront (Electric Field)
 * TENTATIVE VERSION !
 */
struct SRWLStructWaveFront {
	char *arEx, *arEy; /* horizontal and vertical electric field component arrays */
	//double eStart, eFin, xStart, xFin, yStart, yFin, zStart; /* initial and final values of photon energy (/time), horizontal, vertical and longitudinal positions */
	//long ne, nx, ny; /* numbers of points vs photon energy, horizontal and vertical positions */
	SRWLRadMesh mesh;

	double Rx, Ry; /* instant wavefront radii */
	double dRx, dRy; /* error of wavefront radii */
	double xc, yc; /* instant transverse coordinates of wavefront instant "source center" */
	//double xMin, xMax, yMin, yMax; /* exact wavefront boundaries (?) */
	double avgPhotEn; /* averarage photon energy for time-domain simulations */

	char presCA; /* presentation/domain: 0- coordinates, 1- angles */
	char presFT; /* presentation/domain: 0- frequency (photon energy), 1- time */
	char numTypeElFld; /* electric field numerical type: 'f' (float) or 'd' (double) */
	char unitElFld; /* electric field units: 0- arbitrary, 1- sqrt(Phot/s/0.1%bw/mm^2) ? */

	SRWLPartBeam partBeam; /* particle beam source; strictly speaking, it should be just SRWLParticle; however, "multi-electron" information can appear useful for those cases when "multi-electron intensity" can be deduced from the "single-electron" one by convolution */
	double *arElecPropMatr; /* effective 1st order "propagation matrix" for electron beam parameters */
	double *arMomX, *arMomY; /* statistical moments (of Wigner distribution) */
	double *arWfrAuxData; /* misc. auxiliary data about wavefront */
};
typedef struct SRWLStructWaveFront SRWLWfr;

/**
 * Radiation Stokes Parameters
 * TENTATIVE VERSION !
 */
struct SRWLStructStokes {
	char *arS0, *arS1, *arS2, *arS3; /* Stokes component arrays */
	//double eStart, eFin, xStart, xFin, yStart, yFin, zStart; /* initial and final values of photon energy (/time), horizontal, vertical and longitudinal positions */
	//long ne, nx, ny; /* numbers of points vs photon energy, horizontal and vertical positions */
	SRWLStructRadMesh mesh;
	
	double avgPhotEn; /* averarage photon energy for time-domain simulations */

	char presCA; /* presentation/domain: 0- coordinates, 1- angles */
	char presFT; /* presentation/domain: 0- frequency (photon energy), 1- time */
	char numTypeStokes; /* Stokes parameters numerical type: 'f' (float) or 'd' (double) */
	char unitStokes; /* Stokes units: 0- arbitrary, 1- Phot/s/0.1%bw/mm^2 ? */
};
typedef struct SRWLStructStokes SRWLStokes;

/**
 * Optical Element:
 * Drift Space ("drift" type)
 */
struct SRWLStructOpticsDrift {
	double L; /* length [m] */
	char treat; /* switch specifying whether the absolute optical path should be taken into account in radiation phase (=1) or not (=0, default) */
};
typedef struct SRWLStructOpticsDrift SRWLOptD;

/**
 * Optical Element:
 * Aperture/Obstacle ("aperture" or "obstacle" type)
 */
struct SRWLStructOpticsAperture {
	char shape; /* 'r' for rectangular, 'c' for circular */
	char ap_or_ob; /* 'a' for aperture, 'o' for obstacle */
	double Dx, Dy; /* transverse dimensions [m]; in case of circular aperture, only Dx is used for diameter */
	double x, y; /* transverse coordinates of center [m] */
};
typedef struct SRWLStructOpticsAperture SRWLOptA;

/**
 * Optical Element:
 * Thin Lens ("lens" type)
 */
struct SRWLStructOpticsLens {
	double Fx, Fy; /* focal lengths [m] */
	double x, y; /* transverse coordinates of center [m] */
};
typedef struct SRWLStructOpticsLens SRWLOptL;

/**
 * Optical Element:
 * Angle ("angle" type)
 */
struct SRWLStructOpticsAngle {
	double AngX, AngY; /* horizontal and vertical angles [rad] */
};
typedef struct SRWLStructOpticsAngle SRWLOptAng;

/**
 * Optical Element:
 * Shift ("shift" type)
 */
struct SRWLStructOpticsShift {
	double ShiftX, ShiftY; /* horizontal and vertical shifts [m] */
};
typedef struct SRWLStructOpticsShift SRWLOptShift;

/**
 * Optical Element:
 * Zone Plate ("zp" type)
 */
struct SRWLStructOpticsZonePlate {
	int nZones; /* total number of zones */
	double rn; /* auter zone radius [m] */
	double thick; /* thickness [m] */
	double delta1, delta2; /* refractuve index decrements of the "main" and "complementary" materials */
	double atLen1, atLen2; /* attenuation length [m] of the "main" and "complementary" materials */
	double x, y; /* transverse coordinates of center [m] */
};
typedef struct SRWLStructOpticsZonePlate SRWLOptZP;

/**
 * Optical Element:
 * Waveguide (rectangular) ("waveguide" type)
 */
struct SRWLStructOpticsWaveguide {
	double L; /* length [m] */
	double Dx, Dy; /* transverse dimensions [m] */
	double x, y; /* transverse coordinates of center [m] */
};
typedef struct SRWLStructOpticsWaveguide SRWLOptWG;

/**
 * Optical Element:
 * Grating (plane) ("grating" type)
 */
struct SRWLStructOpticsGrating {
	double grDen; /* groove density [lines/mm] */
	char disPl; /* dispersion plane: 'x' ('h') or 'y' ('v') */
	double ang; /* angle between optical axis and grating plane [rad] */
	int m; /* output order to be used */
	double refl; /* average reflectivity (with resp. to intensity) */
};
typedef struct SRWLStructOpticsGrating SRWLOptG;

/**
 * Optical Element:
 * Transmission ("transmission" type)
 */
struct SRWLStructOpticsTransmission {
	double *arTr; /* complex C-aligned data array (of 2*nx*ny length) storing amplitude transmission and optical path difference as function of transverse position */ 
	
	SRWLStructRadMesh mesh; //mesh vs photon energy, horizontal and vertical positions
	
	//int nx, ny; /* numbers of transmission data points in the horizontal and vertical directions */
	//double rx, ry; /* ranges of horizontal and vertical coordinates [m] for which the transmission is defined */
	char extTr; /* 0- transmission outside the grid/mesh is zero; 1- it is same as on boundary */
	double Fx, Fy; /* estimated focal lengths [m] */
	//double x, y; /* transverse coordinates of center [m] */
};
typedef struct SRWLStructOpticsTransmission SRWLOptT;

/**
 * Optical Element:
 * Generic Mirror (common parameters for all mirrors)
 */
struct SRWLStructOpticsMirror {
	double dt, ds; /* mirror dimensions in tangential and sagital directions [m] */
	char apShape; /* shape of aperture in local frame ('r' for rectangular, 'e' for elliptical) */
	double Fx, Fy; /* estimated focal lengths [m] */
	char meth; /* simulation method (1 for "thin" approximation, 2 for "thick" approximation) */
	int npt, nps; /* numbers of transmission data points to represent mirror in tangential direction (used for "thin" approximation) */
	char treatInOut; /* switch specifying how to treat input and output wavefront before and after the main propagation through the optical element:
                0- assume that the input wavefront is defined in the plane before the optical element, and the output wavefront is required in a plane just after the element;
                1- assume that the input wavefront is defined in the plane at the optical element center and the output wavefront is also required at the element center;
                2- assume that the input wavefront is defined in the plane at the optical element center and the output wavefront is also required at the element center; however, before the propagation though the optical element, the wavefront should be propagated through a drift back to a plane just before the optical element, then a special propagator will bring the wavefront to a plane at the optical element exit, and after this the wavefront will be propagated through a drift back to the element center;*/
	double extIn; /* optical element extent on the input side, i.e. distance between the input plane and the optical center (positive, in [m]) to be used at wavefront propagation manipulations; if 0, this extent will be calculated internally from optical element parameters */
	double extOut; /* optical element extent on the output side, i.e. distance between the optical center and the output plane (positive, in [m]) to be used at wavefront propagation manipulations; if 0, this extent will be calculated internally from optical element parameters */

	double *arRefl; /* complex C-aligned data array (of 2*2*nx*ny length) storing compex transmission coefficient as function of component (sigma, pi), photon energy, grazing angle */ 
	int reflNumComp, reflNumPhEn, reflNumAng; /* numbers of reflectivity data points vs component (1 or 2), photon energy, grazing angle */
	double reflPhEnStart, reflPhEnFin; /* initial and final photon energy values for which the reflectivity coefficient is specified */
	double reflAngStart, reflAngFin; /* initial and final grazing angle values for which the reflectivity coefficient is specified */
	char reflPhEnScaleType[4], reflAngScaleType[4]; /* photon energy and angle sampling type (1 for linear, 2 for logarithmic) */

	double nvx, nvy, nvz; /* horizontal, vertical and longitudinal coordinates of central normal vector [m] in the frame of incident beam */
	double tvx, tvy; /* horizontal and vertical coordinates of central tangential vector [m] in the frame of incident beam */
	double x, y; /* transverse coordinates of center [m] */
};
typedef struct SRWLStructOpticsMirror SRWLOptMir;

/**
 * Optical Element:
 * Plane Mirror
 */
struct SRWLStructOpticsMirrorPlane {
	SRWLOptMir baseMir; /* general information about the mirror */
};
typedef struct SRWLStructOpticsMirrorPlane SRWLOptMirPl;

/**
 * Optical Element:
 * Ellipsoidal Mirror
 */
struct SRWLStructOpticsMirrorEllipsoid {
	double p, q; /* distance [m] from first focus ('source') to mirror center, and from center to second focus ('image') */
	double angGraz; /* grazing angle [rad] at mirror center at perfect orientation */
	double radSag; /* sagital radius of curvature [m] at mirror center */
	SRWLOptMir baseMir; /* general information about the mirror */
};
typedef struct SRWLStructOpticsMirrorEllipsoid SRWLOptMirEl;

/**
 * Optical Element:
 * Toroidal Mirror
 */
struct SRWLStructOpticsMirrorToroid {
	double radTan, radSag; /* tangential and sagital radii of the torus [m] */
	SRWLOptMir baseMir; /* general information about the mirror */
};
typedef struct SRWLStructOpticsMirrorToroid SRWLOptMirTor;

/**
 * Optical Element:
 * Container ("container" type)
 */
struct SRWLStructOpticsContainer {
	void **arOpt; /* array of pointers to optical elements */
	char **arOptTypes; /* array of types of optical elements (C strings) in arOpt array */
	int nElem; /* number of magnetic field elements in arMagFld array */
	double **arProp; /* array of arrays of propagation parameters to be used for individual optical elements */
	int nProp; /* number of propagation instructions (length of arProp array); 
			      can be nProp <= (nElem + 1); if nProp == (nElem + 1), last resizing is applied after the propagation */
};
typedef struct SRWLStructOpticsContainer SRWLOptC;

/************************************************************************//**
 * Main SRW C API
 ***************************************************************************/
/** 
 * Sets pointer to external function which modifies size (amount of data) of a wavefront.  
 * @param [in] pExtFunc pointer to the external function
 * @see ... 
 */
EXP void CALL srwlUtiSetWfrModifFunc(int (*pExtFunc)(int action, SRWLWfr* pWfrIn, char pol));

/** 
 * Sets pointer to external function to show progress of computation process
 * @param [in] pExtFunc pointer to the external function
 * @see ... 
 */
EXP void CALL srwlUtiSetProgrIndFunc(int (*pExtFunc)(double curVal));

/** 
 * Specifies current SRW version number.  
 * @param [out] verNoStr string specifying current version number of SRW (/API)
 * @param [in] code specifies which code version to return: =1 for SRW code, =2 for SRW C API (SRWLIB)
 * @return	integer error (>0) or warnig (<0) code
 * @see ...
 */
EXP int CALL srwlUtiVerNo(char* verNoStr, int code);

/** 
 * Provides text of error or warning based on its number.  
 * @param [out] t output text of error or warning
 * @param [in] erNo integer number of error (if >0) or warning (if <0)
 * @return	integer error (>0) or warnig (<0) code
 * @see ...
 */
EXP int CALL srwlUtiGetErrText(char* t, int erNo);

/** 
 * Calculates (tabulates) 3D magnetic field created by multiple elements
 * @param [in, out] pDispMagFld pointer to resulting magnetic field container with one element - 3D magnetic field structure to keep tabulated field data (all arrays should be allocated in a calling function/application)
 * @param [in] pMagFld pointer to input magnetic field (container) structure
 * @return	integer error (>0) or warnig (<0) code
 * @see ...
 */
EXP int CALL srwlCalcMagFld(SRWLMagFldC* pDispMagFld, SRWLMagFldC* pMagFld);

/** 
 * Calculates charged particle trajectory in external 3D magnetic field (in Cartesian laboratory frame) 
 * @param [in, out] pTrj pointer to resulting trajectory structure (all data arrays should be allocated in a calling function/application); the initial conditions and particle type must be specified in pTrj->partInitCond; the initial conditions are assumed to be defined for ct = 0, however the trajectory will be calculated for the mesh defined by pTrj->np, pTrj->ctStart, pTrj->ctEnd
 * @param [in] pMagFld pointer to input magnetic field (container) structure
 * @param [in] precPar (optional) method ID and precision parameters; 
 *             if(precPar == 0) default 4th-order Runge-Kutta method is used; otherwise:
 *             precPar[0] is number of precision parameters that will follow
 *             [1] method number: 1- 4th-order Runge-Kutta; 2- 5th-order Runge-Kutta;
 *             [2] interpolation method to use for tabulated magnetic field: 1- simple bi-linear (3D); 2- bi-quadratic (3D); 3- bi-cubic (3D); 4- 1D cubic spline + 2D bi-cubic
 *             [3],[4],[5],[6],[7]: absolute precision values for X[m],X'[rad],Y[m],Y'[rad],Z[m] (yet to be tested!!) - to be taken into account only for R-K fifth order or higher
 *             [8]: rel. tolerance for R-K fifth order or higher (default = 1) 
 *             [9]: max. number of auto-steps for R-K fifth order or higher (default = 5000)
 * @return	integer error (>0) or warnig (<0) code
 * @see ...
 */
EXP int CALL srwlCalcPartTraj(SRWLPrtTrj* pTrj, SRWLMagFldC* pMagFld, double* precPar =0);

/** 
 * Calculates charged particle trajectory from an array of kick matrices
 * @param [in, out] pTrj pointer to resulting trajectory structure (all data arrays should be allocated in a calling function/application); the initial conditions and particle type must be specified in pTrj->partInitCond; the initial conditions are assumed to be defined for ct = 0, however the trajectory will be calculated for the mesh defined by pTrj->np, pTrj->ctStart, pTrj->ctEnd
 * @param [in] arKickM array of kick matrix structures
 * @param [in] nKickM number of kick matrices in the array
 * @param [in] precPar (optional) method ID and precision parameters; 
 *             precPar[0] switch specifying whether the new trajectory data should be added to pre-existing data in pTrj (precPar[0]=1, default) or it should override any pre-existing trajectory data in pTrj (precPar[0]=0)
 * @return	integer error (>0) or warnig (<0) code
 * @see ...
 */
EXP int CALL srwlCalcPartTrajFromKickMatr(SRWLPrtTrj* pTrj, SRWLKickM* arKickM, int nKickM, double* precPar =0);

/** 
 * Calculates Electric Field (Wavefront) of Synchrotron Radiation by a relativistic charged particle traveling in external 3D magnetic field
 * @param [in, out] pWfr pointer to resulting Wavefront structure; all data arrays should be allocated in a calling function/application; the mesh, presentation, etc., should be specified in this structure at input
 * @param [in] pTrj pointer to pre-calculated particle trajectory structure; the initial conditions and particle type must be specified in pTrj->partInitCond; if the trajectory data arrays (pTrj->arX, pTrj->arXp, pTrj->arY, pTrj->arYp) are defined, the SR will be calculated from these data; if these arrays are not supplied (pointers are zero), the function will attempt to calculate the SR from the magnetic field data (pMagFld) which has to be supplied
 * @param [in] pMagFld optional pointer to input magnetic field (container) structure; to be taken into account only if particle trajectroy arrays (pTrj->arX, pTrj->arXp, pTrj->arY, pTrj->arYp) are not supplied
 * @param [in] precPar precision parameters: 
 *	   precPar[0]: method ID (0- "manual", 1- "auto-undulator", 2- "auto-wiggler")
 *			  [1]: step size or relative precision
 *			  [2]: longitudinal position to start integration
 *			  [3]: longitudinal position to finish integration
 *			  [4]: number of points to use for trajectory calculation 
 *			  [5]: calculate terminating terms or not: 0- don't calculate two terms, 1- do calculate two terms, 2- calculate only upstream term, 3- calculate only downstream term 
 *			  [6]: sampling factor (for propagation, effective if > 0)
 *			  [7]: ... 
 * @param [in] nPrecPar number of precision parameters 
 * @return	integer error (>0) or warnig (<0) code
 * @see ...
 */
EXP int CALL srwlCalcElecFieldSR(SRWLWfr* pWfr, SRWLPrtTrj* pTrj, SRWLMagFldC* pMagFld, double* precPar =0, int nPrecPar =0);

/** 
 * Calculates Electric Field (Wavefront) of a coherent Gaussian Beam
 * @param [in, out] pWfr pointer to resulting Wavefront structure; all data arrays should be allocated in a calling function/application; the mesh, presentation, etc., should be specified in this structure at input
 * @param [in] pGsnBm pointer to a Gaussian beam parameters structure
 * @param [in] precPar precision parameters: [0]- sampling factor (for propagation, effective if > 0)
 * @return	integer error (>0) or warnig (<0) code
 * @see ...
 */
EXP int CALL srwlCalcElecFieldGaussian(SRWLWfr* pWfr, SRWLGsnBm* pGsnBm, double* precPar =0);

/** 
 * Calculates Spectral Flux (Stokes components) of Synchrotron Radiation by a relativistic finite-emittance electron beam traveling in periodic magnetic field of an Undulator
 * @param [in, out] pStokes pointer to the resulting Stokes structure; all data arrays should be allocated in a calling function/application; the mesh, presentation, etc., should be specified in this structure at input
 * @param [in] pElBeam  pointer to input electron beam structure
 * @param [in] pUnd  pointer to input undulator (periodic magnetic field) structure
 * @param [in] precPar precision parameters: 
 *             precPar[0]: initial harmonic
 *             [1]: final harmonic
 *             [2]: longitudinal integration precision parameter
 *             [3]: azimuthal integration precision parameter
 *             [4]: calculate flux (precPar[4] == 1) or intensity (precPar[4] == 2)
 * @return	integer error (>0) or warnig (<0) code
 * @see ...
 */
EXP int CALL srwlCalcStokesUR(SRWLStokes* pStokes, SRWLPartBeam* pElBeam, SRWLMagFldU* pUnd, double* precPar =0);

/** 
 * Calculates Power Density distribution of Synchrotron Radiation by a relativistic finite-emittance electron beam traveling in arbitrary magnetic field
 * @param [in, out] pStokes pointer to resulting Stokes structure (pStokes->arS0); all data arrays should be allocated in a calling function/application; the mesh, presentation, etc., should be specified in this structure at input
 * @param [in] pElBeam pointer to input electron beam structure 
 * @param [in] pTrj pointer to input trajectory structure (can be == 0; in such case, the power density is calculated based on pElBeam and pMagFld)
 * @param [in] pMagFld pointer to input magnetic field container structure (can be == 0; in such case, power density is calculated from pTrj (if pTrj != 0) and pElBeam (if pElBeam != 0))
 * @param [in] precPar precision parameters: 
 *             arPrecP[0]: precision factor (1 default, >1 more precision)
 *             [1]: power density computation method (1- "near field" (default), 2- "far field")
 *             [2]: initial longitudinal position (effective if < arPrecP[3])
 *             [3]: final longitudinal position (effective if > arPrecP[2])
 *			   [4]: number of points to use for trajectory calculation 
 * @return	integer error (>0) or warnig (<0) code
 * @see ...
 */
EXP int CALL srwlCalcPowDenSR(SRWLStokes* pStokes, SRWLPartBeam* pElBeam, SRWLPrtTrj* pTrj, SRWLMagFldC* pMagFld, double* precPar =0);

/** 
 * Calculates/extracts Intensity and/or other characteristics from pre-calculated Electric Field
 * @param [out] pInt pointer to resulting Intensity (array)
 * @param [in] pWfr pointer to pre-calculated Wavefront structure
 * @param [in] pol polarization component to extract: 
 *             0- Linear Horizontal; 
 *             1- Linear Vertical; 
 *             2- Linear 45 degrees; 
 *             3- Linear 135 degrees; 
 *             4- Circular Right; 
 *             5- Circular Left; 
 *             6- Total
 * @param [in] intType "type" of a characteristic to be extracted: 
 *             0- Single-Electron Intensity; 
 *             1- Multi-Electron Intensity; 
 *             2- Single-Electron Flux; 
 *             3- Multi-Electron Flux; 
 *             4- Single-Electron Radiation Phase; 
 *             5- Re(E): Real part of Single-Electron Electric Field;
 *             6- Im(E): Imaginary part of Single-Electron Electric Field
 * @param [in] depType type of dependence to extract: 
 *             0- vs e (photon energy or time); 
 *             1- vs x (horizontal position or angle); 
 *             2- vs y (vertical position or angle); 
 *             3- vs x&y; (horizontal and vertical positions or angles);
 *             4- vs e&x; 
 *             5- e&y; 
 *             6- vs e&x&y 
 * @param [in] e photon energy (to keep fixed)
 * @param [in] x horizontal position (to keep fixed)
 * @param [in] y vertical position (to keep fixed)
 * @return	integer error (>0) or warnig (<0) code
 * @see ...
 */
EXP int CALL srwlCalcIntFromElecField(char* pInt, SRWLWfr* pWfr, char pol, char intType, char depType, double e, double x, double y);

/** 
 * "Resizes" Electric Field Wavefront vs transverse positions / angles or photon energy / time
 * @param [in, out] pWfr pointer to pre-calculated Wavefront structure
 * @param [in] type character specifying whether the resizing should be done vs coordinates/angles ('c') or vs photon energy/time ('f') 
 * @param [in] par array of parameters: 
 *             [0]- method (0- regular method, without FFT, 1- "special" method involving FFT),
 *             [1]- range resizing factor for horizontal poosition / angle (if type == 'c') or for photon energy / time (if type == 'f')
 *             [2]- resolution resizing factor for horizontal poosition / angle (if type == 'c') or for photon energy / time (if type == 'f')
 *             [3]- range resizing factor for vertical poosition / angle (effective only if type == 'c')
 *             [4]- resolution resizing factor for vertical poosition / angle (effective only if type == 'c')
 * @return	integer error (>0) or warnig (<0) code
 * @see ...
 */
EXP int CALL srwlResizeElecField(SRWLWfr* pWfr, char type, double* par);

/** 
 * Changes Representation of Electric Field: coordinates<->angles, frequency<->time
 * @param [in, out] pWfr pointer to pre-calculated Wavefront structure
 * @param [in] repr character specifying desired representation ('c' for coordinate, 'a' for angle, 'f' for frequency, 't' for time)
 * @return	integer error (>0) or warnig (<0) code
 * @see ...
 */
EXP int CALL srwlSetRepresElecField(SRWLWfr* pWfr, char repr);

/** 
 * "Propagates" Electric Field Wavefront through Optical Elements and free spaces
 * @param [in, out] pWfr pointer to pre-calculated Wavefront structure
 * @param [in] pOpt pointer to container of optical elements the propagation should be done through
 * @return	integer error (>0) or warnig (<0) code
 * @see ...
 */
EXP int CALL srwlPropagElecField(SRWLWfr* pWfr, SRWLOptC* pOpt);

/** TEST
 * "Propagates" multple Electric Field Wavefronts from different electrons through Optical Elements and free spaces
 * @param [in, out] pWfr0 pointer to pre-calculated Wavefront structure from an average electron
 * @param [in] pOpt pointer to container of optical elements the propagation should be done through
 * @param [in] precPar precision parameters: 
 *             precPar[0]: number of "macro-electrons" / coherent wavefronts
 *             [1]: how many "macro-electrons" / coherent wavefronts to propagate before calling (*pExtFunc)(int action, SRWLStokes* pStokesIn) for instant visualization
 *             [2]: parallel interface to use (0- none, 1- mpi)
 * @param [in] pExtFunc pointer to external function which modifies or "visualizes" instant state of SRWLStokes* pStokes
 * @return	integer error (>0) or warnig (<0) code
 * @see ...
 */
EXP int CALL srwlPropagRadMultiE(SRWLStokes* pStokes, SRWLWfr* pWfr0, SRWLOptC* pOpt, double* precPar, int (*pExtFunc)(int action, SRWLStokes* pStokesInst));

/***************************************************************************/

#ifdef __cplusplus  
}
#endif
#endif