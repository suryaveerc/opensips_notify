FROM suryaveer/opensips:1.1
MAINTAINER suryaveer

LABEL version="6.0"
LABEL description="Replaced calloc with mallocs at few doubtful places. Changed 'keys' to 'key' for GET requests"

COPY opensips/  /opensipsfe/opensips/
COPY opensips_residential.cfg libnats.so nats.h status.h version.h /

RUN cp -v /*.h /usr/include \
	&& cp -v /libnats.so /usr/lib && ldconfig \
	&& cd /opensipsfe/opensips && make install \
	&& cp -v /opensips_residential.cfg /usr/local/opensips/etc/opensips/opensips_residential.cfg \
	&& make clean 
#	\
#	&& apt-get update && apt-get install -y mysql-client

COPY init.sh /etc/my_init.d/init.sh
EXPOSE 5060/udp


# Clean up APT when done.
RUN apt-get clean && rm -rvf /var/lib/apt/lists/* /tmp/* /var/tmp/*
