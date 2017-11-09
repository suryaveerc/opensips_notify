#! /bin/bash
#cp -v /opensipsfe/opensips_residential.cfg /usr/local/opensips/etc/opensips/opensips_residential.cfg

REDIS_PORT=$(printenv REDIS_SERVICE_PORT)
REDIS_HOST=$(printenv REDIS_SERVICE_HOST)
REST_URL=$(printenv REPO_CLIENT_SERVICE_HOST)
REST_PORT=$(printenv REPO_CLIENT_SERVICE_PORT)
HOST_IP="$( ip route get 8.8.8.8 | awk 'NR==1 {print $NF}' )"
LOG_LEVEL=$(printenv DEBUG_LEVEL)
SCENARIO=$(printenv TEST_SCENARIO)
DUMP_CORE=$(printenv CORE_DUMP)
QUEUE_SUBJECT=$(printenv QUEUE_SUBJECT)
QUEUE_TYPE=$(printenv QUEUE_TYPE)
MSG_HOST=$(printenv MSG_SERVICE_HOST)
MICRO_SRV_ARCH=$(printenv MICRO_SRV_ARCH)
DISP_DB_HOST=$(printenv MYSQL_SERVICE_HOST)
DISP_HOST=$(printenv DISPATCHER_SERVICE_HOST)
DEST_SET_ID=$(printenv DEST_SET_ID)
AVAILABLE_PUBS=$(printenv AVAILABLE_PUBS)
AVAILABLE_SUBS=$(printenv AVAILABLE_SUBS)

DESTINATION=sip:$HOST_IP:5060
echo HOST_IP: $HOST_IP
echo DESTINATION: $DESTINATION
echo REDIS_PORT: $REDIS_PORT
echo REST_URL: $REST_URL
echo REDIS_HOST: $REDIS_HOST
echo REST_PORT: $REST_PORT
echo LOG_LEVEL: $LOG_LEVEL
echo QUEUE_SUBJECT: $QUEUE_SUBJECT
echo QUEUE_TYPE: $QUEUE_TYPE
echo MSG_HOST: $MSG_HOST
echo MICRO_SRV_ARCH: $MICRO_SRV_ARCH
echo DISP_DB_HOST: $DISP_DB_HOST
echo DISP_HOST: $DISP_HOST
echo DEST_SET_ID: $DEST_SET_ID
echo AVAILABLE_PUBS: $AVAILABLE_PUBS
echo AVAILABLE_SUBS: $AVAILABLE_SUBS

cp /usr/share/zoneinfo/EST /etc/localtime

sed -i "s/LOG_LEVEL/$LOG_LEVEL/" /usr/local/opensips/etc/opensips/opensips_residential.cfg
sed -i "s/HOST_IP/$HOST_IP/" /usr/local/opensips/etc/opensips/opensips_residential.cfg
sed -i "s/REDIS_HOST/$REDIS_HOST/" /usr/local/opensips/etc/opensips/opensips_residential.cfg
sed -i "s/REDIS_PORT/$REDIS_PORT/" /usr/local/opensips/etc/opensips/opensips_residential.cfg
sed -i "s/REST_SERVICE/$REST_URL/" /usr/local/opensips/etc/opensips/opensips_residential.cfg
sed -i "s/REST_PORT/$REST_PORT/" /usr/local/opensips/etc/opensips/opensips_residential.cfg
sed -i "s/MSG_HOST/$MSG_HOST/" /usr/local/opensips/etc/opensips/opensips_residential.cfg
sed -i "s/MICRO_SRV_ARCH/$MICRO_SRV_ARCH/" /usr/local/opensips/etc/opensips/opensips_residential.cfg
sed -i "s/QUEUE_SUBJECT/$QUEUE_SUBJECT/" /usr/local/opensips/etc/opensips/opensips_residential.cfg
sed -i "s/QUEUE_TYPE/$QUEUE_TYPE/" /usr/local/opensips/etc/opensips/opensips_residential.cfg
sed -i "s/AVAILABLE_PUBS/$AVAILABLE_PUBS/" /usr/local/opensips/etc/opensips/opensips_residential.cfg
sed -i "s/AVAILABLE_SUBS/$AVAILABLE_SUBS/" /usr/local/opensips/etc/opensips/opensips_residential.cfg
sed -i "s/CORE_DUMP/$DUMP_CORE/" /etc/default/opensips
mv /home/opensips /etc/init.d/opensips

if [ "$DISP_HOST" != "" ]
then
	echo "Dispatcher Available"
	export TERM=dumb
	mysql -h $DISP_DB_HOST -uroot -proot -e "use opensips; insert into dispatcher (setid, destination) values('$DEST_SET_ID','$DESTINATION');"
	echo ":ds_reload:" >/dev/udp/$DISP_HOST/5050
else
	echo "Dispatcher NOT Available"
fi
service opensips restart

