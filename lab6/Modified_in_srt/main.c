//
//  main.c
//  srt
//
//  Created by vector on 11/2/10.
//  Copyright (c) 2010 Brian F. Allen.

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "raymath.h"
#include "shaders.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <pthread.h>

static scene_t scene;
static Vec3 camera_pos;
static Vec3 camera_dir;
static double camera_fov;
static Vec3 bg_color;
static double pixel_dx;
static double pixel_dy;
static double subsample_dx;
static double subsample_dy;
static float * color_str;
static int thread_num;

static double dirs[6][3] =
{ {1,0,0}, {-1,0,0}, {0,1,0}, {0,-1,0}, {0,0,1}, {0,0,-1} };
static const int opposites[] = { 1, 0, 3, 2, 5, 4 };


static void
add_sphereflake( scene_t* scene, int sphere_id, int parent_id, int dir,
		 double ratio, int recursion_level )
{
    sphere_t* parent = &scene->spheres[parent_id];
    sphere_t* child = &scene->spheres[sphere_id];

    /* start at parents origin */
    mul( child->org, dirs[dir], (1.+ratio)*parent->rad );
    add( child->org, child->org, parent->org );
    child->rad = parent->rad * ratio;
    copy( child->color, parent->color );
    child->shader = parent->shader;
    scene->sphere_count++;
}

static int
recursive_add_sphereflake( scene_t* scene, int parent_id, int parent_dir,
			   int sphere_id, int dir,
			   int recursion_level, int recursion_limit )
{
    const double ratio = 0.35;

    add_sphereflake( scene, sphere_id, parent_id, dir, ratio, recursion_level );
    if( recursion_level > recursion_limit )
    {
        return sphere_id + 1;
    }

    /* six children, one at each cardinal point */
    parent_id = sphere_id;
    sphere_id = sphere_id + 1;
    for( int child_dir=0; child_dir<6; ++child_dir )
    {
        /* skip making spheres inside parent */
        if( parent_dir == opposites[child_dir] ) continue;
        sphere_id = recursive_add_sphereflake( scene, parent_id, parent_dir,
                                               sphere_id, child_dir,
                                               recursion_level + 1,
                                               recursion_limit );
    }
    return sphere_id;
}

static scene_t
create_sphereflake_scene( int recursion_limit )
{
    scene_t scene;
    Vec3 color;
    sphere_t* sphere;

    init_scene( &scene );

    // Pantone UC Gold 122
    add_light( &scene, 2, 5, 0, 0.996, 0.733, 0.212 );

    // Pantone UCLA Blue (50,132,191)
    add_light( &scene, -5, 3, -5, 0.196, 0.517, 0.749 );

    int max_sphere_count = 2 + powl( 6, recursion_limit + 2 );
    scene.spheres = realloc( scene.spheres,
                             max_sphere_count*sizeof( sphere_t ) );
    if( !scene.spheres )
    {
        fprintf( stderr, "Failed to get memory for sphereflake.  aborting.\n" );
        exit( -1 );
    }

//    sphere = &(scene.spheres[0]);
//    set( sphere->org, -0.5, -1.0, 0 );
//    sphere->rad = 0.75;
//    set( color, 0.85, 0.25, 0.25 );
//    copy( sphere->color, color );
//    sphere->shader = mirror_shader;


    /* center sphere is special, child inherent shader and color */
    sphere = &(scene.spheres[0]);
    scene.sphere_count++;
    set( sphere->org, 0, -1, 0 );
    sphere->rad = 0.75;
    set( color, 0.75, 0.75, 0.75 );
    copy( sphere->color, color );
    sphere->shader = mirror_shader;
    recursive_add_sphereflake( &scene,
                               0, /* parent is the first sphere */
                               -1, /* -1 means no dir, make all children */
                               1, /* next free sphere index */
                               2, /* starting dir */
                               0, /* starting recursion level */
                               recursion_limit );

    return scene;
}

static void
free_scene( scene_t* arg )
{
    free( arg->lights );
    arg->light_count = 0;
    free( arg->spheres );
    arg->sphere_count = 0;
}

/******
 * Constants that have a large effect on performance */

/* how many levels to generate spheres */
enum { sphereflake_recursion = 3 };

/* output image size */
enum { height = 131 };
enum { width = 131 };

/* antialiasing samples, more is higher quality, 0 for no AA */
enum { halfSamples = 4 };
/******/

/* color depth to output for ppm */
enum { max_color = 255 };

/* z value for ray */
enum { z = 1 };


