/* -*- mode: c++ -*-
 * Kaleidoscope-LED-Stalker -- Stalk keys pressed by lighting up and fading back the LED under them
 * Copyright (C) 2017, 2018  Keyboard.io, Inc
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <Kaleidoscope-LED-Stalker.h>
#include <LEDUtils.h>

namespace kaleidoscope {

uint8_t StalkerEffect::map_[ROWS][COLS];
StalkerEffect::ColorComputer *StalkerEffect::variant;
uint16_t StalkerEffect::step_length = 50;
uint16_t StalkerEffect::step_start_time_;

void StalkerEffect::onActivate(void) {
  memset(map_, 0, sizeof(map_));
}

EventHandlerResult StalkerEffect::onKeyswitchEvent(Key &mapped_key, byte row, byte col, uint8_t keyState) {
  if (row >= ROWS || col >= COLS)
    return EventHandlerResult::OK;

  if (keyIsPressed(keyState)) {
    map_[row][col] = 0xff;
  }

  return EventHandlerResult::OK;
}

void StalkerEffect::update(void) {
  if (!variant)
    return;

  uint16_t now = millis();
  bool time_out = (now - step_start_time_) > step_length;

  for (byte r = 0; r < ROWS; r++) {
    for (byte c = 0; c < COLS; c++) {
      uint8_t step = map_[r][c];
      if (step) {
        ::LEDControl.setCrgbAt(r, c, variant->compute(&step));
      }

      if (time_out) {
        map_[r][c] = step;
      }

      if (!map_[r][c])
        ::LEDControl.setCrgbAt(r, c, (cRGB) {
        0, 0, 0
      });
    }
  }

  if (time_out)
    step_start_time_ = now;
}

namespace stalker {

cRGB Haunt::highlight_color_;

// Haunt
Haunt::Haunt(const cRGB highlight_color) {
  highlight_color_ = highlight_color;
}

cRGB Haunt::compute(uint8_t *step) {
  cRGB color = CRGB((uint8_t)min(*step * highlight_color_.r / 255, 255),
                    (uint8_t)min(*step * highlight_color_.g / 255, 255),
                    (uint8_t)min(*step * highlight_color_.b / 255, 255));

  if (*step >= 0xf0)
    *step -= 1;
  else if (*step >= 0x40)
    *step -= 16;
  else if (*step >= 32)
    *step -= 32;
  else
    *step = 0;

  return color;
}

// BlazingTrail
BlazingTrail::BlazingTrail(void) {
}

uint8_t hue_start = 50.0 / 360 * 0xff;
uint8_t hue_end = 0;
cRGB BlazingTrail::compute(uint8_t *step) {
  cRGB color;

  // Get a float from 0 to 1 for our position in the animation
  float pos = (0xff - *step) / (float) 0xff;

  // Fade hue linearly from orange to red
  uint8_t hue = hue_start + pos * (hue_end - hue_start);

  // Fade value from full following a -x^4+1 curve
  uint8_t val = 0xff * (1 - pos * pos * pos * pos);

  color = hsvToRgb(hue, 0xff, val);

  if (*step < 4) {
    *step = 0;
  } else {
    *step -= 4;
  }

  return color;
}

// Rainbow
Rainbow::Rainbow(void) {
}

cRGB Rainbow::compute(uint8_t *step) {
  if (*step > 0)
    *step -= 1;
  else
    *step = 0;

  return hsvToRgb(255 - *step, 255, *step);
}

}

}

kaleidoscope::StalkerEffect StalkerEffect;
