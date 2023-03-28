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


echo "------ Camera HAL Graph State Dump ------"
for f in $(ls -t /data/vendor/camera/hal_graph_state*.txt |head -1); do
  echo $f
  cat $f
done

