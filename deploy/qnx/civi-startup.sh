#!/bin/sh
# CIVI cockpit start-up on QNX SDP 8.0 (Snapdragon cockpit reference board).
# Ordering implements the fast-boot path (CIVI-PLT-003): telltales first, from the IFS,
# before any Qt/HMI process exists. Rear-view camera early-display is provided by the
# Qualcomm BSP (AIS/qcarcam early-RVC service) and is started by the BSP's own script
# ahead of this one.
#
# Assumes: screen (graphics) and pps are already running from the BSP build file.

CIVI=/opt/civi
export CIVI_PPS_ROOT=/pps/civi
export CIVI_CONFIG=/etc/civi/civi-config.json
export QT_QPA_PLATFORM=qnx
export QT_QPA_FONTDIR=$CIVI/fonts
export QML_DISK_CACHE_PATH=/var/civi/qmlcache       # AOT/qmlcache warm cache across boots
export LD_LIBRARY_PATH=$CIVI/lib:$LD_LIBRARY_PATH

# PPS object tree. Vehicle-owned objects are created by the vehicle-services server and
# TCU with group-read for civi_hmi; request objects are group-writable (CIVI-SW-002).
mkdir -p $CIVI_PPS_ROOT/vs $CIVI_PPS_ROOT/tcu $CIVI_PPS_ROOT/hmi $CIVI_PPS_ROOT/alerts \
         $CIVI_PPS_ROOT/nav $CIVI_PPS_ROOT/media $CIVI_PPS_ROOT/ota $CIVI_PPS_ROOT/wifi \
         $CIVI_PPS_ROOT/privacy $CIVI_PPS_ROOT/audio

# 1. Safety telltales — highest Screen z-order on the cluster display, CPU-rendered.
on -T civi_telltale_t -u civi_tt:civi_tt -p 30 \
   $CIVI/bin/civi-telltaled -d 0 -w 1920 -h 84 -z 1000 -t 300 -a $CIVI/share/telltales &

# 2. Cluster HMI (cluster partition). Priority above infotainment.
on -T civi_hmi_t -u civi_hmi:civi_hmi -p 22 $CIVI/bin/civi-cluster &
on -T civi_hmi_t -u civi_hmi:civi_hmi -p 21 $CIVI/bin/civi-hud &

# 3. Infotainment displays (replaceable without touching 1-2: CIVI-SW-008).
on -T civi_hmi_t -u civi_hmi:civi_hmi -p 12 $CIVI/bin/civi-center &
on -T civi_hmi_t -u civi_hmi:civi_hmi -p 11 $CIVI/bin/civi-passenger &

# In production each process is supervised by the platform's launcher/HAM so a crash
# restarts it; on restart the HMI re-reads PPS state and shows any active takeover or
# warning immediately (see AlertArbiter::attachVehicle).
