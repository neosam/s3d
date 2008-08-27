/*
 * File misc.h
 *
 * s3d is the legal property of Simon Goller (neosam@gmail.com).
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */


/*
 * This file contains some helper functions for c.
 */

#ifndef MISC_H
#define MISC_H

#include <stdlib.h>

/*
 * Memory allocation
 */
#define MALLOC(t) (t*)malloc(sizeof(t))
#define MALLOCN(t, n) (t*)malloc(sizeof(t) * n)
#define REALLOC(t, ptr) (t*)realloc(ptr, sizeof(t))
#define REALLOCN(t, ptr, n) (t*)realloc(ptr, sizeof(t) * n)

#endif
