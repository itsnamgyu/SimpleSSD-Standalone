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

#include "bil/baseline_scheduler.hh"

#include "sil/none/none.hh"
#include "simplessd/hil/hil.hh"

namespace BIL {

BaselineScheduler::BaselineScheduler(Engine &e, DriverInterface *i)
    : Scheduler(e, i) {
  schedulerEvent =
      engine.allocateEvent([this](uint64_t tick) { invokeScheduler(tick); });
}

BaselineScheduler::~BaselineScheduler() {}

void BaselineScheduler::init() {
  bioPool = std::vector<std::vector<BIO>>((int)totalLogicalBlocks,
                                          std::vector<BIO>());
  bioDeadline = std::vector<ulong>((int)totalLogicalBlocks, 0);
}

void BaselineScheduler::submitIO(BIO &bio) {
  const uint64_t currentTick = engine.getCurrentTick();

  bio.requestedAt = currentTick;
  printf("----------------------------------------");
  printf("----------------------------------------\n");
  printf("[%06ld] TYPE:         %s\n", bio.id,
         bio.type == BIO_READ    ? "READ"
         : bio.type == BIO_WRITE ? "WRITE"
                                 : "OTHER");
  printf("[%06ld] SUBMIT_TICK:  %010ld\n", bio.id, currentTick);
  uint64_t startBlock = bio.offset / logicalBlockSize;
  uint64_t endBlock = (bio.offset + bio.length) / logicalBlockSize;
  printf("[%06ld] LBN:          %06ld - %06ld (%06ld)\n", bio.id, startBlock,
         endBlock, endBlock - startBlock + 1);

  if (bio.type == BIO_READ) {
    pInterface->submitIO(bio);
  }
  else {  // BIO_WRITE
    const uint64_t logicalBlockIndex = bio.offset / logicalBlockSize;
    assert(logicalBlockIndex < totalLogicalBlocks);
    bioPool[logicalBlockIndex].emplace_back(bio);
    if (bioDeadline[logicalBlockIndex] == 0) {
      bioDeadline[logicalBlockIndex] = currentTick + SCHEDULER_DEADLINE;
    }
    if (!writeScheduled) {
      engine.scheduleEvent(schedulerEvent, getNextDeadline());
    }
  }
}

void BaselineScheduler::invokeScheduler(uint64_t tick) {
  const uint64_t currentTick = engine.getCurrentTick();

  printf("[SCHEDULER] Invoke scheduler at %012ld\n", tick);

  // Find first block with expired deadline
  for (ulong i = 0; i < bioPool.size(); ++i) {
    if (bioDeadline[i] != 0 && bioDeadline[i] <= currentTick) {
      auto &pool = bioPool[i];

      printf("[SCHEDULER] Committing %012ld writes from block %012ld\n",
             pool.size(), i);
      for (auto &bio : pool) {
        pInterface->submitIO(bio);
      }
      pool.clear();
      bioDeadline[i] = 0;
      break;  // allow one read request before next block write
    }
  }

  const uint64_t nextDeadline = getNextDeadline();
  if (nextDeadline) {
    engine.scheduleEvent(schedulerEvent, nextDeadline);
  }
  else {
    writeScheduled = false;
  }
}

uint64_t BaselineScheduler::getNextDeadline() {
  auto minDeadline = (uint64_t)-1;
  for (const auto deadline : bioDeadline) {
    if (deadline != 0 && deadline < minDeadline) {
      minDeadline = deadline;
    }
  }

  if (minDeadline == (uint64_t)-1) {
    return 0;
  }
  return minDeadline;
}

}  // namespace BIL
