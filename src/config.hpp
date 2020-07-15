/*****************************************************************************
 *   kmtricks
 *   Authors: T. Lemane
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as
 *  published by the Free Software Foundation, either version 3 of the
 *  License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*****************************************************************************/

#pragma once
#include <string>
#include <gatb/gatb_core.hpp>
#include <fmt/format.h>

using namespace std;
typedef const string cs;

/*
├── logs
│   ├── counter[N].log
│   ├── merger[N].log
│   ├── partitioner[N].log
│   └── superk[N].log
├── storage
│   ├── config_storage_gatb
|   ├── partition_storage_gatb
│   ├── fof.txt
│   ├── kmers_partitions
│   ├── matrix
│   ├── superk_partitions
│   └── vectors
│       ├── howde
│       └── sdsl
└── synchro
    ├── counter
    ├── merger
    ├── partitioner
    └── superk
*/

// arg flags not in gatb core
cs STR_NOHUP        = "nohup";
cs STR_DIR_SYNCHRO  = "-dir-synchro";
cs STR_RUN_DIR      = "-run-dir";
cs STR_BIN_DIR      = "-bin-dir";
cs STR_MIN_HASH     = "-min-hash";
cs STR_MAX_HASH     = "-max-hash";
cs STR_REC_MIN      = "-recurrence-min";
cs STR_PART_ID      = "-part-id";
cs STR_MODE         = "-mode";
cs STR_NB_PROC      = "-nb-procs";
cs STR_MAX_M_C      = "-mem-per-proc";
cs STR_COUNT_SIZE   = "-count-size";
cs STR_HASHER       = "-hasher";
cs STR_HASH_SEED    = "-hash-seed";
cs STR_MAT_FMT      = "-matrix-fmt";
cs STR_SPLIT        = "-split";
cs STR_UP_TO        = "-until";
cs STR_ONLY         = "-only";
cs STR_HSIZE        = "-hsize";

// commands
cs PARTITIONER_CMD  = 
  "{} {} -file {} -kmer-size {} -nb-cores {} -run-dir {} -dir-synchro {} &> {} &";
cs SUPERK_CMD       = 
  "{} {} -file {} -run-dir {} -kmer-size {} -dir-synchro {} -nb-cores {} &> {} &";
cs COUNTER_CMD      = 
  "{} {} -file {} -run-dir {} -kmer-size {} -abundance-min {} -max-hash {} -mode {} -nb-cores {} -part-id {} -hasher {} &> {} &";
cs MERGER_CMD       = 
  "{} {} -file {} -run-dir {} -min-hash {} -max-hash {} -part-id {} -abundance-min {} -recurrence-min {} -mode {} &> {} &";


cs TEMP_S           = "/{}.superk";

// end template
cs END_TEMP_P       = "/partitioner.sync";
cs END_TEMP_S       = "/superk_{}.sync";
cs END_TEMP_C       = "/counter_{}.sync";
cs END_TEMP_M       = "/merger_{}.sync";

cs PART_DIR         = "/partition_{}";
cs PART_TEMP_K      = "/partition_{}/{}.kmer";
cs PART_TEMP_K_F    = "/{}.kmers";

cs PA_TEMP          = "/partition_{}/pa_matrix{}.mat";
cs BF_NT_TEMP       = "/partition_{}/no_trp_bf{}.mat";
cs BF_T_TEMP       = "/partition_{}/trp_bf{}.mat";
cs AS_TEMP          = "/partition_{}/ascii_matrix{}.mat";
cs CO_TEMP          = "/partition_{}/count_matrix{}.mat";

cs MAT_TEMP         = "/partition_{}/{}.mat";

cs CONFIG_GRP       = "config";
cs REPART_GRP       = "minimRepart";

cs KILLALL          = "killall km_part km_superk km_count km_merge &> /dev/null";

  class Env
{
public:
  Env(string main_dir, string binaries_dir);
  ~Env();
  void build();
  void build_p(uint p);

public:
  string DIR;
  string BIN;

  string SYNCHRO;
  string STORE;
  string LOG;

