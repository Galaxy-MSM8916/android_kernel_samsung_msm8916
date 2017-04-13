# host notify driver
obj-$(CONFIG_USB_HOST_NOTIFY)		+= host_notify_class.o
obj-$(CONFIG_USB_NOTIFY_LAYER)		+= usb_notify_layer.o
usb_notify_layer-y			:= usb_notify.o  usb_notify_sysfs.o dock_notify.o

ifdef CONFIG_USB_NOTIFIER
obj-$(CONFIG_ARCH_EXYNOS)			+= usb_notifier_exynos.o
obj-$(CONFIG_X86_INTEL_MID)			+= usb_notifier_intel.o
obj-$(CONFIG_ARCH_MSM)				+= usb_notifier_qcom.o
obj-$(CONFIG_USB_SPRD_DWC)			+= usb_notifier_sprd.o
endif


