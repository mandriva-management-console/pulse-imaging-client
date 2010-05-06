/*
 * (c) 2010 Mandriva, http://www.mandriva.com
 *
 * $Id$
 *
 * This file is part of Pulse 2, http://pulse2.mandriva.org
 *
 * Pulse 2 is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Pulse 2 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Pulse 2; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */

/**
 * @file timespec.h 
 * @brief function to compute with struct timespec values
 * @author Yann Droneaud <ydroneaud@mandriva.com>
 * 
 */
#if defined(__TIMESPEC_H__) && __TIMESPEC_H__ != 20100205
#error Include version mismatch
#elif !defined(__TIMESPEC_H__)
#define __TIMESPEC_H__ 20100205

#include <time.h>
#include <string.h>
#include <assert.h>

/* max value + 1 for tv_nsec field */
#define TIME_RESOLUTION (1000000000L)

#if defined(__STDC__) && __STDC__ >= 199900L
# define RESTRICT restrict
#elif defined(__GNUC__)
# define RESTRICT __restrict__
#else
# define RESTRICT
#endif

#if defined(__STDC__) && __STDC__ >= 199900L
# define INLINE inline
#elif defined(__GNUC__)
# define INLINE __inline__
#else
# define INLINE
#endif

/**
 * @brief copy the content of src to dst
 * @param[in] src
 * @param[out] dst
 */
static INLINE void
timespec_copy(struct timespec * RESTRICT dst, 
	      const struct timespec * RESTRICT src)
{
  assert(src->tv_sec >= 0);
  assert(src->tv_nsec >= 0);
  assert(src->tv_nsec < TIME_RESOLUTION);

  dst->tv_sec  = src->tv_sec;
  dst->tv_nsec = src->tv_nsec;
}

/**
 * @brief set the content of timespec to 0
 */
static INLINE void
timespec_reset(struct timespec *t) 
{
  t->tv_sec = (time_t) 0;
  t->tv_nsec = 0L;
}

/**
 * @brief set the value of a timespec
 */
static INLINE void
timespec_set(struct timespec *t, 
	     time_t sec,
	     long nsec)
{
  assert(sec >= 0);
  assert(nsec >= 0);
  assert(nsec < TIME_RESOLUTION);

  t->tv_sec = sec;
  t->tv_nsec = nsec;
}

/**
 * @brief compare two timespec value
 *
 * @param[in] x
 * @param[in] y
 *
 * @return 1 if x > y,
 *        -1 if x < y,
 *         0 if x = y
 */
static INLINE int
timespec_compare(const struct timespec * RESTRICT x,
		 const struct timespec * RESTRICT y)
{
  assert(x->tv_sec >= 0);
  assert(y->tv_sec >= 0);
  assert(x->tv_nsec >= 0);
  assert(y->tv_nsec >= 0);
  assert(x->tv_nsec < TIME_RESOLUTION);
  assert(y->tv_nsec < TIME_RESOLUTION);

  if (x->tv_sec > y->tv_sec) {
    return 1;
  } else if (x->tv_sec < y->tv_sec) {
    return -1;
  } else if (x->tv_nsec > y->tv_nsec) {
    return 1;
  } else if (x->tv_nsec < y->tv_nsec) {
    return -1;
  }

  /* equal */
  return 0;
}

/**
 * @brief add two timespec values and store the result
 *
 * @param[out] res
 * @param[in] x
 * @param[in] y
 * 
 * @return always 0
 *
 */
static INLINE int
timespec_add(struct timespec * RESTRICT res,
	     const struct timespec * RESTRICT x,
	     const struct timespec * RESTRICT y)
{
  assert(x->tv_sec >= 0);
  assert(y->tv_sec >= 0);
  assert(x->tv_nsec >= 0);
  assert(y->tv_nsec >= 0);
  assert(x->tv_nsec < TIME_RESOLUTION);
  assert(y->tv_nsec < TIME_RESOLUTION);

  res->tv_sec  = x->tv_sec  + y->tv_sec;
  res->tv_nsec = x->tv_nsec + y->tv_nsec;

  if (res->tv_nsec > TIME_RESOLUTION) {
    res->tv_sec += 1;
    res->tv_nsec %= TIME_RESOLUTION;
  }
  
  return 0;
}

/**
 * @brief substract two timespec values and store the result
 *
 * @param[out] res
 * @param[in] x
 * @param[in] y
 * 
 * the result is always positive.
 *
 * @return sign of the result, see timespec_compare()
 *
 */
static INLINE int
timespec_substract(struct timespec * RESTRICT res,
		   const struct timespec * RESTRICT x,
		   const struct timespec * RESTRICT y)
{
  int sign; 

  assert(x->tv_sec >= 0);
  assert(y->tv_sec >= 0);
  assert(x->tv_nsec >= 0);
  assert(y->tv_nsec >= 0);
  assert(x->tv_nsec < TIME_RESOLUTION);
  assert(y->tv_nsec < TIME_RESOLUTION);

  sign = timespec_compare(x,y);

  if (sign == 0) {
    timespec_reset(res);

    goto leave;
  } 

  /* x > y */
  if (sign > 0) {

    *res = *x;

    if (res->tv_nsec < y->tv_nsec) {
      res->tv_sec -= 1;
      res->tv_nsec += TIME_RESOLUTION - y->tv_nsec; 
    } else {
      res->tv_nsec -= y->tv_nsec; 
    }
    
    res->tv_sec -= y->tv_sec;

    goto leave;
  }

  /* y > x */
  if (sign < 0) {

    *res = *y;

    if (res->tv_nsec < x->tv_nsec) {
      res->tv_sec -= 1;
      res->tv_nsec += TIME_RESOLUTION - x->tv_nsec; 
    } else {
      res->tv_nsec -= x->tv_nsec; 
    }
    
    res->tv_sec -= x->tv_sec;

    goto leave;
  }

 leave:

  return sign;
}

#undef RESTRICT
#undef INLINE

#endif /* __TIMESPEC_H__ */
