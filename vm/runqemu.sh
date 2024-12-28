#!/bin/bash
FTPBASE="http://ftp.de.debian.org/debian/dists/bookworm/main/installer-arm64/current/images/netboot"

# https://web.archive.org/web/20200909020002/https://blahcat.github.io/2018/01/07/building-a-debian-stretch-qemu-image-for-aarch64/

# This block fetches the netboot kernel, initrd.gz images from debian
# and boots them on QEMU. Took a bit of trial and error to get the SCSI devices working
# When troubleshooting note that the SCSI devices will likely appear after the installer grabs the right
# modules, ie, at the point where it asks you to partition the drives.
# Cancelling the installer to go to the shell or appending init=/bin/bash will not show you the disks.

cd $(dirname $0)

if [ "$1" == "install" ]; then
    [ -f linux ] || wget $FTPBASE/debian-installer/arm64/linux
    [ -f initrd.gz ] || wget $FTPBASE/debian-installer/arm64/initrd.gz
    [ -f disk.qcow2 ] || qemu-img create -f qcow2 disk.qcow2 32G

    echo "Running QEMU..."
    qemu-system-aarch64 -M virt -cpu max -m 2G -smp 2 -nographic \
    -kernel linux -initrd initrd.gz -append "console=ttyAMA0" \
    -device virtio-scsi-device,id=scsi \
    -drive file=disk.qcow2,id=rootimg,if=none,media=disk \
    -device scsi-hd,drive=rootimg \
    -netdev user,id=vmnic -device virtio-net-device,netdev=vmnic
    exit 0
fi

# After the install is done, we have to extract the system's (not the netboot's)
# kernel and initrd files. They will have different names, so no conflict here.
# This is done by mounting the qcow2 and taking them out of /boot
# /boot was the first partition during the install (if using the guide)
#
#   sudo modprobe nbd max_part=8
#   sudo qemu-nbd --connect=/dev/nbd0 disk.qcow2
#   sudo mount /dev/nbd0p1 /mnt
#   sudo cp /mnt/{initrd.img,vmlinuz} .
#   sudo chown user initrd.img vmlinuz
#   sudo qemu-nbd --disconnect /dev/nbd0
#
# After this is done we boot the system with the installed kernel (vmlinuz) and initrd.img

echo "Running QEMU..."
qemu-system-aarch64 -M virt -cpu max -m 2G -smp 2 -nographic \
    -kernel vmlinuz -initrd initrd.img -append "console=ttyAMA0 root=/dev/sda2" \
    -global virtio-blk-device.scsi=off \X
    -device virtio-scsi-device,id=scsi \
    -drive file=disk.qcow2,id=rootimg,if=none,cache=writeback \
    -device scsi-hd,drive=rootimg \
    -netdev user,id=vmnic,hostfwd=tcp::2222-:22 \
    -device virtio-net-device,netdev=vmnic

# Host:2222 forwarded to vm:22
