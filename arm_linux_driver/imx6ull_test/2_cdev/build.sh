
echo "copy source  ............"
BUILD_DIR=~/ARM-Linux/build # 虚拟机编译目录
mkdir -p ${BUILD_DIR}
rm -rf ${BUILD_DIR}/*
cp -r ./* ${BUILD_DIR}
cd ${BUILD_DIR}

pwd

echo "build   ............"
export ARCH=arm
export CROSS_COMPILE=arm-linux-gnueabihf-
make

echo "copy *.ko to rootfs........."
TARGET_DIR=~/share/nfs_share/rootfs/root/test # 开发板网络挂载根文件系统
mkdir -p ${TARGET_DIR}
cp *.ko ${TARGET_DIR}/

echo "target dir:${TARGET_DIR}  "
ls ${TARGET_DIR}
echo ""

echo "end ........."
