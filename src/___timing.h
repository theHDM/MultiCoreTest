#pragma once
#include <stdint.h>
#include "pico/time.h"

uint32_t now() {
  return (temp << 32) | ;
}
class Task_Alarm {
  protected:
    uint64_t startTime = 0;
    uint64_t alarm_uS = 0;
    uint64_t defer_uS = 0;
    bool running = false;
    bool finishNow = false;
    void start_flags() {
      running = true;
      finishNow = false;
    }
  public:
    Task_Alarm();
    Task_Alarm(uint64_t _alarm_uS) {
      alarm_uS = _alarm_uS;
    };
    Task_Alarm(uint64_t _alarm_uS, uint64_t _defer_uS) {
      alarm_uS = _alarm_uS;
      defer_uS = _defer_uS;
    };
    void start() {
      startTime = now() + defer_uS;
      start_flags();
    }
    void wait_then_start(uint64_t _defer_uS) {
      defer_uS = _defer_uS;
      start();
    };
    void stop() {
      running = false;
      finishNow = false;
    }
    void repeat() {
      startTime += alarm_uS;
      start_flags();
    }
    void restart() {
      wait_then_start(0);
    }
    void finish_early() {
      finishNow = true;
    }
    bool justFinished() {
      if (running && (finishNow || (getElapsed() >= alarm_uS))) {
        stop();
        return true;
      } // else {
      return false;  
    }
    bool isRunning() {
      return running;
    }
    bool ifDone_thenRepeat() {
      if (justFinished()) {
          repeat();
          return true;
      }
      return false;
    }    
    bool ifDone_thenRestart() {
      if (justFinished()) {
          restart();
          return true;
      }
      return false;
    }    
    uint64_t getStartTime() {
      return startTime;  
    }
    uint64_t getElapsed() {
      uint64_t temp = now();
      return (temp < startTime ? 0 : temp - startTime);
    }
    uint64_t getRemaining() {
      if (running) {
        uint64_t temp = getElapsed();
        if (finishNow || (temp >= alarm_uS)) {
          return 0;
        } else {
          return (alarm_uS - temp);
        }
      } else {
        return 0;
      }  
    }
    uint64_t getAlarmPeriod()  {
      return alarm_uS;
    }
};