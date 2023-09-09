# 配置
KDIR="/home/pdg/ARM-Linux/current_uboot_linux/linux-imx-4.1.15"
DTS_NAME="imx6ull-alientek-emmc"
SHARE_DIR="/home/pdg/share/tftp_share/"


set -e
# 定义变量
ORIGIN_DIR=$(pwd)
TARGET_DTS_FILE="${KDIR}/arch/arm/boot/dts/${DTS_NAME}.dts"

echo -e "\n============== modify device tree ==============\n"
mv "${TARGET_DTS_FILE}" "${TARGET_DTS_FILE}.backup" # 备份原文件
cp "${DTS_NAME}.dts" "${TARGET_DTS_FILE}" # 拷贝当前设备树文件
# 编译并替换 共享文件夹的设备树
cd "${KDIR}"
export ARCH=arm
export CROSS_COMPILE=arm-linux-gnueabihf-
make dtbs
cp -f "${KDIR}/arch/arm/boot/dts/${DTS_NAME}.dtb" "${SHARE_DIR}"
# 恢复设备树文件
mv "${TARGET_DTS_FILE}.backup" "${TARGET_DTS_FILE}"
echo -e "\n============== end ==============\n"