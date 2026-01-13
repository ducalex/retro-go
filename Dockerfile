FROM espressif/idf:release-v5.1

# Build argument to specify which apps to build (space-separated, or "none" to skip build and only pack)
ARG APPS=updater launcher retro-core prboom-go snes9x gwenesis fmsx gbsp

WORKDIR /app

ADD . /app

# RUN pip install -r requirements.txt

# Apply patches
RUN cd /opt/esp/idf && \
	patch --ignore-whitespace -p1 -i "/app/tools/patches/panic-hook (esp-idf 5).diff" && \
	patch --ignore-whitespace -p1 -i "/app/tools/patches/sdcard-fix (esp-idf 5).diff"

# Build
SHELL ["/bin/bash", "-c"]
# Build apps (if APPS is not "none"), then pack firmware
RUN if [ "$APPS" != "none" ]; then \
      . /opt/esp/idf/export.sh && \
      python rg_tool.py --target=odroid-go build $APPS; \
    fi && \
    . /opt/esp/idf/export.sh && \
    python rg_tool.py --target=odroid-go build-fw
