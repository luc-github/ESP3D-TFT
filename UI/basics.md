## Change screen background style
https://docs.lvgl.io/master/overview/style.html
https://docs.lvgl.io/master/overview/style-props.html

2 solutions:
1 - Use active screen and change the style
```
lv_obj_t *ui_Screen1 = lv_scr_act(); 
lv_obj_set_style_bg_color(ui_Screen1, lv_color_hex(0x000000), LV_PART_MAIN);
```
optionaly change opacity, it is good practice to do it to ensure changes are well applied:
```
lv_obj_set_style_bg_opa(ui_Screen1, LV_OPA_COVER,LV_PART_MAIN);
```

2 - Create a screen from scratch and set it as active one
```
lv_obj_t *ui_Screen1 = lv_obj_create(NULL);
lv_obj_clear_flag(ui_Screen1, LV_OBJ_FLAG_SCROLLABLE);  /// Flags
lv_obj_set_flex_flow(ui_Screen1, LV_FLEX_FLOW_ROW);
lv_obj_set_flex_align(ui_Screen1, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
lv_obj_set_style_bg_color(ui_Screen1, lv_color_hex(0x000000), LV_PART_MAIN);
lv_obj_set_style_bg_opa(ui_Screen1, LV_OPA_COVER, LV_PART_MAIN);
lv_disp_load_scr(ui_Screen1);
```

Note: object may have default style depemdeing on previous object created, when adding style you may need to clear existing object style to be sure extra style not defined in new style is not present, use:
`lv_obj_remove_style_all(object);`

## Screen
https://docs.lvgl.io/master/overview/object.html#screens
 * To get the currently active screen, use `lv_scr_act()`
 * To load a new screen, use `lv_scr_load(scr1)` 
 * To load a new screen with some animation, use `lv_scr_load_anim(newscr, transition_type, time, delay, auto_del)`   
 where transition type can be:  
    - `LV_SCR_LOAD_ANIM_NONE` Switch immediately after `delay` in milliseconds
    - `LV_SCR_LOAD_ANIM_OVER_LEFT/RIGHT/TOP/BOTTOM` Move the new screen over the current towards the given direction  
    - `LV_SCR_LOAD_ANIM_OUT_LEFT/RIGHT/TOP/BOTTOM` Move out the old screen over the current towards the given direction  
    - `LV_SCR_LOAD_ANIM_MOVE_LEFT/RIGHT/TOP/BOTTOM` Move both the current and new screens towards the given direction  
    - `LV_SCR_LOAD_ANIM_FADE_IN/OUT` Fade the new screen over the old screen, or vice versa   

    The parameter `time` is the animation duration   
    Setting `auto_del` to true will automatically delete the old screen when the animation is finished
* Make screen not scrollable
Because any object become automaticaly scrollable if anything is displayed out of area, need to clear the flag using:   
`lv_obj_clear_flag(new_screen, LV_OBJ_FLAG_SCROLLABLE);`

## Alignement 
<img src="https://raw.githubusercontent.com/luc-github/ESP3D-TFT/main/UI/align.png"/>

### Size
Constantes about resolution: `LV_HOR_RES` / `LV_VER_RES`
Object size in pixels: `lv_obj_get_height(object);` /  `lv_obj_get_width(object);` 
Note: be sure style is already applied to get correct values.

## Label
https://docs.lvgl.io/master/widgets/label.html
The label need a style to define the properties:
``` 
static lv_style_t style_text;
lv_style_init(&style_text);
lv_style_set_text_opa(&style_text, LV_OPA_COVER);
lv_style_set_text_color(&style_text, lv_color_hex(0xFFFFFF));
lv_style_set_text_font(&style_text, &lv_font_montserrat_12);
```
Create the label and attach to screen :
```
lv_obj_t *label1 = lv_label_create(ui_Screen1);
lv_obj_add_style(label1, &style_text, LV_PART_MAIN);
```
Add the text to the label:
```
lv_label_set_text(label1, "Hello world!");
```
Here the size is automaticaly defined to use the minimum size to display all the text.

