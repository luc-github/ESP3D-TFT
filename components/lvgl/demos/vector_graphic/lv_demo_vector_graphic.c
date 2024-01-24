/**
 * @file lv_demo_vector_graphic.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "lv_demo_vector_graphic.h"

#if LV_USE_DEMO_VECTOR_GRAPHIC

/*********************
 *      DEFINES
 *********************/
#define WIDTH 640
#define HEIGHT 480

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void draw_pattern(lv_vector_dsc_t * ctx, lv_vector_path_t * path)
{
    lv_vector_path_clear(path);
    lv_vector_dsc_identity(ctx);

    lv_fpoint_t pts[] = {{200, 200}, {300, 200}, {300, 300}, {200, 300}};
    lv_vector_path_move_to(path, &pts[0]);
    lv_vector_path_line_to(path, &pts[1]);
    lv_vector_path_quad_to(path, &pts[2], &pts[3]);
    lv_vector_path_close(path);

    lv_draw_image_dsc_t img_dsc;
    lv_draw_image_dsc_init(&img_dsc);

    LV_IMAGE_DECLARE(img_demo_vector_avatar);
    img_dsc.header = img_demo_vector_avatar.header;
    img_dsc.src = &img_demo_vector_avatar;

    lv_vector_dsc_set_fill_image(ctx, &img_dsc);
    lv_vector_dsc_translate(ctx, 250, 250);
    lv_vector_dsc_rotate(ctx, 25);
    lv_vector_dsc_translate(ctx, -250, -250);
    lv_vector_dsc_add_path(ctx, path); // draw a path
}

static void draw_gradient(lv_vector_dsc_t * ctx, lv_vector_path_t * path)
{
    lv_vector_path_clear(path);
    lv_vector_dsc_identity(ctx);

    lv_fpoint_t pts[] = {{400, 200}, {600, 200}, {400, 400}};
    lv_vector_path_move_to(path, &pts[0]);
    lv_vector_path_quad_to(path, &pts[1], &pts[2]);
    lv_vector_path_close(path);

    lv_grad_dsc_t grad;
    grad.dir = LV_GRAD_DIR_HOR;
    grad.stops_count = 2;
    grad.stops[0].color = lv_color_hex(0xff0000);
    grad.stops[0].opa = LV_OPA_COVER;
    grad.stops[0].frac = 0;
    grad.stops[1].color = lv_color_hex(0x00ff00);
    grad.stops[1].opa = LV_OPA_COVER;
    grad.stops[1].frac = 255;
    // grad.stops[2].color = lv_color_hex(0x0000ff);
    // grad.stops[2].opa = LV_OPA_COVER;
    // grad.stops[2].frac = 255;

    lv_matrix_t mt;
    lv_matrix_identity(&mt);
    lv_matrix_rotate(&mt, 30);
    lv_vector_dsc_set_fill_transform(ctx, &mt);

    lv_vector_dsc_set_fill_linear_gradient(ctx, &grad, LV_VECTOR_GRADIENT_SPREAD_PAD);
    lv_vector_dsc_add_path(ctx, path); // draw a path
}

static void draw_radial_gradient(lv_vector_dsc_t * ctx, lv_vector_path_t * path)
{
    lv_vector_path_clear(path);
    lv_vector_dsc_identity(ctx);

    lv_fpoint_t pts[] = {{400, 50}, {500, 50}, {500, 200}, {400, 200}};
    lv_vector_path_move_to(path, &pts[0]);
    lv_vector_path_line_to(path, &pts[1]);
    lv_vector_path_line_to(path, &pts[2]);
    lv_vector_path_line_to(path, &pts[3]);
    lv_vector_path_close(path);

    lv_grad_dsc_t grad;
    grad.dir = LV_GRAD_DIR_HOR;
    grad.stops_count = 2;
    grad.stops[0].color = lv_color_hex(0xff0000);
    grad.stops[0].opa = LV_OPA_COVER;
    grad.stops[0].frac = 0;
    grad.stops[1].color = lv_color_hex(0x0000ff);
    grad.stops[1].opa = LV_OPA_COVER;
    grad.stops[1].frac = 255;
    // grad.stops[2].color = lv_color_hex(0x0000ff);
    // grad.stops[2].opa = LV_OPA_COVER;
    // grad.stops[2].frac = 255;

    lv_vector_dsc_set_fill_radial_gradient(ctx, &grad, 50, 50, 20, LV_VECTOR_GRADIENT_SPREAD_REFLECT);
    lv_vector_dsc_add_path(ctx, path); // draw a path
}

