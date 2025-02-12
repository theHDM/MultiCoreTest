#pragma once
#include "timing.h"
#include "hardware/irq.h"       // library of code to let you interrupt code execution to run something of higher priority
#include <functional>
#include "debug.h"
#include <stdint.h>

// task manager takes a list of repeat periods and callback functions
// these should run in the background
// set an alarm to run the "on_irq" boolean every "tick" microseconds.
// load in the background functions you want using the "bind" language.

static void on_irq_hexBoard_task_manager();

class hexBoard_Task_Manager {
  protected:
    struct Task {
      int priority;
      uint64_t counter;
      uint64_t period;
      std::function<void()> exec_on_trigger;
      Task(int arg_priority, uint64_t arg_period, std::function<void()> arg_func)
      : priority(arg_priority), period(arg_period)
      , exec_on_trigger(arg_func), counter(0) {}
      void increment(uint64_t add_uS) {
        counter += add_uS;
      }
      void execute() {
        exec_on_trigger();
      }
      bool triggered() {
        if (counter >= period) {
          counter -= period;
          return true;
        }
        return false;
      }
    };
    uint32_t tick_uS;
    uint32_t next_trigger;
    uint8_t alarm_ID; // defined as a "byte" in the RP2040 hardware
    std::vector<Task> task_list;
  public:
    hexBoard_Task_Manager(uint32_t arg_uS) 
    : tick_uS(arg_uS), next_trigger(0), alarm_ID(0) {}
    uint32_t get_tick_uS() {
      return tick_uS;
    }
    void add_task(int priority, uint32_t arg_repeat_uS, std::function<void()> arg_on_trigger) {
      Task newTask(priority, arg_repeat_uS, arg_on_trigger);
      if (task_list.empty()) {
        task_list.emplace_back(newTask);
      } else {
        std::vector<Task>::iterator it = task_list.begin();
        for (auto it = task_list.begin(); it < task_list.end(); ++it) {
          if (it->priority >= priority) break;
        }
        task_list.insert(it, newTask);
      }
    }
    void reset_timer() {
      next_trigger += tick_uS;
      timer_hw->alarm[alarm_ID] = next_trigger;
    }
    void start_timer() {
      next_trigger = timer_hw->timerawl;
      reset_timer();
    }
    void begin() {
      hw_set_bits(&timer_hw->inte, 1u << alarm_ID);  // initialize the timer
      irq_set_exclusive_handler(alarm_ID, on_irq_hexBoard_task_manager);    // function to run every interrupt
      irq_set_enabled(alarm_ID, true);               // ENGAGE!
      start_timer();
    }
    void tick() {
      hw_set_bits(&timer_hw->intr, 1u << alarm_ID);
      reset_timer();
      for (auto& i : task_list) {
        i.increment(tick_uS);
      }
      // execute tasks strictly in order they were created
      for (unsigned i = 0; i < task_list.size(); ++i) {
        if (task_list[i].triggered()) {
          task_list[i].execute();
          return;
        }
      } 
    }
};

extern hexBoard_Task_Manager task_mgr;

static void on_irq_hexBoard_task_manager(void) {
  task_mgr.tick();
}