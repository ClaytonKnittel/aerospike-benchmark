/*******************************************************************************
 * Copyright 2020 by Aerospike.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 ******************************************************************************/
#pragma once

#include <stddef.h>
#include <stdint.h>

#include <aerospike/as_record.h>
#include <aerospike/as_random.h>


// forward declaration (defined only in source file)
struct bin_spec;

struct obj_spec {
	struct bin_spec* bin_specs;
	uint32_t n_bin_specs;
};


/*
 * Initialize the given obj_spec struct according to the following format:
 *
 *    <bin-type>[,<bin-type>...]
 *
 * where the possible bin types are:
 *
 * Scalar bins:
 *	   I<bytes> | B<size> | S<length> | D # Default: I
 *
 *    I) Generate an integer bin or value in a specific byte range (treat I as I4)
 *        I1 for 0 - 255
 *        I2 for 256 - 65535
 *        I3 for 65536 - 2^24
 *        I4 for 2^24 - 2^32-1
 *        I5 for 2^32 - 2^40-1
 *        I6 for 2^40 - 2^48-1
 *        I7 for 2^48 - 2^56-1
 *        I8 for 2^56 - 2^64-1
 *    B) Generate a bytes bin or value with an bytearray of random bytes
 *        B12 - generates a bytearray of 12 random bytes
 *    S) Generate a string bin or value made of space-separated a-z{1,9} words
 *        S16 - a string with a 16 character length. ex: "uir a mskd poiur"
 *    D) Generate a Double bin or value (8 byte)
 *
 * Collection bins:
 *     [] - a list
 *         [3*I2] - ex: [312, 1651, 756]
 *         [I2, S4, I2] - ex: [892, "sf b", 8712]
 *         [2*S12, 3*I1] - ex: ["be lkr sqp s", "ndvi qd r fr", 18, 109, 212]
 *         [3*[I1, I1]] - ex: [[1,11],[123,321],[78,241]]
 *
 *     {} - a map
 *         {5*S1:I1} - ex {"a":1, "b":2, "d":4, "z":26, "e":5}
 *         {2*S1:[3*I:1]} - ex {"a": [1,2,3], "b": [6,7,8]}
 * 
 * Example:
 *     I2,S12,[3*I1] => b1: 478; b2: "dfoiu weop g"; b3: [12, 45, 209]
 */
int obj_spec_parse(struct obj_spec* base_obj, const char* obj_spec_str);

void obj_spec_free(struct obj_spec*);


/*
 * generates random values for each of the bings based on the object spec
 *
 * the bins are named as follows:
 * 	<bin_name_template>
 * 	<bin_name_template>_2
 * 	<bin_name_template>_3
 * 	...
 */
int obj_spec_populate_bins(const struct obj_spec*, as_record*, as_random*,
		const char* bin_name_template);


// define bin printing methods only for testing
#ifdef _TEST
void _dbg_sprint_obj_spec(const struct obj_spec* obj_spec, char* out_str,
		size_t str_size);
#endif /* _TEST */

