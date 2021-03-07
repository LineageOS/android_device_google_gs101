#!/vendor/bin/sh

#############################################################
### init.insmod.cfg format:                               ###
### ----------------------------------------------------- ###
### [insmod|setprop|enable/moprobe|wait] [path|prop name] ###
### ...                                                   ###
#############################################################

# imitates wait_for_file() in init
wait_for_file()
{
    filename="${1}"
    timeout="${2:-5}"

    expiry=$(($(date "+%s")+timeout))
    while [[ ! -e "${filename}" ]] && [[ "$(date "+%s")" -le "${expiry}" ]]
    do
        sleep 0.01
    done
}

install_display_drivers()
{
  panel_drv=`getprop ro.boot.primary_panel_drv`
  if [[ -z "$panel_drv" ]]; then
    panel_drv="panel-samsung-emul"
  fi
  modprobe -d /vendor/lib/modules exynos-drm.ko
  modprobe -d /vendor/lib/modules $panel_drv.ko
}

if [ $# -eq 1 ]; then
  cfg_file=$1
else
  # Set property even if there is no insmod config
  # to unblock early-boot trigger
  setprop vendor.common.modules.ready
  setprop vendor.device.modules.ready
  exit 1
fi

if [ -f $cfg_file ]; then
  while IFS="|" read -r action arg
  do
    case $action in
      "insmod") insmod $arg ;;
      "setprop") setprop $arg 1 ;;
      "enable") echo 1 > $arg ;;
      "modprobe") modprobe -a -d /vendor/lib/modules $arg ;;
      "wait") wait_for_file $arg ;;
      "install_display_drivers") install_display_drivers ;;
    esac
  done < $cfg_file
fi
