/*	
 * 	Licensed under the GNU Public License version 3.
 *	Read attached LICENSE
 * 	Copyright (C) 2012 - Fredrik Haikarainen
 * 	Fredrik.haikarainen@gmail.com
 */

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>

typedef enum LBOOL{
	TRUE = 0,
	FALSE = 1
} LBOOL;

typedef enum LOPTYPE{
	SET = 0,
	ADD = 1,
	SUB = 2
} LOPTYPE;

/* Flags */
LBOOL q; /* Quiet, supresses output */
LBOOL c; /* print unprecise current value in percentage */
LBOOL m; /* read max value*/
LBOOL p; /* precision read current value */
int wbright;
LOPTYPE ot;

LBOOL readint(const char* f, unsigned int *i){
	FILE* ff = fopen(f, "r");
	if(ff){
		fscanf(ff, "%u", i);
		fclose(ff);
		return TRUE;
	}else{
		return FALSE;
	}
}

LBOOL readchar(const char* f, char *c){
	
	FILE* ff = fopen(f, "r");

	/*c = malloc(sz+1);*/
	if(ff){
		unsigned int sz;
		fseek(ff, 0L, SEEK_END);
		sz = ftell(ff);
		fseek(ff, 0L, SEEK_SET);
		fgets(c,sz,ff);
		fclose(ff);
		return TRUE;
	}else{
		return FALSE;
	}
}

LBOOL writeint(const char* f, unsigned int i){
	FILE* ff = fopen(f, "w");
	if(ff){
		fprintf(ff, "%u", i);
		fclose(ff);
		return TRUE;
	}else{
		return FALSE;
	}
}


LBOOL is_dir(const char* d){
	DIR* dd = opendir(d);
	if(dd){
		closedir(dd);
		return TRUE;
	}else{
		return FALSE;
	}
}

LBOOL is_writable(const char* f){
	FILE* dd = fopen(f, "w");
	if(dd){
		fclose(dd);
		return TRUE;
	}else{
		return FALSE;
	}	
}

typedef struct {
	char		name[256];
	unsigned int	current_brightness;
	unsigned int	max_brightness;
	char		c_path[256]; /* Controller-path */
	char		cb_path[256]; /* Current brightness-path */
	char		mb_path[256]; /* Max brightness-path */
	char		b_path[256]; /* Brightness-path */
	enum LBOOL	is_ok;
	unsigned int	guide_id;
} controller;

typedef struct {
	controller	controllers[256];
	unsigned int	num_controllers;
} fetch_result;


fetch_result fetch_controllers(const char* ctrldir){
	fetch_result returner;
	struct dirent* ep;

	DIR* lcdir = opendir(ctrldir);
	
	returner.num_controllers = 0;

	if(!lcdir){
		if(q == FALSE)
			printf("Error: Could not open directory '%s'!\n", ctrldir);
	}else{
		while( ( ep = readdir(lcdir) ) ){
			char* currctrldir = "";
			char* currctrldir_curr = "";
			char* currctrldir_max = "";
			char* currctrldir_f = "";
			if( ep->d_name[0] != '.'){
				strncpy(returner.controllers[returner.num_controllers].name, ep->d_name, sizeof(returner.controllers[returner.num_controllers].name));
				
				/* Set some default values, in case something fails we dont just get null */
				returner.controllers[returner.num_controllers].current_brightness = 0;
				returner.controllers[returner.num_controllers].max_brightness = 0;
				returner.controllers[returner.num_controllers].is_ok = TRUE;
				/*
				returner.controllers[returner.num_controllers].c_path = NULL;
				returner.controllers[returner.num_controllers].cb_path = NULL;
				returner.controllers[returner.num_controllers].b_path = NULL;
				returner.controllers[returner.num_controllers].mb_path = NULL;
				*/
				/* Get path to the current controller dir */
				asprintf(&currctrldir, "%s/%s", ctrldir, ep->d_name);
				
				strncpy(returner.controllers[returner.num_controllers].c_path, currctrldir, sizeof(returner.controllers[returner.num_controllers].c_path));
				
				if(is_dir(currctrldir) == FALSE){
					if(q == FALSE)
						printf("Warning: '%s' is not a directory, check your system.\n", currctrldir);
					returner.controllers[returner.num_controllers].is_ok = FALSE;
				}
				
				/* Get path to current actual_brightness-file */
				asprintf(&currctrldir_curr, "%s/%s", currctrldir, "actual_brightness");
				strncpy(returner.controllers[returner.num_controllers].cb_path, currctrldir_curr, sizeof(returner.controllers[returner.num_controllers].cb_path));
				if( readint(currctrldir_curr, &returner.controllers[returner.num_controllers].current_brightness) == FALSE ){
					if(q == FALSE)
						printf("Warning: Can't read actual_brightness-file of '%s'. Will ignore this controller.\n", ep->d_name);
					returner.controllers[returner.num_controllers].is_ok = FALSE;
				}				
				
				/* Get path to current max_brightness-file*/
				asprintf(&currctrldir_max, "%s/%s", currctrldir, "max_brightness");
				strncpy(returner.controllers[returner.num_controllers].mb_path, currctrldir_max, sizeof(returner.controllers[returner.num_controllers].mb_path));
				
				if( readint(currctrldir_max, &returner.controllers[returner.num_controllers].max_brightness) == FALSE ){
					if(q == FALSE)
						printf("Warning: Can't read max_brightness-file of '%s'. Will ignore this controller.\n", ep->d_name);
					returner.controllers[returner.num_controllers].is_ok = FALSE;
				}
				
				/* Get path to current brightness-file */
				asprintf(&currctrldir_f, "%s/%s", currctrldir, "brightness");
				strncpy(returner.controllers[returner.num_controllers].b_path, currctrldir_f, sizeof(returner.controllers[returner.num_controllers].b_path));
				if( is_writable(currctrldir_f) == FALSE){
					if(q == FALSE)
						printf("Warning: Controllerfile of '%s' is not writable. Will ignore this controller.\n", ep->d_name);
					returner.controllers[returner.num_controllers].is_ok = FALSE;
				}
				
				
				returner.num_controllers++;
			}
		}
		closedir(lcdir);	
	}
	return returner;
}