static void draw_shapes(lv_vector_dsc_t * ctx, lv_vector_path_t * path)
{
    lv_vector_path_clear(path);
    lv_vector_dsc_identity(ctx);

    lv_fpoint_t pts[] = {{50, 50}, {200, 200}, {50, 200}};
    lv_vector_path_move_to(path, &pts[0]);
    lv_vector_path_line_to(path, &pts[1]);
    lv_vector_path_line_to(path, &pts[2]);
    lv_vector_path_close(path);
    lv_vector_dsc_set_fill_color(ctx, lv_color_make(0xFF, 0x00, 0x00));
    lv_vector_dsc_scale(ctx, 0.5, 0.5);
    lv_vector_dsc_add_path(ctx, path); // draw a path

    lv_vector_path_clear(path);
    lv_vector_dsc_identity(ctx);

    lv_area_t rect = {300, 300, 400, 400};
    lv_vector_path_append_rect(path, &rect, 50, 60);
    lv_vector_dsc_set_fill_color(ctx, lv_color_make(0x00, 0x80, 0xff));
    lv_vector_dsc_skew(ctx, 5, 0);
    lv_vector_dsc_add_path(ctx, path); // draw a path

    lv_vector_path_clear(path);
    lv_vector_dsc_identity(ctx);

    lv_area_t rect2 = {100, 300, 200, 400};
    lv_vector_path_append_rect(path, &rect2, 10, 10);
    lv_vector_dsc_set_fill_color(ctx, lv_color_make(0x80, 0x00, 0x80));

    lv_vector_path_t * path2 = lv_vector_path_create(LV_VECTOR_PATH_QUALITY_MEDIUM);
    lv_fpoint_t p = {50, 420};
    lv_vector_path_append_circle(path2, &p, 50, 30);
    lv_vector_path_append_path(path, path2);

    lv_vector_dsc_add_path(ctx, path); // draw a path

    lv_vector_path_delete(path2);
}

static void draw_lines(lv_vector_dsc_t * ctx, lv_vector_path_t * path)
{
    lv_vector_path_clear(path);
    lv_vector_dsc_identity(ctx);

    lv_fpoint_t pts[] = {{50, 50}, {200, 200}, {250, 300}, {350, 150}};

    lv_vector_path_move_to(path, &pts[0]);
    lv_vector_path_cubic_to(path, &pts[1], &pts[2], &pts[3]);

    lv_vector_dsc_set_stroke_color(ctx, lv_color_make(0x00, 0xff, 0x00));
    lv_vector_dsc_set_stroke_opa(ctx, LV_OPA_COVER);
    lv_vector_dsc_set_fill_opa(ctx, LV_OPA_0);
    lv_vector_dsc_set_stroke_width(ctx, 8.0f);

    float dashes[] = {10, 15, 20, 12};
    lv_vector_dsc_set_stroke_dash(ctx, dashes, 4);

    lv_vector_dsc_add_path(ctx, path); // draw a path

    lv_vector_dsc_set_stroke_opa(ctx, LV_OPA_0);
    lv_vector_dsc_set_fill_opa(ctx, LV_OPA_COVER);
}

static void draw_blend(lv_vector_dsc_t * ctx, lv_vector_path_t * path)
{
    lv_vector_path_clear(path);
    lv_vector_dsc_identity(ctx);

    lv_fpoint_t pts[] = {{200, 200}, {400, 200}, {450, 350}, {350, 150}};

    lv_vector_path_move_to(path, &pts[0]);
    lv_vector_path_cubic_to(path, &pts[1], &pts[2], &pts[3]);
    lv_vector_path_close(path);
    lv_vector_dsc_set_fill_color(ctx, lv_color_make(0xFF, 0x00, 0xFF));
    lv_vector_dsc_set_blend_mode(ctx, LV_VECTOR_BLEND_SCREEN);

    lv_vector_dsc_add_path(ctx, path); // draw a path
}

static void draw_vector(lv_layer_t * layer)
{
    lv_vector_dsc_t * ctx = lv_vector_dsc_create(layer);

    lv_area_t rect = {0, 100, 300, 300};
    lv_vector_dsc_set_fill_color(ctx, lv_color_lighten(lv_color_black(), 50));
    lv_vector_clear_area(ctx, &rect); // clear screen

    lv_vector_path_t * path = lv_vector_path_create(LV_VECTOR_PATH_QUALITY_MEDIUM);

    draw_shapes(ctx, path);
    draw_lines(ctx, path);
    draw_pattern(ctx, path);
    draw_radial_gradient(ctx, path);
    draw_gradient(ctx, path);
    draw_blend(ctx, path);

    lv_draw_vector(ctx); // submit draw
    lv_vector_path_delete(path);
    lv_vector_dsc_delete(ctx);
}

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void lv_demo_vector_graphic(void)
{
    lv_draw_buf_t * draw_buf = lv_draw_buf_create(WIDTH, HEIGHT, LV_COLOR_FORMAT_ARGB8888, LV_STRIDE_AUTO);
    lv_obj_t * canvas = lv_canvas_create(lv_scr_act());
    lv_canvas_set_draw_buf(canvas, draw_buf);

    lv_layer_t layer;
    lv_canvas_init_layer(canvas, &layer);
    draw_vector(&layer);

    lv_canvas_finish_layer(canvas, &layer);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/
#else

void lv_demo_vector_graphic(void)
{
    /*fallback for online examples*/
    lv_obj_t * label = lv_label_create(lv_screen_active());
    lv_label_set_text(label, "Vector graphics is not enabled");
    lv_obj_center(label);
}

#endif
