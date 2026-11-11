echo "Installing Ubuntu/Debian build dependencies"
sudo apt-get install -q make g++ pkg-config \
    libfreetype-dev libx11-dev libxinerama-dev libxrandr-dev libxcursor-dev \
    libxcomposite-dev mesa-common-dev libasound2-dev freeglut3-dev \
    libcurl4-gnutls-dev libjack-dev libbluetooth-dev libgtk-3-dev \
    libwebkit2gtk-4.1-dev libsdl2-dev libusb-1.0-0-dev libhidapi-dev \
    ladspa-sdk libssl-dev libmpv-dev libavahi-client-dev patchelf

echo ""
echo "Note: the NDI SDK is also required (proprietary, not in apt)."
echo "Download it from https://ndi.video/for-developers/ndi-sdk/ and copy"
echo "include/* to /usr/local/include and bin/x86_64/lib* to /usr/local/lib"
