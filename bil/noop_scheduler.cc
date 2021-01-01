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

#include "sil/none/none.hh"
#include "simplessd/hil/hil.hh"

namespace BIL {

NoopScheduler::NoopScheduler(Engine &e, DriverInterface *i) : Scheduler(e, i) {}

NoopScheduler::~NoopScheduler() {}

void NoopScheduler::init() {}

void NoopScheduler::submitIO(BIO &bio) {
  const uint64_t currentTick = engine.getCurrentTick();

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

  pInterface->submitIO(bio);
}

}  // namespace BIL