  // binaries
  string PARTITIONER_BIN;
  string SUPERK_BIN; 
  string COUNTER_BIN;
  string MERGER_BIN;
  
  // synchro
  string SYNCHRO_P;
  string SYNCHRO_S;
  string SYNCHRO_C;
  string SYNCHRO_M;

public:
  // storage
  string STORE_SUPERK;
  string STORE_KMERS;
  string STORE_MATRIX;
  string STORE_VECTORS;
  string STORE_SDSL;
  string STORE_HOWDE;
  string STORE_CONFIG;
  string STORE_PART;
  string FOF_FILE;

  // log
  string LOG_SUPERK_D;
  string LOG_COUNTER_D;
  string LOG_MERGER_D;
  string LOG_PARTITIONER;
  string LOG_SUPERK;
  string LOG_COUNTER;
  string LOG_MERGER;
  string LOG_CMD;
};

Env::Env(string main_dir, string binaries_dir)
  : DIR(main_dir), BIN(binaries_dir)
{

  SYNCHRO          = DIR + "/synchro";
  STORE            = DIR + "/storage";
  LOG              = DIR + "/logs";
  // binaries
  PARTITIONER_BIN  = BIN + "/km_part";
  SUPERK_BIN       = BIN + "/km_superk";
  COUNTER_BIN      = BIN + "/km_count";
  MERGER_BIN       = BIN + "/km_merge";

  // synchro
  SYNCHRO_P        = SYNCHRO + "/partitioner";
  SYNCHRO_S        = SYNCHRO + "/superk";
  SYNCHRO_C        = SYNCHRO + "/counter";
  SYNCHRO_M        = SYNCHRO + "/merger";

  // storage
  STORE_SUPERK     = STORE + "/superk_partitions";
  STORE_KMERS      = STORE + "/kmers_partitions";
  STORE_MATRIX     = STORE + "/matrix";
  STORE_VECTORS    = STORE + "/vectors";
  STORE_SDSL       = STORE + "/vectors/sdsl";
  STORE_HOWDE      = STORE + "/vectors/howde";
  STORE_CONFIG     = STORE + "/config_storage";
  STORE_PART       = STORE + "/partition_storage";
  FOF_FILE         = STORE + "/fof.txt";
  // log files

  LOG_SUPERK_D     = LOG + "/superk";
  LOG_COUNTER_D    = LOG + "/counter";
  LOG_MERGER_D     = LOG + "/merger";
  LOG_CMD          = LOG + "/cmds.log";

  LOG_PARTITIONER  = LOG + "/partitioner.log";
  LOG_SUPERK       = LOG_SUPERK_D + "/superk{}.log";
  LOG_COUNTER      = LOG_COUNTER_D + "/counter{}_{}.log";
  LOG_MERGER       = LOG_MERGER_D + "/merger{}.log";
  
  
}

Env::~Env() {};

void Env::build()
{
  System::file().mkdir(DIR, -1);
  System::file().mkdir(SYNCHRO, -1);
  System::file().mkdir(STORE, -1);
  System::file().mkdir(LOG, -1);
  System::file().mkdir(SYNCHRO_P, -1);
  System::file().mkdir(SYNCHRO_S, -1);
  System::file().mkdir(SYNCHRO_C, -1);
  System::file().mkdir(SYNCHRO_M, -1);
  System::file().mkdir(STORE_SUPERK, -1);
  System::file().mkdir(STORE_KMERS, -1);
  System::file().mkdir(STORE_MATRIX, -1);
  System::file().mkdir(STORE_VECTORS, -1);
  System::file().mkdir(STORE_SDSL, -1);
  System::file().mkdir(STORE_HOWDE, -1);
  System::file().mkdir(LOG_SUPERK_D, -1);
  System::file().mkdir(LOG_COUNTER_D, -1);
  System::file().mkdir(LOG_MERGER_D, -1);
}

void Env::build_p(uint p)
{
  for (int i=0; i<p; i++)
  {
    System::file().mkdir(fmt::format(STORE_KMERS + PART_DIR, i), -1);
    System::file().mkdir(fmt::format(STORE_MATRIX + PART_DIR, i), -1);
  }
}
