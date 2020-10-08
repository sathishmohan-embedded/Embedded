#!/usr/bin/bash

set -e

sleep 10

espota_file=/usr/local/bin/espota.py

usb_mnt_point=$(lsblk | grep sdb1)
usb_mnt_point=$(echo $usb_mnt_point | awk -F' ' '{print $NF}')
echo $usb_mnt_point

#Check Binary File is Available in USB
bin_file=$usb_mnt_point/ota-flash.bin
if [ -f "$bin_file" ]; then
    echo "$bin_file exists."
else 
    echo "$bin_file does not exist."
    exit
fi

echo "Scanning for Arduino OTA devices on the network ..."
ip_arr=($(((avahi-browse _arduino._tcp --resolve --parsable --terminate) | grep -F "=;") | cut -d\; -f8))
if [ ${#ip_arr[@]} == 0 ]
then
    echo "No devices found"
    exit
fi
echo "IP addresses found: "
printf '\t%s\n' "${ip_arr[@]}"

#read -p "Path to binary: " path
#read -p "Password: " password

password=password

#Flash the File to Hardware
for ip in ${ip_arr[@]}
do
	echo "Success"
    python3 $espota_file -i $ip -p 8266 --auth="$password" -f "$bin_file" 2> /dev/null && echo -e "Success:\t$ip" || echo -e "Fail:   \t$ip" &
done
wait

#Remove Files After Flashing
rm $bin_file
