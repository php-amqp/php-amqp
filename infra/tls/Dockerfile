FROM alpine:3
RUN apk add openssl
RUN apk add acl
CMD /src/infra/tools/pamqp-certificates-generate /src/infra/tls/certificates
