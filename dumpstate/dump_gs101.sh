#!/vendor/bin/sh
echo "------ Camera HAL Graph State Dump ------"
for f in $(ls -t /data/vendor/camera/hal_graph_state*.txt |head -1); do
  echo $f
  cat $f
done

build_type="$(getprop ro.build.type)"

echo "\n------ Power Stats Times ------"
echo -n "Boot: " && /vendor/bin/uptime -s && echo -n "Now: " && date;

echo "\n------ ACPM stats ------"
for f in /sys/devices/platform/acpm_stats/*_stats ; do
  echo "\n\n$f"
  cat $f
done

echo "\n------ CPU PM stats ------"
cat "/sys/devices/system/cpu/cpupm/cpupm/time_in_state"

echo "\n------ GENPD summary ------"
cat "/d/pm_genpd/pm_genpd_summary"

echo "\n------ Power supply property battery ------"
cat "/sys/class/power_supply/battery/uevent"
echo "\n------ Power supply property dc ------"
cat "/sys/class/power_supply/dc/uevent"
echo "\n------ Power supply property gcpm ------"
cat "/sys/class/power_supply/gcpm/uevent"
echo "\n------ Power supply property gcpm_pps ------"
cat "/sys/class/power_supply/gcpm_pps/uevent"
echo "\n------ Power supply property main-charger ------"
cat "/sys/class/power_supply/main-charger/uevent"
echo "\n------ Power supply property pca9486-mains ------"
cat "/sys/class/power_supply/pca9468-mains/uevent"
echo "\n------ Power supply property tcpm ------"
cat /sys/class/power_supply/tcpm-source-psy-*/uevent
echo "\n------ Power supply property usb ------"
cat "/sys/class/power_supply/usb/uevent"
echo "\n------ Power supply property wireless ------"
cat "/sys/class/power_supply/wireless/uevent"

if [ -d "/sys/class/power_supply/maxfg" ]
then
  echo "\n------ Power supply property maxfg ------"
  cat "/sys/class/power_supply/maxfg/uevent"
  echo "\n------ m5_state ------"
  cat "/sys/class/power_supply/maxfg/m5_model_state"
  echo "\n------ maxfg ------"
  cat "/dev/logbuffer_maxfg"
  echo "\n------ maxfg ------"
  cat "/dev/logbuffer_maxfg_monitor"
else
  echo "\n------ Power supply property maxfg_base ------"
  cat "/sys/class/power_supply/maxfg_base/uevent"
  echo "\n------ Power supply property maxfg_flip ------"
  cat "/sys/class/power_supply/maxfg_flip/uevent"
  echo "\n------ m5_state ------"
  cat "/sys/class/power_supply/maxfg_base/m5_model_state"
  echo "\n------ maxfg_base ------"
  cat "/dev/logbuffer_maxfg_base"
  echo "\n------ maxfg_flip ------"
  cat "/dev/logbuffer_maxfg_flip"
  echo "\n------ maxfg_base ------"
  cat "/dev/logbuffer_maxfg_base_monitor"
  echo "\n------ maxfg_flip ------"
  cat "/dev/logbuffer_maxfg_flip_monitor"
fi

if [ -d "/sys/class/power_supply/dock" ]
then
  echo "\n------ Power supply property dock ------"
  cat "/sys/class/power_supply/dock/uevent"
fi

if [ -e "/dev/logbuffer_tcpm" ]
then
  echo "\n------ Logbuffer TCPM ------"
  cat "/dev/logbuffer_tcpm"
  if [ -d "/sys/kernel/debug/tcpm" ]
  then
    echo "\n------ TCPM logs ------"
    cat /sys/kernel/debug/tcpm/*
  else
    echo "\n------ TCPM logs ------"
    cat /sys/kernel/debug/usb/tcpm*
  fi
fi

echo "\n------ TCPC ------"
max77759tcpc_path="/sys/devices/platform/10d50000.hsi2c/i2c-12/12-0025"
echo "registers:"
cat $max77759tcpc_path/registers
echo "frs:"
cat $max77759tcpc_path/frs
echo "auto_discharge:"
cat $max77759tcpc_path/auto_discharge
echo "bc12_enabled:"
cat $max77759tcpc_path/bc12_enabled
echo "cc_toggle_enable:"
cat $max77759tcpc_path/cc_toggle_enable
echo "contaminant_detection:"
cat $max77759tcpc_path/contaminant_detection
echo "contaminant_detection_status:"
cat $max77759tcpc_path/contaminant_detection_status

echo "\n------ PD Engine ------"
cat "/dev/logbuffer_usbpd"
echo "\n------ POGO Transport ------"
cat "/dev/logbuffer_pogo_transport"
echo "\n------ PPS-google_cpm ------"
cat "/dev/logbuffer_cpm"
echo "\n------ PPS-dc ------"
cat "/dev/logbuffer_pca9468"

echo "\n------ Battery Health ------"
cat "/sys/class/power_supply/battery/health_index_stats"
echo "\n------ Battery Health SoC Residency ------"
cat "/sys/class/power_supply/battery/swelling_data"
echo "\n------ BMS ------"
cat "/dev/logbuffer_ssoc"
echo "\n------ TTF ------"
cat "/dev/logbuffer_ttf"
echo "\n------ TTF details ------"
cat "/sys/class/power_supply/battery/ttf_details"
echo "\n------ TTF stats ------"
cat "/sys/class/power_supply/battery/ttf_stats"
echo "\n------ maxq ------"
cat "/dev/logbuffer_maxq"
echo "\n------ aacr_state ------"
cat "/sys/class/power_supply/battery/aacr_state"
echo "\n------ TEMP/DOCK-DEFEND ------"
cat "/dev/logbuffer_bd"

echo "\n------ TRICKLE-DEFEND Config ------"
cd /sys/devices/platform/google,battery/power_supply/battery/
for f in `ls bd_*`
do
  echo $f: `cat $f`
done

echo "\n------ DWELL-DEFEND Config ------"
cd /sys/devices/platform/google,charger/
for f in `ls charge_s*`
do
  echo "$f: `cat $f`"
done

echo "\n------ TEMP-DEFEND Config ------"
cd /sys/devices/platform/google,charger/
for f in `ls bd_*`
do
  echo "$f: `cat $f`"
done

echo "\n------ DC_registers dump ------"
cat "/sys/class/power_supply/pca9468-mains/device/registers_dump"
echo "\n------ max77759_chg registers dump ------"
cat "/sys/class/power_supply/main-charger/device/registers_dump"
echo "\n------ max77729_pmic registers dump ------"
cat /sys/devices/platform/10d50000.hsi2c/i2c-*/*-0066/registers_dump

