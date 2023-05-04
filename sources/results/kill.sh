ps -aux | grep prj | grep miaox | grep -v root | awk '{print $2}' | head -n -1 | xargs -n 1 kill -9