void * parallel_render (void * arg){
	//starting point
	int sub_start = *((int *)arg);
	/* for every pixel */
    for( int px= sub_start; px<width; px+=thread_num )
    {
        const double x = pixel_dx * ((double)( px-(width/2) ));
        for( int py=0; py<height; ++py )
        {
            const double y = pixel_dy * ((double)( py-(height/2) ));
            Vec3 pixel_color;
            set( pixel_color, 0, 0, 0 );

            for( int xs=-halfSamples; xs<=halfSamples; ++xs )
            {
                for( int ys=-halfSamples; ys<=halfSamples; ++ys )
                {
                    double subx = x + ((double)xs)*subsample_dx;
                    double suby = y + ((double)ys)*subsample_dy;

                    /* construct the ray coming out of the camera, through
                     * the screen at (subx,suby)
                     */
                    ray_t pixel_ray;
                    copy( pixel_ray.org, camera_pos );
                    Vec3 pixel_target;
                    set( pixel_target, subx, suby, z );
                    sub( pixel_ray.dir, pixel_target, camera_pos );
                    norm( pixel_ray.dir, pixel_ray.dir );

                    Vec3 sample_color;
                    copy( sample_color, bg_color );
                    /* trace the ray from the camera that
                     * passes through this pixel */
                    trace( &scene, sample_color, &pixel_ray, 0 );
                    /* sum color for subpixel AA */
                    add( pixel_color, pixel_color, sample_color );
                }
            }

            /* at this point, have accumulated (2*halfSamples)^2 samples,
             * so need to average out the final pixel color
             */
            if( halfSamples )
            {
                mul( pixel_color, pixel_color,
                     (1.0/( 4.0 * halfSamples * halfSamples ) ) );
            }

            /* done, final floating point color values are in pixel_color */
            float scaled_color[3];
            scaled_color[0] = gamma( pixel_color[0] ) * max_color;
            scaled_color[1] = gamma( pixel_color[1] ) * max_color;
            scaled_color[2] = gamma( pixel_color[2] ) * max_color;

            /* enforce caps, replace with real gamma */
            for( int i=0; i<3; i++)
                scaled_color[i] = max( min(scaled_color[i], 255), 0);

            /* write this pixel out to disk. ppm is forgiving about whitespace,
             * but has a maximum of 70 chars/line, so use one line per pixel
             */
			 //copy to color_str
            int where = px*height*3 + py*3;
			for( int i=0; i<3; i++)
				color_str[where+i] = scaled_color[i];
        }
    }
	return NULL;
}

void Output_handle(float * color_array, int width, int height){
	int x,y,p;
	for (x = 0; x < width; x++){
		for (y = 0; y < height; y++){
			p = x*height*3+y*3;
			printf( "%.0f %.0f %.0f\n",
		    color_array[p], color_array[p+1], color_array[p+2] );
		}
		printf("\n");
	}
}

int
main( int argc, char **argv )
{
    int nthreads = argc == 2 ? atoi( argv[1] ) : 0;

    if( nthreads < 1 )
    {
      fprintf( stderr, "%s: usage: %s NTHREADS\n", argv[0], argv[0] );
      return 1;
    }

    scene = create_sphereflake_scene( sphereflake_recursion ); //mark

    /* Write the image format header */
    /* P3 is an ASCII-formatted, color, PPM file */
    printf( "P3\n%d %d\n%d\n", width, height, max_color );
    printf( "# Rendering scene with %d spheres and %d lights\n",
            scene.sphere_count,
            scene.light_count );

    set( camera_pos, 0., 0., -4. ); //mark

    set( camera_dir, 0., 0., 1. ); //mark
    camera_fov = 75.0 * (PI/180.0); //mark

    set( bg_color, 0.8, 0.8, 1 ); //mark

    pixel_dx = tan( 0.5*camera_fov ) / ((double)width*0.5);	//mark
    pixel_dy = tan( 0.5*camera_fov ) / ((double)height*0.5); //mark
    subsample_dx
        = halfSamples  ? pixel_dx / ((double)halfSamples*2.0)
                       : pixel_dx; //mark
    subsample_dy
        = halfSamples ? pixel_dy / ((double)halfSamples*2.0)
                      : pixel_dy;

	// initialize the array
    color_str = (float *)malloc(sizeof(float) * width * height * 3);
	if(color_str == NULL){
		fprintf(stderr, "Allocating memory for color string failed.");
		exit(1);
	}

	// no need for too much threads
	nthreads = nthreads >= width ? width : nthreads;
	thread_num = nthreads;

	// store for thread ids
	pthread_t * thread_id = (pthread_t *)malloc(sizeof(pthread_t) * nthreads);
	if(thread_id == NULL){
		fprintf(stderr, "Allocating memory for thread id string failed.");
		exit(1);
	}

	int * sub_start = (int *)malloc(sizeof(int) * nthreads);
	if(sub_start == NULL){
		fprintf(stderr, "Allocating memory for starting point string failed.");
		exit(1);
	}

	int i = 0;
	int err;
	for (; i < nthreads; i++){
		sub_start[i] = i;
	}

	for (i = 0; i < nthreads; i++){
		if( (err = pthread_create(thread_id+i, NULL, parallel_render, sub_start + i)) != 0){
			fprintf(stderr, "Creating thread failure, exit number is %d", err);
			exit(1);
		}
	}

	for (i = 0; i < nthreads; i++){
		if(pthread_join(thread_id[i], NULL)){
			fprintf(stderr, "Error while merging threads");
			exit(1);
		}
	}

	Output_handle(color_str, width, height);

	//free space
	free(color_str);
	free(thread_id);
	free(sub_start);
    free_scene( &scene );

    if( ferror( stdout ) || fclose( stdout ) != 0 )
    {
        fprintf( stderr, "Output error\n" );
		exit(1);
    }

    return 0;
}
