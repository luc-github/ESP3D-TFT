#include "../../lv_examples.h"
#if LV_USE_BUTTONMATRIX && LV_BUILD_EXAMPLES

static void event_cb(lv_event_t * e)
{
    lv_obj_t * obj = lv_event_get_target(e);
    lv_draw_task_t * draw_task = lv_event_get_draw_task(e);
    lv_draw_dsc_base_t * base_dsc = draw_task->draw_dsc;
    /*When the button matrix draws the buttons...*/
    if(base_dsc->part == LV_PART_ITEMS) {
        bool pressed = false;
        if(lv_buttonmatrix_get_selected_button(obj) == base_dsc->id1 && lv_obj_has_state(obj, LV_STATE_PRESSED)) {
            pressed = true;
        }

        /*Change the draw descriptor of the 2nd button*/
        if(base_dsc->id1 == 1) {
            if(draw_task->type == LV_DRAW_TASK_TYPE_FILL) {
                lv_draw_rect_dsc_t * rect_draw_dsc = draw_task->draw_dsc;
                rect_draw_dsc->radius = 0;
                if(pressed) rect_draw_dsc->bg_color = lv_palette_darken(LV_PALETTE_BLUE, 3);
                else rect_draw_dsc->bg_color = lv_palette_main(LV_PALETTE_BLUE);
                rect_draw_dsc->shadow_width = 6;
                rect_draw_dsc->shadow_offset_x = 3;
                rect_draw_dsc->shadow_offset_y = 3;
            }
            if(draw_task->type == LV_DRAW_TASK_TYPE_LABEL) {
                lv_draw_label_dsc_t * label_draw_dsc = draw_task->draw_dsc;
                label_draw_dsc->color = lv_color_white();
            }

        }
        /*Change the draw descriptor of the 3rd button*/
        else if(base_dsc->id1 == 2) {
            if(draw_task->type == LV_DRAW_TASK_TYPE_FILL) {
                lv_draw_rect_dsc_t * rect_draw_dsc = draw_task->draw_dsc;
                rect_draw_dsc->radius = LV_RADIUS_CIRCLE;
                if(pressed) rect_draw_dsc->bg_color = lv_palette_darken(LV_PALETTE_RED, 3);
                else rect_draw_dsc->bg_color = lv_palette_main(LV_PALETTE_RED);
            }
        }
        else if(base_dsc->id1 == 3) {
            if(draw_task->type == LV_DRAW_TASK_TYPE_LABEL) {
                lv_draw_label_dsc_t * label_draw_dsc = draw_task->draw_dsc;
                label_draw_dsc->opa = 0;
            }
            if(draw_task->type == LV_DRAW_TASK_TYPE_FILL) {
                LV_IMAGE_DECLARE(img_star);
                lv_image_header_t header;
                lv_result_t res = lv_image_decoder_get_info(&img_star, &header);
                if(res != LV_RESULT_OK) return;

                lv_area_t a;
                a.x1 = 0;
                a.x2 = header.w - 1;
                a.y1 = 0;
                a.y2 = header.h - 1;
                lv_area_align(&draw_task->area, &a, LV_ALIGN_CENTER, 0, 0);

                lv_draw_image_dsc_t img_draw_dsc;
                lv_draw_image_dsc_init(&img_draw_dsc);
                img_draw_dsc.src = &img_star;
                img_draw_dsc.recolor = lv_color_black();
                if(pressed) img_draw_dsc.recolor_opa = LV_OPA_30;

                lv_draw_image(base_dsc->layer, &img_draw_dsc, &a);

            }
        }
    }
}

/**
 * Add custom drawer to the button matrix to customize buttons one by one
 */
void lv_example_buttonmatrix_2(void)
{
    lv_obj_t * btnm = lv_buttonmatrix_create(lv_screen_active());
    lv_obj_add_event_cb(btnm, event_cb, LV_EVENT_DRAW_TASK_ADDED, NULL);
    lv_obj_add_flag(btnm, LV_OBJ_FLAG_SEND_DRAW_TASK_EVENTS);
    lv_obj_center(btnm);
}

#endif
