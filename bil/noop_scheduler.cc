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

#include "bil/noop_scheduler.hh"
#include "simplessd/hil/hil.hh"
#include "sil/none/none.hh"

namespace BIL {

NoopScheduler::NoopScheduler(Engine &e, DriverInterface *i) : Scheduler(e, i) {
  scheduleEvent = engine.allocateEvent([this](uint64_t tick) {
    invokeScheduler(tick);
    engine.scheduleEvent(scheduleEvent, tick + SCHEDULER_INTERVAL);
  });
  engine.scheduleEvent(scheduleEvent,
                       engine.getCurrentTick() + SCHEDULER_INTERVAL);
}

NoopScheduler::~NoopScheduler() {}

void NoopScheduler::init() {
  bioPool = std::vector<std::vector<BIO>>((int) totalLogicalBlocks, std::vector<BIO>());
}

void NoopScheduler::submitIO(BIO &bio) {
  static int index = 0;
  const uint64_t logicalBlockSize = 3 << 27;
  const uint64_t currentTick = engine.getCurrentTick();

  bio.requestedAt = currentTick;
  index++;
  printf("--------------------------------------------------------------------------------\n");
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
  } else {
    const uint64_t logicalBlockIndex = bio.offset / logicalBlockSize;
    assert(logicalBlockIndex < totalLogicalBlocks);
    bioPool[logicalBlockIndex].emplace_back(bio);
  }
}

void NoopScheduler::invokeScheduler(uint64_t tick) {
  auto blocksToCommit = std::vector<ulong>();
  const uint64_t currentTick = engine.getCurrentTick();

  printf("Scheduler function at %06ld\n", tick);

  // Determine which blocks to commit (when a BIO is passed its deadline
  for (ulong i = 0; i < bioPool.size(); ++i) {
    for (const auto &bio : bioPool[i]) {
      const uint64_t elapsed = currentTick - bio.requestedAt;
      if (elapsed > SCHEDULER_DEADLINE) {
        printf("ELAPSED %ld\n", elapsed);
        blocksToCommit.emplace_back(i);
        break;
      }
    }
  }

  // Commit IO
  if (!blocksToCommit.empty()) {
    printf("Scheduler committing %ld blocks!\n", blocksToCommit.size());

    for (const auto &block : blocksToCommit) {
      auto &pool = bioPool[block];

      printf("Scheduling %ld blocks from %ld\n", pool.size(), block);
      for (auto &bio : pool) {
        pInterface->submitIO(bio);
      }
      pool.clear();
    }
  }
}

}  // namespace BIL
