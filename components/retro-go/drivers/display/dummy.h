static void lcd_init(void)
{
}

static void lcd_deinit(void)
{
}

static void lcd_set_backlight(float percent)
{
}

static inline uint16_t *lcd_get_buffer(size_t length)
{
    return (void *)0;
}

static inline void lcd_send_buffer(uint16_t *buffer, size_t length)
{
}

static void lcd_sync(void)
{
}

const rg_display_driver_t rg_display_driver_dummy = {
    .name = "dummy",
};
