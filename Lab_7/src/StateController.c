#include "../include/StateController.h"
#include <xc.h>

#define FAULT_INPUT PORTBbits.RB0
#define ARM_INPUT PORTBbits.RB1
#define SAFE_LED LATDbits.LATD3
#define ARMING_LED LATDbits.LATD5
#define ARMED_LED LATDbits.LATD4
#define FAULT_LED LATDbits.LATD1
#define HEARTBEAT_LED LATDbits.LATD2

static DroneState state = SAFE;
static MotionState motion = STABLE;
static uint8_t sensor_ready = 0;
static uint8_t heartbeat = 0;
static uint8_t arm_seen = 0;
static uint8_t comm_seen = 0;
static uint8_t sample_seen = 0;
static uint8_t excessive_seen = 0;
static uint32_t arm_time = 0;
static uint32_t comm_time = 0;
static uint32_t last_arm_event = 0;
static uint32_t last_comm_event = 0;
static uint32_t heartbeat_time = 0;
static uint32_t excessive_time = 0;
static int16_t old_ax = 0;
static int16_t old_ay = 0;
static int16_t old_az = 0;

static int16_t abs16(int16_t value) { return value < 0 ? -value : value; }

static void change_state(DroneState next, uint32_t tick) {
  if (state != next) {
    state = next;
    heartbeat = 0;
    heartbeat_time = tick;
    excessive_seen = 0;
  }
}

static void update_outputs(void) {
  SAFE_LED = (state == SAFE);
  ARMING_LED = (state == ARMING);
  ARMED_LED = (state == ARMED);
  FAULT_LED = (state == FAULT);
  HEARTBEAT_LED = heartbeat;
}

void fsm_init(uint8_t sensor_ok) {
  sensor_ready = sensor_ok;
  state = sensor_ok ? SAFE : FAULT;
  motion = STABLE;
  heartbeat = 0;
  update_outputs();
}

void fsm_run(uint32_t tick, uint8_t fault, uint8_t arm, uint8_t comm) {
  uint16_t interval;

  if (fault) {
    change_state(FAULT, tick);
  } else {
    if (arm) {
      if (arm_seen && (uint32_t)(tick - last_arm_event) < 50u) {
        arm = 0;
      } else {
        last_arm_event = tick;
        arm_seen = 1;
      }
    }

    if (comm) {
      if (comm_seen && (uint32_t)(tick - last_comm_event) < 50u) {
        comm = 0;
      } else {
        last_comm_event = tick;
        comm_seen = 1;
      }
    }

    switch (state) {
    case SAFE:
      if (sensor_ready && arm && ARM_INPUT == 0) {
        change_state(ARMING, tick);
        arm_time = tick;
      }
      break;

    case ARMING:
      if (ARM_INPUT != 0) {
        change_state(SAFE, tick);
      } else if ((uint32_t)(tick - arm_time) >= 2000u) {
        change_state(ARMED, tick);
        comm_time = tick;
      }
      break;

    case ARMED:
      if (ARM_INPUT != 0) {
        change_state(SAFE, tick);
      } else {
        if (comm) {
          comm_time = tick;
        }
        if ((uint32_t)(tick - comm_time) >= 3000u) {
          change_state(FAULT, tick);
        }
      }
      break;

    case FAULT:
      if (sensor_ready && FAULT_INPUT != 0 && ARM_INPUT != 0 &&
          motion != EXCESSIVE) {
        change_state(SAFE, tick);
      }
      break;
    }
  }

  if (state == FAULT) {
    heartbeat = 0;
  } else {
    if (state == SAFE) {
      interval = 500;
    } else if (state == ARMING) {
      interval = 200;
    } else {
      interval = 1000;
    }

    if ((uint32_t)(tick - heartbeat_time) >= interval) {
      heartbeat = !heartbeat;
      heartbeat_time = tick;
    }
  }

  update_outputs();
}

void fsm_motion(int16_t ax, int16_t ay, int16_t az, uint32_t tick) {
  int32_t x = ax;
  int32_t y = ay;
  int32_t z = az;
  uint32_t magnitude_sq = (uint32_t)(x * x + y * y + z * z);

  if (magnitude_sq > 2560000UL || magnitude_sq < 160000UL) {
    motion = EXCESSIVE;
  } else if (sample_seen &&
             (abs16(ax - old_ax) > 100 || abs16(ay - old_ay) > 100 ||
              abs16(az - old_az) > 100)) {
    motion = MOVING;
  } else if (abs16(ax) > 500 || abs16(ay) > 500) {
    motion = TILTED;
  } else {
    motion = STABLE;
  }

  old_ax = ax;
  old_ay = ay;
  old_az = az;
  sample_seen = 1;

  if (state == ARMED && motion == EXCESSIVE) {
    if (!excessive_seen) {
      excessive_seen = 1;
      excessive_time = tick;
    } else if ((uint32_t)(tick - excessive_time) >= 200u) {
      change_state(FAULT, tick);
      update_outputs();
    }
  } else {
    excessive_seen = 0;
  }
}

DroneState fsm_state(void) { return state; }

MotionState fsm_motion_state(void) { return motion; }

const char *fsm_state_name(void) {
  switch (state) {
  case SAFE:
    return "SAFE";
  case ARMING:
    return "ARMING";
  case ARMED:
    return "ARMED";
  case FAULT:
    return "FAULT";
  }
  return "FAULT";
}

const char *fsm_motion_name(void) {
  switch (motion) {
  case STABLE:
    return "STABLE";
  case MOVING:
    return "MOVING";
  case TILTED:
    return "TILTED";
  case EXCESSIVE:
    return "EXCESSIVE";
  }
  return "STABLE";
}
