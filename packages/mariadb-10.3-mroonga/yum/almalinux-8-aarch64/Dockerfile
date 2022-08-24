FROM --platform=linux/arm/v8 arm64v8/almalinux:8

ARG DEBUG

RUN \
  quiet=$([ "${DEBUG}" = "yes" ] || echo "--quiet") && \
  dnf update -y ${quiet}
