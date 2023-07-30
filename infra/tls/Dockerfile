FROM alpine:3
RUN apk add openssl
RUN sed -e "s|:100:|:1000:|" -i /etc/passwd -i /etc/group
RUN adduser -u 100 certuser -D
USER certuser
CMD /tls/certgen.sh && exec sleep infinity