if [ $build_type = "eng" ]
then
  echo "\n------ Charging table dump ------"
  cat "/d/google_battery/chg_raw_profile"

  echo "\n------ fg_model ------"
  for f in /d/maxfg*
  do
    regs=`cat $f/fg_model`
    echo $f:
    echo "$regs"
  done

  echo "\n------ fg_alo_ver ------"
  for f in /d/maxfg*
  do
    regs=`cat $f/algo_ver`
    echo $f:
    echo "$regs"
  done

  echo "\n------ fg_model_ok ------"
  for f in /d/maxfg*
  do
    regs=`cat $f/model_ok`
    echo $f:
    echo "$regs"
  done

  echo "\n------ fg registers ------"
  for f in /d/maxfg*
  do
    regs=`cat $f/registers`
    echo $f:
    echo "$regs"
  done
fi

echo "\n------ Battery EEPROM ------"
if [ -e "/sys/devices/platform/10970000.hsi2c/i2c-8/8-0050/eeprom" ]
then
  xxd /sys/devices/platform/10970000.hsi2c/i2c-8/8-0050/eeprom
fi

echo "\n------ Charger Stats ------"
cat "/sys/class/power_supply/battery/charge_details"
if [ $build_type = "eng" ]
then
  echo "\n------ Google Charger ------"
  cd /sys/kernel/debug/google_charger/
  for f in `ls pps_*`
  do
    echo "$f: `cat $f`"
  done
  echo "\n------ Google Battery ------"
  cd /sys/kernel/debug/google_battery/
  for f in `ls ssoc_*`
  do
    echo "$f: `cat $f`"
  done
fi

echo "\n------ WLC logs ------"
cat "/dev/logbuffer_wireless"
echo "\n------ WLC VER ------"
cat "/sys/class/power_supply/wireless/device/version"
echo "\n------ WLC STATUS ------"
cat "/sys/class/power_supply/wireless/device/status"
echo "\n------ WLC FW Version ------"
cat "/sys/class/power_supply/wireless/device/fw_rev"
echo "\n------ RTX ------"
cat "/dev/logbuffer_rtx"

if [ $build_type = "eng" ]
then
  echo "\n------ gvotables ------"
  cat /sys/kernel/debug/gvotables/*/status
fi

echo "\n------ Mitigation Stats ------"
echo "Source\t\tCount\tSOC\tTime\tVoltage"
for f in `ls /sys/devices/virtual/pmic/mitigation/last_triggered_count/*`
do
  count=`cat $f`
  a=${f/\/sys\/devices\/virtual\/pmic\/mitigation\/last_triggered_count\//}
  b=${f/last_triggered_count/last_triggered_capacity}
  c=${f/last_triggered_count/last_triggered_timestamp/}
  d=${f/last_triggered_count/last_triggered_voltage/}
  cnt=`cat $f`
  cap=`cat ${b/count/cap}`
  ti=`cat ${c/count/time}`
  volt=`cat ${d/count/volt}`
  echo "${a/_count/} \t$cnt\t$cap\t$ti\t$volt"
done

echo "\n------ Clock Divider Ratio ------"
echo \"Source\t\tRatio\"
for f in `ls /sys/devices/virtual/pmic/mitigation/clock_ratio/*`
do ratio=`cat $f`
  a=${f/\/sys\/devices\/virtual\/pmic\/mitigation\/clock_ratio\//}
  echo "${a/_ratio/} \t$ratio"
done

echo "\n------ Clock Stats ------"
echo "Source\t\tStats"
for f in `ls /sys/devices/virtual/pmic/mitigation/clock_stats/*`
do
  stats=`cat $f`
  a=${f/\/sys\/devices\/virtual\/pmic\/mitigation\/clock_stats\//};
  echo "${a/_stats/} \t$stats"
done

echo "\n------ Triggered Level ------"
echo "Source\t\tLevel"
for f in `ls /sys/devices/virtual/pmic/mitigation/triggered_lvl/*`
do
  lvl=`cat $f`
  a=${f/\/sys\/devices\/virtual\/pmic\/mitigation\/triggered_lvl\//}
  echo "${a/_lvl/} \t$lvl"
done

echo "\n------ Instruction ------"
for f in `ls /sys/devices/virtual/pmic/mitigation/instruction/*`
do
  val=`cat $f`
  a=${f/\/sys\/devices\/virtual\/pmic\/mitigation\/instruction\//}
  echo "$a=$val"
done