If the size is limited in width:
```
lv_obj_set_width(label1, 150);
```
Add circular scrolling text feature:
```
lv_label_set_long_mode(label1,LV_LABEL_LONG_SCROLL_CIRCULAR); 
```

On 800X600 - 4.3' screen a font of 24~28 is giving good readibility ~47 chars on a row (e.g:lv_font_montserrat_24)
On 320x240 - 2.8' screen a font of 12~14 is giving good readibility ~38 chars on a row (e.g:lv_font_montserrat_14)

For Chinese only lv_font_simsun_16_cjk  is available - need to add more size according targeted screen
Symbols are actually characteres and may need to be customized by generating a new merged font file. (https://lvgl.io/tools/fontconverter)

## Button
Note: a button widget is not text / image but the button only, to add text/image need to add child widget to the button.
https://docs.lvgl.io/master/widgets/btn.html

```
lv_obj_t * btn1 = lv_btn_create(lv_scr_act());
```

Add label and center text in button:
```
lv_obj_t * label = lv_label_create(btn1);
lv_label_set_text(label, "Button");
lv_obj_center(label);
```
State UI can be modified using style


## Event
https://docs.lvgl.io/master/overview/event.html
Add a callback when event is raised from widget
from a button:

```
lv_obj_t * btn1 = lv_btn_create(lv_scr_act());
lv_obj_t * label = lv_label_create(btn1);
lv_label_set_text(label, "Button");
lv_obj_center(label);
```
Add callback to widget:
```
lv_obj_add_event_cb(btn1, event_handler, LV_EVENT_ALL, NULL);
```

The even can be LV_EVENT_ALL and filtered in callback, or several callbacks can be added according specific events.

```
lv_obj_add_event_cb(btn1, event_handler, LV_EVENT_CLICKED, NULL);
```

The callback can be:
```
static void event_handler(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if(code == LV_EVENT_PRESSED) {
      lv_label_set_text(label, "Pressed");
    } else if(code == LV_EVENT_RELEASED) {
      lv_label_set_text(label, "Button");
    } 
    
}
```
There are several events, not all related to user interaction:
### Input device events
- `LV_EVENT_PRESSED`      An object has been pressed
- `LV_EVENT_PRESSING`     An object is being pressed (called continuously while pressing)
- `LV_EVENT_PRESS_LOST`   An object is still being pressed but slid cursor/finger off of the object
- `LV_EVENT_SHORT_CLICKED`    An object was pressed for a short period of time, then released. Not called if scrolled.
- `LV_EVENT_LONG_PRESSED` An object has been pressed for at least the `long_press_time` specified in the input device driver.  Not called if scrolled.
- `LV_EVENT_LONG_PRESSED_REPEAT`  Called after `long_press_time` in every `long_press_repeat_time` ms.  Not called if scrolled.
- `LV_EVENT_CLICKED`      Called on release if an object did not scroll (regardless of long press)
- `LV_EVENT_RELEASED`     Called in every case when an object has been released
- `LV_EVENT_SCROLL_BEGIN` Scrolling begins. The event parameter is `NULL` or an `lv_anim_t *` with a scroll animation descriptor that can be modified if required.
- `LV_EVENT_SCROLL_THROW_BEGIN` Sent once when the object is released while scrolling but the "momentum" still keeps the content scrolling.
- `LV_EVENT_SCROLL_END`   Scrolling ends.
- `LV_EVENT_SCROLL`       An object was scrolled
- `LV_EVENT_GESTURE`      A gesture is detected. Get the gesture with `lv_indev_get_gesture_dir(lv_indev_get_act());`
- `LV_EVENT_KEY`          A key is sent to an object. Get the key with `lv_indev_get_key(lv_indev_get_act());`
- `LV_EVENT_FOCUSED`      An object is focused
- `LV_EVENT_DEFOCUSED`    An object is unfocused
- `LV_EVENT_LEAVE`        An object is unfocused but still selected
- `LV_EVENT_HIT_TEST`     Perform advanced hit-testing. Use `lv_hit_test_info_t * a = lv_event_get_hit_test_info(e)` and check if `a->point` can click the object or not. If not set `a->res = false`


### Drawing events
- `LV_EVENT_COVER_CHECK` Check if an object fully covers an area. The event parameter is `lv_cover_check_info_t *`.
- `LV_EVENT_REFR_EXT_DRAW_SIZE`  Get the required extra draw area around an object (e.g. for a shadow). The event parameter is `lv_coord_t *` to store the size. Only overwrite it with a larger value.
- `LV_EVENT_DRAW_MAIN_BEGIN` Starting the main drawing phase.
- `LV_EVENT_DRAW_MAIN`   Perform the main drawing
- `LV_EVENT_DRAW_MAIN_END`   Finishing the main drawing phase
- `LV_EVENT_DRAW_POST_BEGIN` Starting the post draw phase (when all children are drawn)
- `LV_EVENT_DRAW_POST`   Perform the post draw phase (when all children are drawn)
- `LV_EVENT_DRAW_POST_END`   Finishing the post draw phase (when all children are drawn)
- `LV_EVENT_DRAW_PART_BEGIN` Starting to draw a part. The event parameter is `lv_obj_draw_dsc_t *`. Learn more [here](/overview/drawing).
- `LV_EVENT_DRAW_PART_END`   Finishing to draw a part. The event parameter is `lv_obj_draw_dsc_t *`. Learn more [here](/overview/drawing).

In `LV_EVENT_DRAW_...` events it's not allowed to adjust the widgets' properties. E.g. you can not call `lv_obj_set_width()`.
In other words only `get` functions can be called.

### Other events
- `LV_EVENT_DELETE`       Object is being deleted
- `LV_EVENT_CHILD_CHANGED`    Child was removed/added
- `LV_EVENT_CHILD_CREATED`    Child was created, always bubbles up to all parents
- `LV_EVENT_CHILD_DELETED`    Child was deleted, always bubbles up to all parents
- `LV_EVENT_SIZE_CHANGED`    Object coordinates/size have changed
- `LV_EVENT_STYLE_CHANGED`    Object's style has changed
- `LV_EVENT_BASE_DIR_CHANGED` The base dir has changed
- `LV_EVENT_GET_SELF_SIZE`    Get the internal size of a widget
- `LV_EVENT_SCREEN_UNLOAD_START` A screen unload started, fired immediately when lv_scr_load/lv_scr_load_anim is called
- `LV_EVENT_SCREEN_LOAD_START` A screen load started, fired when the screen change delay is expired
- `LV_EVENT_SCREEN_LOADED`    A screen was loaded, called when all animations are finished
- `LV_EVENT_SCREEN_UNLOADED`  A screen was unloaded, called when all animations are finished

### Special events
- `LV_EVENT_VALUE_CHANGED`    The object's value has changed (i.e. slider moved)
- `LV_EVENT_INSERT`       Text is being inserted into the object. The event data is `char *` being inserted.
- `LV_EVENT_REFRESH`      Notify the object to refresh something on it (for the user)
- `LV_EVENT_READY`        A process has finished
- `LV_EVENT_CANCEL`       A process has been canceled

To manually send events to an object, use `lv_event_send(obj, <EVENT_CODE> &some_data)`.

### Useful function for callback:
Get object target:
`lv_obj_t * ta = lv_event_get_target(e);`

Get user data provided in callback link:
`lv_obj_add_event_cb(btn1, event_handler, LV_EVENT_CLICKED, userdata);`

`lv_obj_t * ud = lv_event_get_user_data(e);`

## Keyboard
https://docs.lvgl.io/master/widgets/keyboard.html

## Selector
https://docs.lvgl.io/master/widgets/spinbox.html


## Roller
https://docs.lvgl.io/master/widgets/roller.html

## Switch
https://docs.lvgl.io/master/widgets/switch.html

## Slider
https://docs.lvgl.io/master/widgets/slider.html

## Message box
https://docs.lvgl.io/master/widgets/msgbox.html

## Dropdown
https://docs.lvgl.io/master/widgets/dropdown.html

## Spinner
https://docs.lvgl.io/master/widgets/spinner.html

## Layout 
Create a container (Object):
```
lv_obj_t * cont = lv_obj_create(lv_scr_act());
```

Define size / position:
```
lv_obj_set_size(cont, 300, 220);
lv_obj_center(cont);
```

Define style and apply to container:
```
static lv_style_t style;
lv_style_init(&style);
lv_style_set_flex_flow(&style, LV_FLEX_FLOW_ROW_WRAP);
lv_style_set_flex_main_place(&style, LV_FLEX_ALIGN_SPACE_EVENLY);
lv_style_set_layout(&style, LV_LAYOUT_FLEX);
lv_obj_add_style(cont, &style, 0);
```

Place childs inside:
```
uint32_t i;
for(i = 0; i < 8; i++) {
    lv_obj_t * obj = lv_obj_create(cont);
    lv_obj_set_size(obj, 70, LV_SIZE_CONTENT);
    lv_obj_add_flag(obj, LV_OBJ_FLAG_CHECKABLE);

    lv_obj_t * label = lv_label_create(obj);
    lv_label_set_text_fmt(label, "%"LV_PRIu32, i);
    lv_obj_center(label);
}
```

### Flexbox
https://docs.lvgl.io/master/layouts/flex.html

### Grid
https://docs.lvgl.io/master/layouts/grid.html

## Timer
https://docs.lvgl.io/master/overview/timer.html




## Build a font
Check : https://github.com/lvgl/lvgl/tree/master/scripts/built_in_font
### Manually 
Opts: --no-compress --no-prefilter --bpp 4 --size 48 --font Montserrat-Medium.ttf -r 0x20-0x7F,0xB0,0x2022 --font FontAwesome5-Solid+Brands+Regular.woff -r 61441,61448,61451,61452,61452,61453,61457,61459,61461,61465,61468,61473,61478,61479,61480,61502,61507,61512,61515,61516,61517,61521,61522,61523,61524,61543,61544,61550,61552,61553,61556,61559,61560,61561,61563,61587,61589,61636,61637,61639,61641,61664,61671,61674,61683,61724,61732,61787,61931,62016,62017,62018,62019,62020,62087,62099,62212,62189,62810,63426,63650 --format lvgl -o lv_font_montserrat_48.c --force-fast-kern-format

### Online 
1 - Create font 
https://www.youtube.com/watch?v=fTTq6of99tM
https://www.youtube.com/watch?v=MHwLLJC9wUw

1 - Use https://lvgl.io/tools/fontconverter
2 - Download a WOFF or TTF 
https://fontawesome.com/ or github
Use 6.4.0 free

## Image
Adding image is a mitigattion between image size / image quality / decoding / speed 
### Using predecoded image
1 - Use Online/Offline decoder
  https://lvgl.io/tools/imageconverter
  
  will generate a C file, the size described in struc is the size added to your FW
  ```
  const lv_img_dsc_t logo_800_480_BW = {
  .header.cf = LV_IMG_CF_INDEXED_4BIT,
  .header.always_zero = 0,
  .header.reserved = 0,
  .header.w = 603,
  .header.h = 450,
  .data_size = 135964,
  .data = logo_800_480_BW_map,
};
  ```
  Here ~+135KB

  In the file using the image add:
  `LV_IMG_DECLARE(logo_800_480_BW);`
  then  use the image:
  ```
  lv_obj_t *logo = lv_img_create(lv_scr_act());
  lv_img_set_src(logo, &logo_800_480_BW);
  ```