/* See LICENSE file for copyright and license details. */

/* interval between updates (in ms) */
const unsigned int interval = 1000;
const unsigned int primary_statusbar_duration_before_change = 20000;
const unsigned int secondary_statusbar_duration = 15000;
/* text to show if no value can be retrieved */
static const char unknown_str[] = "n/a";

/* maximum output string length */
#define MAXLEN 2048

/*
 * function            description                     argument (example)
 *
 * battery_perc        battery percentage              battery name (BAT0)
 *                                                     NULL on OpenBSD/FreeBSD
 * battery_state       battery charging state          battery name (BAT0)
 *                                                     NULL on OpenBSD/FreeBSD
 * battery_remaining   battery remaining HH:MM         battery name (BAT0)
 *                                                     NULL on OpenBSD/FreeBSD
 * cpu_perc            cpu usage in percent            NULL
 * cpu_freq            cpu frequency in MHz            NULL
 * datetime            date and time                   format string (%F %T)
 * disk_free           free disk space in GB           mountpoint path (/)
 * disk_perc           disk usage in percent           mountpoint path (/)
 * disk_total          total disk space in GB          mountpoint path (/")
 * disk_used           used disk space in GB           mountpoint path (/)
 * entropy             available entropy               NULL
 * gid                 GID of current user             NULL
 * hostname            hostname                        NULL
 * ipv4                IPv4 address                    interface name (eth0)
 * ipv6                IPv6 address                    interface name (eth0)
 * kernel_release      `uname -r`                      NULL
 * keyboard_indicators caps/num lock indicators        format string (c?n?)
 *                                                     see keyboard_indicators.c
 * keymap              layout (variant) of current     NULL
 *                     keymap
 * load_avg            load average                    NULL
 * netspeed_rx         receive network speed           interface name (wlan0)
 * netspeed_tx         transfer network speed          interface name (wlan0)
 * num_files           number of files in a directory  path
 *                                                     (/home/foo/Inbox/cur)
 * ram_free            free memory in GB               NULL
 * ram_perc            memory usage in percent         NULL
 * ram_total           total memory size in GB         NULL
 * ram_used            used memory in GB               NULL
 * run_command         custom shell command            command (echo foo)
 * swap_free           free swap in GB                 NULL
 * swap_perc           swap usage in percent           NULL
 * swap_total          total swap size in GB           NULL
 * swap_used           used swap in GB                 NULL
 * temp                temperature in degree celsius   sensor file
 *                                                     (/sys/class/thermal/...)
 *                                                     NULL on OpenBSD
 *                                                     thermal zone on FreeBSD
 *                                                     (tz0, tz1, etc.)
 * uid                 UID of current user             NULL
 * uptime              system uptime                   NULL
 * username            username of current user        NULL
 * vol_perc            OSS/ALSA volume in percent      mixer file (/dev/mixer)
 * wifi_perc           WiFi signal in percent          interface name (wlan0)
 * wifi_essid          WiFi ESSID                      interface name (wlan0)
 */

static const struct arg args[] = {
	/* function format          argument */
	{ uptime, " [UP:%s]"},
	{ alsa_vol_perc, " [VOL:%s]", "default"},
	{ keymap, " [KEY:%s]" },
	{ cpu_perc, " [CPU:%4s%%]"  },
	{ ram_perc, " [RAM:%4s%%]" },
	{ disk_free, " [DISK:%.4sG]", "/" },
	{ battery_state, " [BAT:%s", "BAT0" },
	{ battery_perc, " %3s%%]", "BAT0" },
	{ datetime, " %s", "(%a)%d-%m %T" },
//	{ temp, "/sys/class/thermal/thermal_zone0"},
};
static const struct arg args2[] = {
	/* function format          argument */
	{ mpd_current_play, "[MUSIC:%s]" },
	{ wifi_perc, " [WIFI:%3s%%", "wlo1" },
	{ netspeed_rx, " Rx:%.7s", "wlo1" },
	{ netspeed_tx, " Tx:%.7s", "wlo1" },
	{ ipv4, " %s", "wlo1" },
	{ wifi_essid, " %s]", "wlo1" },
//	{ entropy, " ENT: %s" },
	//{ run_command, "[VOL: %4s]", "sudo amixer sget Master | awk -F\"[][]\" '/%/ { print $2 }' | head -n1" },
	{ run_command, " [TTY:%s]", "who -q |awk -Fusers= 'FNR==2{print $2}'" },
	{ datetime, " %s", "[%a]%d-%m %T" },
};
