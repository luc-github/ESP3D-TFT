/*
  esp3d_tft

  Copyright (c) 2022 Luc Lebosse. All rights reserved.

  This code is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This code is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include <string>

#include "esp3d_hal.h"
#include "esp3d_log.h"
#include "lvgl.h"
#include "version.h"

LV_IMG_DECLARE(logo_800_480_BW);
#define LV_TICK_PERIOD_MS 10
/**********************
 *  STATIC PROTOTYPES
 **********************/
static lv_timer_t *boot_timer = NULL;
static lv_style_t style_btn;
static lv_style_t style_btn_pressed;
static lv_style_t style_btn_red;
static lv_obj_t *labelSlider;

static void button_event_cb(lv_event_t *e) {}

static void slider_event_cb(lv_event_t *e) {
  lv_obj_t *slider = lv_event_get_target(e);

  /*Refresh the text*/
  lv_label_set_text_fmt(labelSlider, "%" LV_PRId32,
                        lv_slider_get_value(slider));
  lv_obj_align_to(labelSlider, slider, LV_ALIGN_OUT_TOP_MID, 0,
                  -15); /*Align top of the slider*/
}

static lv_color_t darken(const lv_color_filter_dsc_t *dsc, lv_color_t color,
                         lv_opa_t opa) {
  LV_UNUSED(dsc);
  return lv_color_darken(color, opa);
}

static void style_init(void) {
  /*Create a simple button style*/
  lv_style_init(&style_btn);
  lv_style_set_radius(&style_btn, 10);
  lv_style_set_bg_opa(&style_btn, LV_OPA_COVER);
  lv_style_set_bg_color(&style_btn, lv_palette_lighten(LV_PALETTE_GREY, 3));
  lv_style_set_bg_grad_color(&style_btn, lv_palette_main(LV_PALETTE_GREY));
  lv_style_set_bg_grad_dir(&style_btn, LV_GRAD_DIR_VER);

  lv_style_set_border_color(&style_btn, lv_color_black());
  lv_style_set_border_opa(&style_btn, LV_OPA_20);
  lv_style_set_border_width(&style_btn, 2);

  lv_style_set_text_color(&style_btn, lv_color_black());

  /*Create a style for the pressed state.
   *Use a color filter to simply modify all colors in this state*/
  static lv_color_filter_dsc_t color_filter;
  lv_color_filter_dsc_init(&color_filter, darken);
  lv_style_init(&style_btn_pressed);
  lv_style_set_color_filter_dsc(&style_btn_pressed, &color_filter);
  lv_style_set_color_filter_opa(&style_btn_pressed, LV_OPA_20);

  /*Create a red style. Change only some colors.*/
  lv_style_init(&style_btn_red);
  lv_style_set_bg_color(&style_btn_red, lv_palette_main(LV_PALETTE_RED));
  lv_style_set_bg_grad_color(&style_btn_red,
                             lv_palette_lighten(LV_PALETTE_RED, 3));
}

