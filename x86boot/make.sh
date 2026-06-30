#!/bin/bash

# 1. 변수 설정
BR_VERSION="2023.11.x" # 안정 버전
BR_DIR="buildroot"
OUT_DIR="output"
APP_SOURCE_DIR="$(pwd)/spc1000"
OVERLAY_DIR="$(pwd)/overlay"

# 2. Buildroot 소스 가져오기
if [ ! -d "$BR_DIR" ]; then
    git clone https://github.com/buildroot/buildroot.git -b $BR_VERSION --depth 1
fi

# 3. 오버레이 디렉토리 생성 (실행파일 및 자동실행 설정)
# /usr/bin에 앱을 넣고, /etc/init.d/S99app을 통해 부팅 즉시 실행
mkdir -p $OVERLAY_DIR/usr/bin
mkdir -p $OVERLAY_DIR/etc/init.d
cp $APP_SOURCE_DIR/spc1000 $OVERLAY_DIR/usr/bin/
cp -r $APP_SOURCE_DIR/taps $OVERLAY_DIR

cat <<EOF > $OVERLAY_DIR/etc/init.d/S99app
#!/bin/sh
case "\$1" in
  start)
    echo "Starting SPC1000 SDL2 App..."
    ls -l /dev/dri/
    # KMS/DRM 환경 변수 강제 및 실행
    while [ ! -c /dev/dri/card0 ] && [ ! -c /dev/dri/card1 ]; do sleep 0.5; done
    export SDL_VIDEODRIVER=kmsdrm
    export SDL_NOMOUSE=1
    /usr/bin/spc1000 &
    ;;
esac
exit 0
EOF
chmod +x $OVERLAY_DIR/etc/init.d/S99app
chmod +x $OVERLAY_DIR/usr/bin/spc1000

# 4. Buildroot 설정 (Defconfig 생성)
# make -C $BR_DIR clean

if [ ! -f "$BR_DIR/.config" ]; then
    make -C $BR_DIR qemu_x86_64_defconfig # 32비트 기준 (64비트는 x86_64_defconfig)
fi

cat <<EOF > build_options.conf 
# 최적화: 개발 도구 제외 및 압축 최소화
BR2_GCC_VERSION_13_X=y
BR2_TOOLCHAIN_BUILDROOT_GLIBC=y
BR2_INSTALL_LIBSTDCPP=y
BR2_TOOLCHAIN_BUILDROOT_CXX=y
BR2_PACKAGE_MESA3D=y
BR2_PACKAGE_MESA3D_GALLIUM_DRIVER_VIRGL=y
BR2_PACKAGE_MESA3D_GBM=y
BR2_PACKAGE_MESA3D_OPENGL_EGL=y
BR2_PACKAGE_MESA3D_OPENGL_ES=y
BR2_PACKAGE_SDL2=y
BR2_PACKAGE_SDL2_KMSDRM=y
BR2_PACKAGE_SDL2_IMAGE=y
BR2_PACKAGE_SDL2_TTF=y
BR2_PACKAGE_SDL2_ALSA=y
BR2_PACKAGE_ALSA_LIB=y
BR2_PACKAGE_ALSA_UTILS=y
BR2_PACKAGE_ALSA_LIB_MIXER=y
BR2_PACKAGE_ALSA_LIB_PCM=y
BR2_PACKAGE_BZIP2=y
BR2_PACKAGE_HOST_LZ4=y
BR2_ROOTFS_OVERLAY="$OVERLAY_DIR"
BR2_ROOTFS_DEVICE_CREATION_DYNAMIC_MDEV=y
BR2_PACKAGE_LIBDRM=y

# 부팅 속도를 위한 Initramfs 설정
BR2_TARGET_ROOTFS_INITRAMFS=y
BR2_TARGET_ROOTFS_CPIO_LZ4=y
# 커널 사이즈 최적화 (불필요한 드라이버 제거는 수동 권장)
BR2_LINUX_KERNEL_CUSTOM_CONFIG_FILE=""
BR2_LINUX_KERNEL_USE_DEFCONFIG=y
BR2_LINUX_KERNEL_DEFCONFIG="x86_64"
BR2_LINUX_KERNEL_CONFIG_FRAGMENT_FILES="../kernel_extra.config"
EOF

(
    cd $BR_DIR; support/kconfig/merge_config.sh -m .config ../build_options.conf
)

# 5. 빌드 시작
make -C $BR_DIR olddefconfig
make -C $BR_DIR toolchain
make -C $BR_DIR

echo "빌드 완료: $BR_DIR/output/images/bzImage"
