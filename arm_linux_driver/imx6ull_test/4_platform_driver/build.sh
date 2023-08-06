
set -e # 出错就停止
echo ""
echo "<<<<<<<<<<<<<< strt >>>>>>>>>>>>>>"

echo ""
echo "============== copy source file =============="
BUILD_DIR=~/ARM-Linux/build # 虚拟机编译目录
mkdir -p ${BUILD_DIR}
rm -rf ${BUILD_DIR}/*
cp -r ./* ${BUILD_DIR}
cd ${BUILD_DIR}
pwd

echo ""
echo "============== build =============="
export ARCH=arm
export CROSS_COMPILE=arm-linux-gnueabihf-
make
make test

echo ""
echo "============== copy to rootfs =============="
RUN_DIR=~/share/nfs_share/rootfs/root/test # 开发板网络挂载根文件系统目录
mkdir -p ${RUN_DIR}
cp *.ko ${RUN_DIR}/
cp a.out ${RUN_DIR}/

echo "RUN_DIR:${RUN_DIR}"
ls ${RUN_DIR}

echo ""
echo "<<<<<<<<<<<<<< end >>>>>>>>>>>>>>"
