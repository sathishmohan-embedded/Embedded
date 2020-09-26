#!/usr/bin/bash

set -e -x

echo "Scanning for Arduino OTA devices on the network ..."
ip_arr=($(((avahi-browse _arduino._tcp --resolve --parsable --terminate) | grep -F "=;") | cut -d\; -f8))
if [ ${#ip_arr[@]} == 0 ]
then
    echo "No devices found"
    exit
fi
echo "IP addresses found: "
printf '\t%s\n' "${ip_arr[@]}"

read -p "Path to binary: " path
read -p "Password: " password

read -p "Press [ENTER] to start OTA"
echo    "--------------------------"

for ip in ${ip_arr[@]}
do
	echo "Success"
    python3 espota.py -i $ip -p 8266 --auth="$password" -f "$path" 2> /dev/null && echo -e "Success:\t$ip" || echo -e "Fail:   \t$ip" &
done
wait
