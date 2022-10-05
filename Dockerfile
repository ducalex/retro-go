FROM espressif/idf:release-v4.3

WORKDIR /app

ADD . /app

# RUN pip install -r requirements.txt

# Apply patches
RUN cd /opt/esp/idf && \
	patch --ignore-whitespace -p1 -i "/app/tools/patches/panic-hook (esp-idf 4.2 and 4.3).diff" && \
	patch --ignore-whitespace -p1 -i "/app/tools/patches/sdcard-fix (esp-idf 4.2 and 4.3).diff"

# Build
RUN . /opt/esp/idf/export.sh && \
	python rg_tool.py --with-networking --target=odroid-go release && \
	python rg_tool.py --with-networking --target=mrgc-g32 release
