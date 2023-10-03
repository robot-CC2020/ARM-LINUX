
set -e # 出错就停止
echo -e "\n<<<<<<<<<<<<<< strt >>>>>>>>>>>>>>\n"

echo -e "\n============== copy source file ==============\n"
BUILD_DIR=~/ARM-Linux/build # 虚拟机编译目录
mkdir -p ${BUILD_DIR}
rm -rf ${BUILD_DIR}/*
cp -r ./* ${BUILD_DIR}
cd ${BUILD_DIR}
pwd

echo -e "\n============== build module ==============\n"
export ARCH=arm
export CROSS_COMPILE=arm-linux-gnueabihf-
make

echo -e "\n============== copy to rootfs ==============\n"
RUN_DIR=~/share/nfs_share/rootfs/root/test # 开发板网络挂载根文件系统目录
mkdir -p ${RUN_DIR}
cp *.ko ${RUN_DIR}/

echo "RUN_DIR:${RUN_DIR}"
ls ${RUN_DIR}

echo ""
echo "<<<<<<<<<<<<<< end >>>>>>>>>>>>>>"

