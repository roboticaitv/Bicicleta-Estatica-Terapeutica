# syntax=docker/dockerfile:1
FROM espressif/idf:v5.3.2

ENV IDF_TARGET=esp32s3 \
    IDF_CCACHE_ENABLE=1 \
    CCACHE_DIR=/tmp/.ccache \
    HOME=/tmp

RUN git config --system --add safe.directory '*' \
    && mkdir -p /tmp/.ccache /tmp/.cache/Espressif/ComponentManager \
    && chmod -R 777 /tmp/.ccache /tmp/.cache

WORKDIR /project

CMD ["idf.py", "-B", "build/out", "build"]
