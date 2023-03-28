#!/vendor/bin/sh
echo "------ Pixel CMA stat ------"
for d in $(ls -d /sys/kernel/pixel_stat/mm/cma/*); do
  if [ -f $d ]; then
    echo --- $d
    cat $d
  else
    for f in $(ls $d); do
      echo --- $d/$f
      cat $d/$f
    done
  fi
done

