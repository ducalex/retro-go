typedef struct
{
    const char *name;
    const char *TZ;
} timezone_t;

static const timezone_t timezones[] = {
    {"GMT-11:00", "UTC+11"},
    {"GMT-10:00", "UTC+10"},
    {"GMT-09:00", "UTC+9"},
    {"GMT-08:00", "UTC+8"},
    {"GMT-07:00", "UTC+7"},
    {"GMT-06:00", "UTC+6"},
    {"GMT-05:00", "UTC+5"},
    {"GMT-04:00", "UTC+4"},
    {"GMT-03:00", "UTC+3"},
    {"GMT-02:00", "UTC+2"},
    {"GMT-01:00", "UTC+1"},
    {"GMT", "UTC+0"},
    {"GMT+01:00", "UTC-1"},
    {"GMT+02:00", "UTC-2"},
    {"GMT+03:00", "UTC-3"},
    {"GMT+04:00", "UTC-4"},
    {"GMT+05:00", "UTC-5"},
    {"GMT+06:00", "UTC-6"},
    {"GMT+07:00", "UTC-7"},
    {"GMT+08:00", "UTC-8"},
    {"GMT+09:00", "UTC-9"},
    {"GMT+10:00", "UTC-10"},
    {"GMT+11:00", "UTC-11"},
    {"GMT+12:00", "UTC-12"},
};

static unsigned int timezones_count = sizeof(timezones) / sizeof(timezone_t);
