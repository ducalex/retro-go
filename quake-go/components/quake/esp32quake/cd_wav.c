#include "quakedef.h"
#include "music_esp32.h"
#include "music_pcm.h"
#include "rg_system.h"

void CDAudio_Play(byte track, qboolean looping)
{
    if (track < 2) {
        Music_Stop();
        return;
    }
    char path[RG_PATH_MAX + 1];
    const rg_app_t *app = rg_system_get_app();
    if (!music_track_path(path, sizeof(path), RG_BASE_PATH_ROMS,
                          app ? app->romPath : NULL, (unsigned)track)) {
        Music_Stop();
        Con_Printf("Music: invalid ROM path\n");
        return;
    }
    Music_SetVolume(bgmvolume.value);
    Music_Play(path, track, looping != 0);
}

void CDAudio_Stop(void) { Music_Stop(); }
void CDAudio_Pause(void) { Music_Pause(true); }
void CDAudio_Resume(void) { Music_Pause(false); }
void CDAudio_Update(void) { Music_SetVolume(bgmvolume.value); }
void CDAudio_Shutdown(void) { Music_Shutdown(); }

static void CD_f(void)
{
    if (Cmd_Argc() == 3 && (!strcmp(Cmd_Argv(1), "play") || !strcmp(Cmd_Argv(1), "loop"))) {
        int track = Q_atoi(Cmd_Argv(2));
        if (track >= 2 && track <= 255)
            CDAudio_Play((byte)track, !strcmp(Cmd_Argv(1), "loop"));
    } else if (Cmd_Argc() == 2 && !strcmp(Cmd_Argv(1), "stop")) {
        CDAudio_Stop();
    } else if (Cmd_Argc() == 2 && !strcmp(Cmd_Argv(1), "pause")) {
        CDAudio_Pause();
    } else if (Cmd_Argc() == 2 && !strcmp(Cmd_Argv(1), "resume")) {
        CDAudio_Resume();
    } else {
        Con_Printf("cd play/loop <track>, cd stop/pause/resume\n");
    }
}

int CDAudio_Init(void)
{
    Cmd_AddCommand("cd", CD_f);
    return Music_Init() ? 0 : -1;
}
