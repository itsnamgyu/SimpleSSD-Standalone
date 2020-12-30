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

namespace BIL {

class NoopScheduler : public Scheduler {
 public:
  NoopScheduler(Engine &, DriverInterface *);
  ~NoopScheduler();

  const uint64_t SCHEDULER_INTERVAL = 10UL * 1000UL * 1000UL;
//  const uint64_t SCHEDULER_DEADLINE = 1000UL * 1000UL * 1000UL * 1000UL;
  const uint64_t SCHEDULER_DEADLINE = 1UL * 1000UL * 1000UL;

  uint64_t totalLogicalBlocks = 3072;  // [NG] Sample config
  SimpleSSD::Event scheduleEvent;
  std::vector<std::vector<BIO>> bioPool;

  void init();
  void submitIO(BIO &);
  void invokeScheduler(uint64_t);
};

}  // namespace BIL

#endif
