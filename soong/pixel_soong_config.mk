# Add pixel common soong config here.
# Set the variables properly in ProductConfig Makefiles for each pixel device

#For USB
SOONG_CONFIG_NAMESPACES += usb
SOONG_CONFIG_usb += older_gadget

SOONG_CONFIG_usb_older_gadget := $(OLDER_GADGET)
