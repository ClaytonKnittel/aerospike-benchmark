/*******************************************************************************
 * Copyright 2008-2020 by Aerospike.
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

#include <aerospike/aerospike.h>
#include <aerospike/as_event.h>
#include <aerospike/as_password.h>
#include <aerospike/as_random.h>
#include <aerospike/as_record.h>

#include <hdr_histogram/hdr_histogram.h>
#include <histogram.h>
#include <latency.h>
#include <object_spec.h>
#include <workload.h>

// forward declare thr_coordinator for use in threaddata
struct thr_coordinator;

typedef enum {
	LEN_TYPE_COUNT,
	LEN_TYPE_BYTES,
	LEN_TYPE_KBYTES
} len_type;

typedef struct arguments_t {
	char* hosts;
	int port;
	const char* user;
	char password[AS_PASSWORD_SIZE];
	const char* namespace;
	const char* set;
	const char* bin_name;
	uint64_t start_key;
	uint64_t keys;
	/*char bintype;
	  int binlen;
	  int numbins;
	  len_type binlen_type;*/
	struct stages stages;

	// the default object spec, in the case that a workload stage isn't defined
	// with one
	struct obj_spec obj_spec;
	// set to true when stages have been initialized by a yaml file
	bool stages_init_by_file;

	bool random;
	/*bool init;
	  int init_pct;
	  int read_pct;
	  bool del_bin;*/

	//uint64_t transactions_limit;
	int transaction_worker_threads;
	int throughput;
	int batch_size;
	bool enable_compression;
	float compression_ratio;
	int read_socket_timeout;
	int write_socket_timeout;
	int read_total_timeout;
	int write_total_timeout;
	int max_retries;
	bool debug;
	bool latency;
	int latency_columns;
	int latency_shift;
	as_vector latency_percentiles;
	bool latency_histogram;
	char* histogram_output;
	int histogram_period;
	char* hdr_output;
	bool use_shm;
	as_policy_replica replica;
	as_policy_read_mode_ap read_mode_ap;
	as_policy_read_mode_sc read_mode_sc;
	as_policy_commit_level write_commit_level;
	int conn_pools_per_node;
	bool durable_deletes;
	bool async;
	int async_max_commands;
	int event_loop_capacity;
	as_config_tls tls;
	as_auth_mode auth_mode;
} arguments;

typedef struct clientdata_t {
	const char* namespace;
	const char* set;
	const char* bin_name;
	struct stages stages;

	uint64_t transactions_count;
	uint64_t period_begin;

	aerospike client;
	as_val *fixed_value;

	// TODO make all these counts thread-local to reduce contention
	uint32_t write_count;
	uint32_t write_timeout_count;
	uint32_t write_error_count;

	uint32_t read_count;
	uint32_t read_timeout_count;
	uint32_t read_error_count;

	latency write_latency;
	latency read_latency;

	struct hdr_histogram* read_hdr;
	struct hdr_histogram* write_hdr;
	as_vector latency_percentiles;

	FILE* histogram_output;
	int histogram_period;
	histogram write_histogram;
	histogram read_histogram;

	FILE* hdr_comp_write_output;
	FILE* hdr_text_write_output;
	FILE* hdr_comp_read_output;
	FILE* hdr_text_read_output;
	struct hdr_histogram* summary_read_hdr;
	struct hdr_histogram* summary_write_hdr;

	uint32_t tdata_count;

	int async_max_commands;
	int transaction_worker_threads;
	int throughput;
	int batch_size;
	int read_pct;
	struct obj_spec obj_spec;

	float compression_ratio;
	// when true, random records are generated for every write transaction,
	// otherwise a single fixed value is set at the beginning and used for
	// every transaction
	bool random;
	bool latency;
	bool debug;
	bool async;

} clientdata;

struct threaddata {
	clientdata* cdata;
	struct thr_coordinator* coord;
	as_random* random;

	// thread index: [0, n_threads)
	uint32_t t_idx;
	// which workload stage we're currrently on
	uint32_t stage_idx;

	/*
	 * note: to stop threads, tdata->finished must be set before tdata->do_work
	 * to prevent deadlocking
	 */
	// when true, things continue as normal, when set to false, worker
	// threads will stop doing what they're doing and await orders
	bool do_work;
	// when true, all threads will stop doing work and close (note that do_work
	// must also be set to false for this to work)
	bool finished;
};

int run_benchmark(arguments* args);
int linear_write(clientdata* data);
int random_read_write(clientdata* data);

//bool write_record_sync(clientdata* cdata, threaddata* tdata, uint64_t key);
//int read_record_sync(clientdata* cdata, threaddata* tdata);
//int batch_record_sync(clientdata* cdata, threaddata* tdata);
void throttle(const clientdata* cdata, const struct stage*);

//void linear_write_async(clientdata* cdata, threaddata* tdata, as_event_loop* event_loop);
//void random_read_write_async(clientdata* cdata, threaddata* tdata, as_event_loop* event_loop);

bool is_stop_writes(aerospike* client, const char* namespace);
