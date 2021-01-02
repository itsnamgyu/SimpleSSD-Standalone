/*
 * Copyright (C) 2017 CAMELab
 *
 * This file is part of SimpleSSD.
 *
 * SimpleSSD is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * SimpleSSD is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with SimpleSSD.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#ifndef __BIL_NOOP_SCHEDULER__
#define __BIL_NOOP_SCHEDULER__

#include "bil/scheduler.hh"
#include "queue"

namespace BIL {

class NoopScheduler : public Scheduler {
 public:
  NoopScheduler(Engine &, DriverInterface *);
  ~NoopScheduler();

  uint64_t totalLogicalBlocks = 3072;   // [NG] Sample config
  uint64_t logicalBlockSize = 3 << 27;  // [NG] Sample config

  std::queue<BIO> queue;

  const bool LIMIT_BUSYNESS = true;
  const uint64_t BUSYNESS_THRESHOLD = 32;
  ulong busy = 0;
  std::mutex m;

  const bool LOG_BLOCK_ADDRESS= true;
  const char *BLOCK_ADDRESS_LOG_FILE = "./block_address.txt";
  FILE *blockAddressLogFile;

  void init();
  void submitIO(BIO &);

 private:
  void _submitIO(BIO &);
};

}  // namespace BIL

#endif
