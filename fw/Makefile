export PATH := $(PATH):$(HOME)/sdcc-4.4.0/bin

MCU		= stm8s001j3
ARCH	= stm8
F_CPU	= 16000000
TARGET	= main

BUILD_DIR = build
SRCS := $(wildcard *.c)

CC       = sdcc
LD       = sdld
AS       = sdasstm8
OBJCOPY  = sdobjcopy
ASFLAGS  = -plosgff
CFLAGS   = -m$(ARCH) -p$(MCU) --std-sdcc11
CFLAGS  += -DF_CPU=$(F_CPU)UL -I.
CFLAGS  += --stack-auto --noinduction --use-non-free
LDFLAGS  = -m$(ARCH) -l$(ARCH) --out-fmt-ihx
OBJS     = $(addprefix $(BUILD_DIR)/,$(notdir $(SRCS:.c=.rel)))
vpath %.c  $(sort $(dir $(SRCS)))

all: $(BUILD_DIR)/$(TARGET).bin size

$(BUILD_DIR)/%.rel: %.c | $(BUILD_DIR)
	$(CC) -c $(CFLAGS) $< -o $@

$(BUILD_DIR)/%.rel: %.s | $(BUILD_DIR)
	$(AS) $(ASFLAGS) $<
	mv *.lst *.rel *.sym $(BUILD_DIR)

$(BUILD_DIR)/$(TARGET).hex: $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) -o $@

$(BUILD_DIR)/$(TARGET).bin: $(BUILD_DIR)/$(TARGET).hex
	$(OBJCOPY) -I ihex --output-target=binary $(BUILD_DIR)/$(TARGET).hex $(BUILD_DIR)/$(TARGET).bin

size: $(BUILD_DIR)/$(TARGET).bin
	@echo "Image size:"
	@stat -L -c %s $(BUILD_DIR)/$(TARGET).bin

$(BUILD_DIR):
	mkdir $@

clean:
	rm -fR $(BUILD_DIR)

flash: $(BUILD_DIR)/$(TARGET).hex
	echo "00 00 00 00 00 00" | xxd -r -p > $(BUILD_DIR)/opts.bin
	stm8flash -c stlinkv2 -p $(MCU) -s opt -w $(BUILD_DIR)/opts.bin
	stm8flash -c stlinkv2 -p $(MCU) -w $(BUILD_DIR)/$(TARGET).hex

.PHONY: clean all flash