void main_screen() {
  lv_obj_t *ui_Screen1 = lv_obj_create(NULL);
  lv_obj_set_style_bg_color(ui_Screen1, lv_color_hex(0x000000), LV_PART_MAIN);
  /*Initialize the style*/
  style_init();

  /*Create a button and use the new styles*/
  lv_obj_t *btn = lv_btn_create(ui_Screen1);
  /* Remove the styles coming from the theme
   * Note that size and position are also stored as style properties
   * so lv_obj_remove_style_all will remove the set size and position too */
  lv_obj_remove_style_all(btn);
  lv_obj_set_pos(btn, 10, 10);
  lv_obj_set_size(btn, 120, 50);
  lv_obj_add_style(btn, &style_btn, 0);
  lv_obj_add_style(btn, &style_btn_pressed, LV_STATE_PRESSED);

  /*Add a label to the button*/
  lv_obj_t *label = lv_label_create(btn);
  lv_label_set_text(label, "Button");
  lv_obj_center(label);
  lv_obj_add_event_cb(btn, button_event_cb, LV_EVENT_CLICKED, NULL);

  /*Create another button and use the red style too*/
  lv_obj_t *btn2 = lv_btn_create(ui_Screen1);
  lv_obj_remove_style_all(btn2); /*Remove the styles coming from the theme*/
  lv_obj_set_pos(btn2, 10, 80);
  lv_obj_set_size(btn2, 120, 50);
  lv_obj_add_style(btn2, &style_btn, 0);
  lv_obj_add_style(btn2, &style_btn_red, 0);
  lv_obj_add_style(btn2, &style_btn_pressed, LV_STATE_PRESSED);
  lv_obj_set_style_radius(btn2, LV_RADIUS_CIRCLE, 0); /*Add a local style too*/

  label = lv_label_create(btn2);
  lv_label_set_text(label, "Button 2");
  lv_obj_center(label);
  lv_obj_align_to(btn2, btn, LV_ALIGN_LEFT_MID, 140, 0);

  lv_obj_t *slider = lv_slider_create(ui_Screen1);
  lv_obj_set_width(slider, 200); /*Set the width*/

  lv_obj_center(slider);
  lv_obj_add_event_cb(slider, slider_event_cb, LV_EVENT_VALUE_CHANGED,
                      NULL); /*Assign an event function*/

  /*Create a label above the slider*/
  labelSlider = lv_label_create(ui_Screen1);
  lv_label_set_text(labelSlider, "0");
  lv_obj_align_to(labelSlider, slider, LV_ALIGN_OUT_TOP_MID, 0,
                  -15); /*Align top of the slider*/

  lv_obj_t *btnTheme;
  lv_obj_t *labelTheme;

  btnTheme = lv_btn_create(ui_Screen1);
  labelTheme = lv_label_create(btnTheme);
  lv_label_set_text(labelTheme, "Original theme");
  lv_obj_center(labelTheme);

  lv_obj_align_to(btnTheme, slider, LV_ALIGN_OUT_BOTTOM_MID, 0,
                  15); /*Align top of the slider*/
  lv_scr_load(ui_Screen1);
  // lv_scr_load_anim(ui_Screen1, LV_SCR_LOAD_ANIM_FADE_IN, 2000, 100, true);
}
void splash_screen();

void splash_in_timer_cb(lv_timer_t *timer) {
  if (boot_timer) {
    lv_timer_del(boot_timer);
    boot_timer = NULL;
  }
  splash_screen();
}

void main_screen_timer_cb(lv_timer_t *timer) {
  if (boot_timer) {
    lv_timer_del(boot_timer);
    boot_timer = NULL;
  }
  main_screen();
}

void splash_out_timer_cb(lv_timer_t *timer) {
  if (boot_timer) {
    lv_timer_del(boot_timer);
    boot_timer = NULL;
  }
  lv_obj_t *ui_Screen = lv_obj_create(NULL);
  lv_obj_set_style_bg_color(ui_Screen, lv_color_hex(0x000000), LV_PART_MAIN);
  lv_scr_load_anim(ui_Screen, LV_SCR_LOAD_ANIM_FADE_IN, 2000, 100, true);
  boot_timer = lv_timer_create(main_screen_timer_cb, 2000, NULL);
}

void boot_screen() {
  lv_obj_set_style_bg_color(lv_scr_act(), lv_color_hex(0x000000), LV_PART_MAIN);
  boot_timer = lv_timer_create(splash_in_timer_cb, 100, NULL);
}

void splash_screen() {
  lv_obj_t *ui_Screen = lv_obj_create(NULL);
  lv_obj_set_style_bg_color(ui_Screen, lv_color_hex(0x000000), LV_PART_MAIN);
  lv_obj_t *logo = lv_img_create(ui_Screen);
  lv_obj_t *label = lv_label_create(ui_Screen);
  lv_label_set_text(label, ESP3D_TFT_VERSION);
  static lv_style_t style_text;
  lv_style_init(&style_text);
  lv_style_set_text_opa(&style_text, LV_OPA_COVER);
  lv_style_set_text_color(&style_text, lv_color_hex(0xFFFFFF));
  lv_obj_add_style(label, &style_text, LV_PART_MAIN);
  // lv_style_set_text_font(&style_text, &lv_font_montserrat_12);
  lv_obj_center(logo);
  lv_obj_align_to(label, logo, LV_ALIGN_OUT_BOTTOM_MID, 0, 120);
  lv_img_set_src(logo, &logo_800_480_BW);
  lv_scr_load_anim(ui_Screen, LV_SCR_LOAD_ANIM_FADE_IN, 1000, 0, true);
  boot_timer = lv_timer_create(splash_out_timer_cb, 1000, NULL);
}
void create_application(void) { boot_screen(); }
