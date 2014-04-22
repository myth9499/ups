ipcrm msg  `ipcs -q|grep dev|grep 666 |awk '{print  $2}'`
ipcrm shm  `ipcs -m|grep dev|grep 666 |awk '{print  $2}'`
ipcrm sem  `ipcs -s|grep dev|grep 666 |awk '{print  $2}'`
