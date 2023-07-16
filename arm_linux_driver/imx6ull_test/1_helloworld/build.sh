echo "make   ............"
export ARCH=arm
export CROSS_COMPILE=arm-linux-gnueabihf-
make

echo "copy *.ko ........."
TARGET_DIR=~/share/nfs_share/rootfs/root/test
mkdir -p ${TARGET_DIR}
cp *.ko ${TARGET_DIR}/

echo "target dir:${TARGET_DIR}  "
ls ${TARGET_DIR}
echo ""

echo "end ........."
