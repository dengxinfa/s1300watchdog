#!/bin/sh

[ -n "$(grep "DK01" /proc/device-tree/model 2>/dev/null)" ] && model="b1300"
[ -n "$(grep "DK04" /proc/device-tree/model 2>/dev/null)" ] && model="s1300"

case "$model" in
"b1300")
	GPIO=58
	;;
"s1300")
	GPIO=39
	;;
esac

echo $GPIO >/sys/class/gpio/export

echo out >/sys/class/gpio/gpio${GPIO}/direction

feed_dog()
{
 	local count=5
        for i in $(seq 1 $count);
        do
                echo 1 >/sys/class/gpio/gpio${GPIO}/value
                sleep 0.05
                echo 0 >/sys/class/gpio/gpio${GPIO}/value
                sleep 0.05
        done
}

detect_reboot_by_hw_watchdog()
{	
	echo in >/sys/class/gpio/gpio$GPIO/direction
	for i in $(seq 1 8)
	do
		v0=$(cat /sys/class/gpio/gpio${GPIO}/value)
		sleep 0.1
		v1=$(cat /sys/class/gpio/gpio${GPIO}/value)
		sleep 0.1
		v2=$(cat /sys/class/gpio/gpio${GPIO}/value)
		sleep 0.1
		v3=$(cat /sys/class/gpio/gpio${GPIO}/value)
		sleep 0.1
		v4=$(cat /sys/class/gpio/gpio${GPIO}/value)
                sleep 0.1                                  
                v5=$(cat /sys/class/gpio/gpio${GPIO}/value)                        
                sleep 0.1
		if [ "$v0" = "$v1" -a "$v1" = "$v2" -a "$v2" = "$v3" -a "$v3" = "$v4" -a "$v4" = "$v5" ]; then
			no_pulse=$(($no_pulse + 1));
		fi
		sleep 0.4
	done
}

feed_dog
no_pulse=0

detect_reboot_by_hw_watchdog

[ "$no_pulse" -lt 7 ] && echo "$(date +%Y-%m-%d/%T) reset" >> /reset_by_hw_watchdog

echo out >/sys/class/gpio/gpio${GPIO}/direction 

while true; do
	feed_dog
	sleep 10
done