controller* get_best_controller(fetch_result* res){
	unsigned int it;
	unsigned int cmax;
	LBOOL foundokctrl;
	controller* returner;
	
	
	it = 0;
	cmax = 0;
	foundokctrl = FALSE;
	
	returner = NULL;
	
	while(it < res->num_controllers){
		if(res->controllers[it].is_ok == TRUE){
			if(foundokctrl != TRUE){
				foundokctrl = TRUE;
				returner = &res->controllers[it];
			}
			if(res->controllers[it].max_brightness > cmax){
				cmax = res->controllers[it].max_brightness;
				returner = &res->controllers[it];
			}
		}
		it++;
	}
	
	return returner;
}

controller* get_controller_by_name(fetch_result* res, const char* name){
	unsigned int it;
	controller* returner;
	
	it = 0;
	returner = NULL;
	
	while(it < res->num_controllers){
		if(strcmp(res->controllers[it].name, name) == 0){
			returner = &res->controllers[it];
		}
		it++;
	}
	
	return returner;
		
}

void usage(){
	printf("Usage: light [-qcaspmfh] [--options] <value>\n\n\tFlags:\n\t-q:\t Run quiet, supresses output.\n\t-c:\t Prints the current brightness in percent and exits.(Not precise)\n\t-p:\t Prints the current brightness directly from controller and exits. (Precise)\n\t-m:\t Prints the max brightness directly from controller and exits. \n\t-a:\t Add the value instead of setting it.\n\t-s:\t Subtract the value instead of setting it.\n\t-h:\t Shows this help and exits.\n");
	printf("\n\tOptions:\n\t--help:\t Shows this help and exits.\n\n\t<value>\t Brightness wanted in percent.\n\n");
}

