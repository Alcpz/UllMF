/*
 * Copyright (c) 2014 Universidad de La Laguna <cap@pcg.ull.es>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 */

/**
 * @file
 * Internal functions for time measurement. Mostly substituted with MPI_Wtime()
 * @ingroup internalapi
 */

#ifndef ULLMF_TIMER_H
#define ULLMF_TIMER_H

/**
 * Returns nanoseconds since some unspecified starting point.
 *
 * Uses the highest precision POSIX timer available.
 * Requires POSIX timer support.
 *
 * @return Timestamp in nanoseconds.
 */
unsigned long long nanotimestamp();

/**
 * Returns milliseconds since some unspecified starting point.
 *
 * Uses the highest precision POSIX timer available.
 * Requires POSIX timer support.
 *
 * @return Timestamp in milliseconds.
 */
unsigned long long millitimestamp();

#endif /*ULLMF_TIMER_H*/
