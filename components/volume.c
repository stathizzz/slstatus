/* See LICENSE file for copyright and license details. */
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "../util.h"

#if defined(__OpenBSD__)
#include <sys/audioio.h>


const char *
vol_perc(const char *card)
{
	static int cls = -1;
	mixer_devinfo_t mdi;
	mixer_ctrl_t mc;
	int afd = -1, m = -1, v = -1;

	if ((afd = open(card, O_RDONLY)) < 0) {
		warn("open '%s':", card);
		return NULL;
	}

	for (mdi.index = 0; cls == -1; mdi.index++) {
		if (ioctl(afd, AUDIO_MIXER_DEVINFO, &mdi) < 0) {
			warn("ioctl 'AUDIO_MIXER_DEVINFO':");
			close(afd);
			return NULL;
		}
		if (mdi.type == AUDIO_MIXER_CLASS &&
				!strncmp(mdi.label.name,
					AudioCoutputs,
					MAX_AUDIO_DEV_LEN))
			cls = mdi.index;
	}
	for (mdi.index = 0; v == -1 || m == -1; mdi.index++) {
		if (ioctl(afd, AUDIO_MIXER_DEVINFO, &mdi) < 0) {
			warn("ioctl 'AUDIO_MIXER_DEVINFO':");
			close(afd);
			return NULL;
		}
		if (mdi.mixer_class == cls &&
				((mdi.type == AUDIO_MIXER_VALUE &&
				  !strncmp(mdi.label.name,
					  AudioNmaster,
					  MAX_AUDIO_DEV_LEN)) ||
				 (mdi.type == AUDIO_MIXER_ENUM &&
				  !strncmp(mdi.label.name,
					  AudioNmute,
					  MAX_AUDIO_DEV_LEN)))) {
			mc.dev = mdi.index, mc.type = mdi.type;
			if (ioctl(afd, AUDIO_MIXER_READ, &mc) < 0) {
				warn("ioctl 'AUDIO_MIXER_READ':");
				close(afd);
				return NULL;
			}
			if (mc.type == AUDIO_MIXER_VALUE)
				v = mc.un.value.num_channels == 1 ?
					mc.un.value.level[AUDIO_MIXER_LEVEL_MONO] :
					(mc.un.value.level[AUDIO_MIXER_LEVEL_LEFT] >
					 mc.un.value.level[AUDIO_MIXER_LEVEL_RIGHT] ?
					 mc.un.value.level[AUDIO_MIXER_LEVEL_LEFT] :
					 mc.un.value.level[AUDIO_MIXER_LEVEL_RIGHT]);
			else if (mc.type == AUDIO_MIXER_ENUM)
				m = mc.un.ord;
		}
	}

	close(afd);

	return bprintf("%d", m ? 0 : v * 100 / 255);
}
#else
#include <alsa/asoundlib.h>
#include <alsa/mixer.h>

static struct snd_mixer_selem_regopt smixer_options;

static int 
parse_simple_id(const char *str, snd_mixer_selem_id_t *sid)
{
	int c, size;
	char buf[128];
	char *ptr = buf;

	while (*str == ' ' || *str == '\t')
		str++;
	if (!(*str))
		return -EINVAL;
	size = 1;	/* for '\0' */
	if (*str != '"' && *str != '\'') {
		while (*str && *str != ',') {
			if (size < (int)sizeof(buf)) {
				*ptr++ = *str;
				size++;
			}
			str++;
		}
	} else {
		c = *str++;
		while (*str && *str != c) {
			if (size < (int)sizeof(buf)) {
				*ptr++ = *str;
				size++;
			}
			str++;
		}
		if (*str == c)
			str++;
	}
	if (*str == '\0') {
		snd_mixer_selem_id_set_index(sid, 0);
		*ptr = 0;
		goto _set;
	}
	if (*str != ',')
		return -EINVAL;
	*ptr = 0;	/* terminate the string */
	str++;

	if (!isdigit(*str))
		return -EINVAL;
	snd_mixer_selem_id_set_index(sid, atoi(str));
_set:
	snd_mixer_selem_id_set_name(sid, buf);
	return 0;
}


const char *
alsa_vol_perc(const char *card)
{
	int err = 0;
	long pvol, pmin = 0, pmax = 0;

	snd_mixer_t *mhandle = NULL;
	snd_mixer_elem_t *elem;
	snd_mixer_selem_id_t *sid;
	const char *scontrol_id = "Master";

	if ((err = snd_mixer_open(&mhandle, 0)) < 0) {
		warn("Mixer open error: %s", snd_strerror(err));
		return NULL;
	}
	if (err = snd_mixer_attach(mhandle, card) < 0) {
		warn("Mixer attach %s error: %s", card, snd_strerror(err));
		goto end;
	}
	if (err = snd_mixer_selem_register(mhandle, NULL, NULL) < 0) {
		warn("Mixer register error: %s", snd_strerror(err));
		goto end;
	}
	if (err = snd_mixer_load(mhandle) < 0) {
		warn("Mixer load %s error: %s", card, snd_strerror(err));
		goto end;
	}
	snd_mixer_selem_id_alloca(&sid);
	if (err = parse_simple_id(scontrol_id, sid)) {
		warn("Wrong scontrol identifier: %s\n", scontrol_id);
		goto end;
	}
	if (!(elem = snd_mixer_find_selem(mhandle, sid))) {
		warn("Unable to find simple control on '%s',%i\n", snd_mixer_selem_id_get_name(sid), snd_mixer_selem_id_get_index(sid));
		err = -1;
		goto end;
	}
	if (err = snd_mixer_selem_get_playback_volume_range(elem, &pmin, &pmax)) {
		warn("Unable to get volume range on '%s',%i\n", snd_mixer_selem_id_get_name(sid), snd_mixer_selem_id_get_index(sid));
		goto end;
	}
	if (err = snd_mixer_selem_get_playback_volume(elem, SND_MIXER_SCHN_MONO, &pvol)) {
		warn("Unable to get playback volume on '%s',%i\n", snd_mixer_selem_id_get_name(sid), snd_mixer_selem_id_get_index(sid));
		goto end;
	}
end:
	if (mhandle) snd_mixer_close(mhandle);
	if (err) {
		return NULL;
	}
	char *ico = "🔈";
	double val = (double)pvol / (double)(pmax - pmin);
	if (!pvol)  ico = "🔇";
	else if (val>0.7) ico = "🔊";
	else if (val>0.3) ico = "🔉";
	
	return bprintf("%s %.0f%%", ico, 100 * val);
}
#endif