int main(int argc, char **argv) {
	unsigned int argsit;
	
	fetch_result	res;
	controller*	best_ctrl;
	unsigned int	citr;
	uid_t		uid;
	unsigned int	minlight;
	LBOOL		given;
	unsigned int	real_wbright;
	unsigned int	minlight_nonp;
	unsigned int	curr_bright;
	unsigned int	curr_brightp;
	LBOOL		useforce;
	char*		useforce_name;
	
	/* Get UID */
	uid = getuid();
	
	useforce = FALSE;
	useforce_name = malloc(256*sizeof(char));
	
	/* Parse arguments */
	q=FALSE;
	c=FALSE;
	m=FALSE;
	p=FALSE;
	ot=SET;
	wbright=0;
	argsit = 1;
	given=FALSE;
	while(argsit < argc){
		char* carg = argv[argsit];
		if(carg[0] == '-'){
			LBOOL argdone;
			argdone = FALSE;
			if(strlen(carg) > 2){
				if(carg[1] == '-'){
					int longargs = strlen(carg) -2;
					char *longarg = (char*) malloc(longargs);
					strncpy(longarg, carg+2, longargs);
					
					if(strcmp(longarg, "help") == 0){
						usage();
						return 0;
					}else{
						printf("Unknown option: \"%s\".\n", longarg);
						return 0;
					}
					argdone = TRUE;
				}
			}
			
			if(argdone == FALSE){
				unsigned int cargit = 1;
				while(cargit < strlen(carg)){
					switch(carg[cargit]){
						case 'q':
							q = TRUE;
						break;
						case 'a':
							ot = ADD;
						break;
						case 's':
							ot = SUB;
						break;
						case 'c':
							c = TRUE;
						break;
						case 'p':
							p = TRUE;
						break;
						case 'm':
							m = TRUE;
						break;
						case 'h':
							usage();
							return 0;
						break;
						default:
							printf("Unknown flag: %c\n", carg[cargit]);
							return 1;
						break;
					}
					cargit++;
				}
			}
		}else{
			wbright = atoi(carg);
			given=TRUE;
		}
		
		argsit++;
	}
	
	
	if(c == TRUE || m == TRUE || p == TRUE){
		q = TRUE;
	}
	
	if(given == FALSE && c == FALSE && m == FALSE && p == FALSE){
		usage();
		return 0;
	}
	if(q == FALSE)
		printf("Light 0.7 - Fredrik Haikarainen\n");
	
	/* Get and check minlight */
	
	if(readint("/etc/light/minlight", &minlight) == FALSE){
		minlight = 5;
		if(q == FALSE)
			printf("Warning: Couldn't read /etc/light/minlight, using 5 as default.\n");
	}
	
	/* Fetch controllers */
	if(q == FALSE)
		printf("Fetching controllers..\n");
	
	res = fetch_controllers("/sys/class/backlight");
	citr = 0;
	while(citr < res.num_controllers){
		controller* currc = &res.controllers[citr];
		if(currc->is_ok == TRUE){
			if(q == FALSE)
				printf("\tFound '%s' (%u/%u)\n", currc->name, currc->current_brightness, currc->max_brightness);
		}else{
			if(q == FALSE)
				printf("\tFound '%s', but ignoring\n", currc->name);
		}
		citr++;
	}

	if(q == FALSE)
		printf("\n");
	/* Read override-file if exists*/
	if(readchar("/etc/light/override",useforce_name) == TRUE){
		printf("Overriding controller '%s' !\n", useforce_name);
		useforce=TRUE;
	}
	if(useforce == TRUE){
		best_ctrl = get_controller_by_name(&res, useforce_name);
		if(best_ctrl == NULL){
			if(q == FALSE)
				printf("Can't override, no such controller. Check/remove your override-file!\n");
			return 1;	
		}
	}else{
		/* Get the best controller */
		best_ctrl = get_best_controller(&res);
		
		if(best_ctrl == NULL){
			if(uid == 0){
				if(q == FALSE)
					printf("No okay controller found, even though you are root! Check your system.\n");
			}else{
				if(q == FALSE)
					printf("No okay controller found, check your permissions or try to run as root.\n");
			}
			return 1;
		}
	}
	
	if(p == TRUE){
		printf("%u\n", best_ctrl->current_brightness);
		return 0;
	}
	
	if(m == TRUE){
		printf("%u\n", best_ctrl->max_brightness);
		return 0;
	}
	
	if(q == FALSE)
		printf("Using controller '%s' ..\n", best_ctrl->name);
	
	if(wbright < 0){ wbright = 0;}
	if(wbright > 100){wbright=100;}
	
	curr_bright = best_ctrl->current_brightness;
	curr_brightp = (float)((float)curr_bright / (float)best_ctrl->max_brightness) * 100;
	
	
	if(c == TRUE){
		printf("%u\n", curr_brightp);
		return 0;
	}
	
	minlight_nonp = best_ctrl->max_brightness * ( (float)minlight / 100) + 1;
	
	switch(ot){
		case SET:
			real_wbright = best_ctrl->max_brightness * ( (float)wbright / 100 );
		break;
		case ADD:
			real_wbright = ( best_ctrl->max_brightness * ( (float)( curr_brightp + wbright +1) / 100 ));
		break;
		case SUB:
			if(curr_brightp <= wbright){
				real_wbright = minlight_nonp;
			}else{
				real_wbright = ( best_ctrl->max_brightness * ( (float)( curr_brightp - wbright + 1) / 100 ));
			}
		break;
		default:
		break;
	}
	


	/* FIXME<- SHOULD BE FIXED NOW, LETS STAY HERE ANYWAY JUST IN CASE 
		Line below makes sure the value never wraps around and gets higher. Puts a (high) limit on max brightness.
		Not sure if safe for portabilities sake.
	 
	if(real_wbright > ((UINT_MAX/2) - best_ctrl->max_brightness)){ real_wbright = minlight_nonp; } 
	*/
	
	
	if(real_wbright > best_ctrl->max_brightness){real_wbright = best_ctrl->max_brightness;}
	if(real_wbright < minlight_nonp){real_wbright = minlight_nonp;}
	
	if(q == FALSE)
		printf("Writing %u to file '%s'..\n", real_wbright, best_ctrl->b_path);
	
	if(writeint(best_ctrl->b_path, real_wbright) == FALSE){
		if(q == FALSE){
			printf("Error: Could not write to file %s, check your permissions!\n", best_ctrl->b_path);
		}
		return 1;
	}
	
	return 0;
}
