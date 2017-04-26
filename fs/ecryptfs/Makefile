#
# Makefile for the Linux eCryptfs
#

ifeq ($(CONFIG_ECRYPT_FS_VIRTUAL_FAT_XATTR),y)
EXTRA_CFLAGS += -DECRYPT_FS_VIRTUAL_FAT_XATTR
endif

obj-$(CONFIG_ECRYPT_FS) += ecryptfs.o
obj-$(CONFIG_SDP) += ecryptfs_dek.o mm.o ecryptfs_sdp_chamber.o

ecryptfs-y := dentry.o file.o inode.o main.o super.o mmap.o read_write.o \
	      crypto.o keystore.o kthread.o debug.o

ecryptfs-$(CONFIG_ECRYPT_FS_MESSAGING) += messaging.o miscdev.o
