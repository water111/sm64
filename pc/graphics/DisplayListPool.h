/*!
 * @file DisplayListPool.h
 * Holds a three display lists.
 */

#ifndef SM64_DISPLAYLISTPOOL_H
#define SM64_DISPLAYLISTPOOL_H

#include <cassert>
#include <mutex>
#include "DisplayList.h"



class DisplayListPool {
public:
  constexpr static uint32_t N_LISTS = 3;

  enum class State {
    WAITING,
    LOCKED,
    LOADING,
  };

  struct List {
    State state = State::WAITING;
    int32_t age = -1;
    DisplayList list;
    bool drawn = false;
  };

  DisplayListPool(uint32_t node_count, uint32_t list_pool_size) {
    for(auto& l : lists) l.list.init_mem(node_count, list_pool_size);
  };

  /*!
   * Get the oldest list, which can then be filled with data.
   * Get the oldest list which is WAITING, mark it as LOADING, and return.
   */
  DisplayList* alloc() {
    int32_t winner = -1;
    int32_t winner_age = -2;

    lock.lock();
    for(int i = 0; i < N_LISTS; i++) {
      assert(lists[i].state != State::LOADING);
      if(lists[i].state == State::WAITING && lists[i].age > winner_age) {
        winner = i;
        winner_age = lists[i].age;
      }
    }

    assert(winner != -1);

    lists[winner].state = State::LOADING;
    lock.unlock();
    return &lists[winner].list;
  }

  /*!
   * Indicate that this list is now fully loaded.
   * It is no longer safe to modify this list.
   */
  void finish_loading(DisplayList* list) {
    lock.lock();
    bool found = false;

    for(auto& l : lists) {
      if(list == &l.list) {
        found = true;
        assert(l.state == State::LOADING);
        l.state = State::WAITING;
        l.age = 0;
        l.drawn = false;
      }
      l.age++;
    }

    assert(found);
    lock.unlock();
  }

  /*!
   * Get the most recent fully loaded list.
   * Mark it as LOCKED until render_complete.
   */
  DisplayList* get_list_for_rendering() {
    int32_t winner = -1;
    int32_t winner_age = INT32_MAX;

    lock.lock();
    for(int i = 0; i < N_LISTS; i++) {
      assert(lists[i].state != State::LOCKED);
      if(lists[i].state == State::WAITING && lists[i].age < winner_age) {
        winner = i;
        winner_age = lists[i].age;
      }
    }

    assert(winner != -1);
    if(lists[winner].drawn) {
      printf("[WARNING] Game logic stall! The same frame may be displayed twice.\n");
    }
    lists[winner].state = State::LOCKED;
    lock.unlock();
    return &lists[winner].list;
  }

  /*!
   * Indicate that a displaylist has been rendered.
   * After this, it is no longer safe to read from this list.
   */
  void finish_rendering(DisplayList* list) {
    lock.lock();
    bool found = false;

    for(auto& l : lists) {
      if(list == &l.list) {
        found = true;
        last_rendered = &l - lists;
        assert(l.state == State::LOCKED);
        l.state = State::WAITING;
        l.drawn = true;
      }
    }

    assert(found);

    lock.unlock();
  }

  int last_rendered = -1;
  List lists[N_LISTS];
  std::mutex lock;
};
#endif //SM64_DISPLAYLISTPOOL_H
