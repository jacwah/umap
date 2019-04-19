//////////////////////////////////////////////////////////////////////////////
// Copyright 2017-2019 Lawrence Livermore National Security, LLC and other
// UMAP Project Developers. See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: LGPL-2.1-only
//////////////////////////////////////////////////////////////////////////////
#ifndef _UMAP_PageFlusher_HPP
#define _UMAP_PageFlusher_HPP

#include "umap/config.h"

#include <vector>

#include "umap/Buffer.hpp"
#include "umap/Flushers.hpp"
#include "umap/Uffd.hpp"
#include "umap/WorkerPool.hpp"
#include "umap/util/Macros.hpp"
#include "umap/store/Store.hpp"

namespace Umap {
  class PageFlusher : WorkerPool {
    public:
      PageFlusher(
            uint64_t num_flushers, Buffer* buffer, Uffd* uffd, Store* store) :
              WorkerPool("Page Flusher", 1), m_buffer(buffer), m_store(store)
      {
        m_page_flushers = new Flushers(PageRegion::getInstance()->get_num_flushers(), m_buffer, uffd);

        start_thread_pool();
      }

      ~PageFlusher( void ) {
        FlushAll();
        delete m_page_flushers;
      }

      void FlushAll( void ) {
        m_buffer->lock();
        for ( auto pd = m_buffer->get_oldest_present_page_descriptor(); pd != nullptr; pd = m_buffer->get_oldest_present_page_descriptor()) {
          WorkItem work;

          work.page_desc = pd;

          if (pd->page_is_dirty())
            work.store = m_store;
          else
            work.store = nullptr;

          m_page_flushers->send_work(work);
        }
        m_buffer->unlock();
      }

    private:
      Buffer* m_buffer;
      Store* m_store;
      Flushers* m_page_flushers;

      inline void ThreadEntry() {
        while ( ! time_to_stop_thread_pool() ) {
          std::vector<PageDescriptor*> page_descs;

          sleep(1);
        }
    }
  };
} // end of namespace Umap

#endif // _UMAP_PageFlusher_HPP
