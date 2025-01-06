#ifndef UI_EVENTS_H
#define UI_EVENTS_H

#ifdef __cplusplus

extern "C"
{
#endif

    typedef struct
    {
        int index;
    } TotpValueChangeEvent;

    void on_totp_screen_gesture(lv_event_t *e);
    void on_screen_keyboard_button_clicked(lv_event_t *e);
    void on_pin_screen_form_submit(lv_event_t *e);
    void on_key_creation_form_submit(lv_event_t *e);

#ifdef __cplusplus
}
#endif

#endif // UI_EVENTS_H
