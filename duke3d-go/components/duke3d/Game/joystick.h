#ifndef __joystick_h
#define __joystick_h
#ifdef __cplusplus
extern "C" {
#endif

    void   JOYSTICK_UpdateHats( void );
    void _joystick_init(void);
    void _joystick_deinit(void);
    int _joystick_update(void);
    int _joystick_axis(int axis);
    int _joystick_hat(int hat);
    int _joystick_button(int button);
    
#ifdef __cplusplus
};

#endif
#endif /* __joystick_h */
